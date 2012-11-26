//============================================================================
// Name        : MessageEndpoint.h
// Copyright   : DataSoft Corporation 2011-2012
//	Nova is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   Nova is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with Nova.  If not, see <http://www.gnu.org/licenses/>.
// Description : Represents all the plumbing needed to hold several concurrent
//		conversations with another remote endpoint over a socket.
//============================================================================

#include "MessageManager.h"
#include "MessageEndpoint.h"
#include "../Lock.h"
#include "messages/ErrorMessage.h"
#include "Ticket.h"

#include <sys/socket.h>
#include "unistd.h"
#include "string.h"
#include <sys/time.h>
#include "errno.h"

namespace RTT
{

MessageEndpoint::MessageEndpoint(int socketFD, struct bufferevent *bufferevent)
{
	pthread_mutex_init(&m_isShutdownMutex, NULL);
	pthread_mutex_init(&m_theirUsedSerialsMutex, NULL);
	pthread_mutex_init(&m_callbackRegisterMutex, NULL);
	pthread_mutex_init(&m_availableCBsMutex, NULL);
	pthread_mutex_init(&m_nextSerialMutex, NULL);

	pthread_cond_init(&m_callbackWakeupCondition, NULL);


	m_nextSerial = 0;

	m_consecutiveTimeouts = 0;

	m_isShutDown = false;
	m_socketFD = socketFD;}

//Destructor should only be called by the callback thread, and also only while
//	the protocol lock in MessageManager is held. This is done to avoid
//	race conditions in deleting the object.
MessageEndpoint::~MessageEndpoint()
{
	pthread_mutex_destroy(&m_isShutdownMutex);
	pthread_mutex_destroy(&m_theirUsedSerialsMutex);
	pthread_mutex_destroy(&m_callbackRegisterMutex);
	pthread_mutex_destroy(&m_availableCBsMutex);
	pthread_mutex_destroy(&m_nextSerialMutex);

	pthread_cond_destroy(&m_callbackWakeupCondition);
}

//blocking call
Message *MessageEndpoint::PopMessage(Ticket &ticket, int timeout)
{
	Message *ret;

	MessageQueue *queue = m_queues.GetByOurSerial(ticket.m_ourSerialNum);
	if(queue == NULL)
	{
		//Tried to read from a queue, but it didn't exist. We must have closed it, and then tried reading.
		//	This is a protocol error
		return new ErrorMessage(PROTOCOL_ERROR);
	}

	//TODO: This is not quite right. We should keep returning valid messages as long as
	//	we have them. We should ask the MessageQueue for a new message (maybe peek?)
	//	and only return a shutdown message if one doesn't exist there.

	//If we're shut down, then return a shutdown message
	{
		Lock shutdownLock(&m_isShutdownMutex);
		if(m_isShutDown)
		{
			return new ErrorMessage(ERROR_SOCKET_CLOSED);
		}
	}

	ret = queue->PopMessage(timeout);

	if(ret->m_messageType == MESSAGE_ERROR)
	{
		ErrorMessage *error = (ErrorMessage*)ret;
		if(error->m_errorType == ERROR_TIMEOUT)
		{
			m_consecutiveTimeouts++;
			//TODO: deal with timeouts
		}
	}

	return ret;
}

uint32_t MessageEndpoint::StartConversation()
{
	uint32_t nextSerial = GetNextOurSerialNum();
	//Technically, there's a race condition here if we happen to go all the way around
	//	the 32 bit integer space in serial numbers before this next command is executed.
	//	Suffice it to say that I think it's safe as it is.
	m_queues.AddQueue(nextSerial);

	return nextSerial;
}

bool MessageEndpoint::PushMessage(Message *message)
{
	uint32_t ourSerial;
	bool isNewCallback = false;

	if(message == NULL)
	{
		return false;
	}

	{
		Lock shutdownLock(&m_isShutdownMutex);
		if(m_isShutDown)
		{
			message->DeleteContents();
			delete message;
			return false;
		}
	}

	//The other endpoint must always provide a valid "our" serial number, ignore if they don't
	if(message->m_ourSerialNumber == 0)
	{
		message->DeleteContents();
		delete message;
		return false;
	}

	//If this is a new conversation from the endpoint (new callback conversation)
	if(message->m_theirSerialNumber == 0)
	{
		//Look up to see if there's a MessageQueue for this serial
		MessageQueue *queue = m_queues.GetByTheirSerial(message->m_ourSerialNumber);

		//If this is a brand new callback message
		if(queue == NULL)
		{
			isNewCallback = true;
			ourSerial = GetNextOurSerialNum();
			m_queues.AddQueue(ourSerial, message->m_ourSerialNumber);

			queue = m_queues.GetByOurSerial(ourSerial);
			//It's possible to have a race condition that would delete this MessageQueue before we could access it
			if(queue == NULL)
			{
				message->DeleteContents();
				delete message;
				return false;
			}
		}
		else
		{
			ourSerial = queue->GetOurSerialNum();
		}
		if(queue->PushMessage(message))
		{
			if(isNewCallback)
			{
				{
					Lock lock(&m_availableCBsMutex);
					m_availableCBs.push(ourSerial);
				}
				pthread_cond_signal(&m_callbackWakeupCondition);
			}
			return true;
		}
		else
		{
			return false;
		}
	}

	//If we got here, the message should be for an existing conversation
	//	(IE: Both serials should be valid)

	MessageQueue *queue = m_queues.GetByOurSerial(message->m_theirSerialNumber);
	if(queue == NULL)
	{
		//If there wasn't a MessageQueue for this serial number, then this message must be late or in error
		message->DeleteContents();
		delete message;
		return false;
	}

	//Update the MessageQueue to have the received "their" serial. It might not have it yet.
	m_queues.AddQueue(message->m_theirSerialNumber, message->m_ourSerialNumber);
	return queue->PushMessage(message);
}

bool MessageEndpoint::RegisterCallback(Ticket &outTicket)
{
	//Only one thread in this function at a time
	Lock lock(&m_callbackRegisterMutex);
	{
		uint32_t nextSerial;
		{
			//Protection for the m_callbackDoWakeup bool
			Lock condLock(&m_availableCBsMutex);

			//TODO: Unprotected read of byte value m_isShutdown. Probably safe though.
			while(m_availableCBs.empty() && !m_isShutDown)
			{
				pthread_cond_wait(&m_callbackWakeupCondition, &m_availableCBsMutex);
			}

			if(m_isShutDown)
			{
				return false;
			}
			//This is the first message of the protocol. This message contains the serial number we will be expecting later
			nextSerial = m_availableCBs.front();
			m_availableCBs.pop();
		}

		outTicket.m_isCallback = true;
		outTicket.m_hasInit = true;
		outTicket.m_ourSerialNum = nextSerial;
		outTicket.m_socketFD = m_socketFD;

		MessageQueue *queue = m_queues.GetByOurSerial(nextSerial);
		if(queue == NULL)
		{
			outTicket.m_theirSerialNum = 0;
		}
		else
		{
			outTicket.m_theirSerialNum = queue->GetTheirSerialNum();
		}
	}

	return true;
}

uint32_t MessageEndpoint::GetNextOurSerialNum()
{
	Lock lock(&m_nextSerialMutex);
	m_nextSerial++;

	//Must not be in use nor zero (we might overflow back to used serials)
	while( (m_queues.GetByOurSerial(m_nextSerial) != NULL) || (m_nextSerial == 0))
	{
		m_nextSerial++;
	}

	return m_nextSerial;
}

void MessageEndpoint::Shutdown()
{
	{
		Lock shutdownLock(&m_isShutdownMutex);
		m_isShutDown = true;
	}

	//Shut down each MessageQueue
	std::vector<uint32_t> serials = m_queues.GetUsedSerials();
	for(uint i = 0; i < serials.size(); i++)
	{
		MessageQueue *queue = m_queues.GetByOurSerial(serials[i]);
		if(queue != NULL)
		{
			queue->Shutdown();
		}
	}

	pthread_cond_signal(&m_callbackWakeupCondition);
}

bool MessageEndpoint::RemoveMessageQueue(uint32_t ourSerial)
{
	return m_queues.RemoveQueue(ourSerial);
}

}

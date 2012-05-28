//============================================================================
// Name        : MessageQueue.cpp
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
// Description : An item in the MessageManager's table. Contains a pair of queues
//	of received messages on a particular socket
//============================================================================

#include "MessageQueue.h"
#include "../Lock.h"
#include "messages/ErrorMessage.h"

#include <sys/socket.h>
#include "unistd.h"
#include "string.h"
#include "stdio.h"
#include <sys/time.h>
#include "errno.h"

namespace RTT
{

MessageQueue::MessageQueue(int socketFD, enum ProtocolDirection forwardDirection)
{
	pthread_mutex_init(&m_forwardQueueMutex, NULL);
	pthread_mutex_init(&m_callbackRegisterMutex, NULL);
	pthread_mutex_init(&m_callbackCondMutex, NULL);
	pthread_mutex_init(&m_callbackQueueMutex, NULL);
	pthread_mutex_init(&m_isShutdownMutex, NULL);
	pthread_cond_init(&m_readWakeupCondition, NULL);
	pthread_cond_init(&m_callbackWakeupCondition, NULL);

	m_expectedcallbackSerial = 0;
	m_forwardSerialNumber = 0;

	m_isShutDown = false;
	m_forwardDirection = forwardDirection;
	m_socketFD = socketFD;

	pthread_create(&m_producerThread, NULL, StaticThreadHelper, this);
	pthread_detach(m_producerThread);
}

//Destructor should only be called by the callback thread, and also only while
//	the protocol lock in MessageManager is held. This is done to avoid
//	race conditions in deleting the object.
MessageQueue::~MessageQueue()
{
	//Shutdown will cause the producer thread to make an ErrorMessage then quit
	//This is probably redundant, but we do it again just to make sure
	shutdown(m_socketFD, SHUT_RDWR);
	close(m_socketFD);

	//Wait for the producer thread to finish,
	// We can't have his object destroyed out from underneath him
	pthread_join(m_producerThread, NULL);

	//Delete any straggling messages in the queues
	while(!m_forwardQueue.empty())
	{
		delete m_forwardQueue.front();
		m_forwardQueue.pop();
	}
	while(!m_callbackQueue.empty())
	{
		delete m_callbackQueue.front();
		m_callbackQueue.pop();
	}

	pthread_mutex_destroy(&m_forwardQueueMutex);
	pthread_mutex_destroy(&m_callbackRegisterMutex);
	pthread_mutex_destroy(&m_callbackCondMutex);
	pthread_mutex_destroy(&m_callbackQueueMutex);
	pthread_mutex_destroy(&m_isShutdownMutex);
	pthread_cond_destroy(&m_readWakeupCondition);
	pthread_cond_destroy(&m_callbackWakeupCondition);
}

//blocking call
Message *MessageQueue::PopMessage(enum ProtocolDirection direction, int timeout)
{
	Message* retMessage;

	//If indefinite read:
	if(timeout == 0)
	{
		if(direction == m_forwardDirection)
		{
			//Protection for the queue structure
			Lock lockQueue(&m_forwardQueueMutex);

			bool gotCorrectSerial = false;
			while(!gotCorrectSerial)
			{
				//While loop to protect against spurious wakeups
				while(m_forwardQueue.empty())
				{
					pthread_cond_wait(&m_readWakeupCondition, &m_forwardQueueMutex);
				}

				retMessage = m_forwardQueue.front();
				m_forwardQueue.pop();
				if(retMessage->m_serialNumber == m_forwardSerialNumber)
				{
					gotCorrectSerial = true;
				}
				else
				{
					//Discard this message and get a new one
					//TODO: Must clear this message's internals or it will leak!
					delete retMessage;
				}
			}
		}
		else
		{
			//Protection for the queue structure
			Lock lockQueue(&m_callbackQueueMutex);

			bool gotCorrectSerial = false;
			while(!gotCorrectSerial)
			{
				//While loop to protect against spurious wakeups
				while(m_callbackQueue.empty())
				{
					pthread_cond_wait(&m_readWakeupCondition, &m_callbackQueueMutex);
				}

				retMessage = m_callbackQueue.front();
				m_callbackQueue.pop();
				if(retMessage->m_serialNumber == m_expectedcallbackSerial)
				{
					gotCorrectSerial = true;
				}
				else
				{
					//Discard this message and get a new one
					//TODO: Must clear this message's internals or it will leak!
					delete retMessage;
				}
			}

		}
	}
	//Read with timeout
	else
	{
		struct timespec timespec;
		struct timeval timeval;
		gettimeofday(&timeval, NULL);		//TODO: Check this for error return code
	    timespec.tv_sec  = timeval.tv_sec;
	    timespec.tv_nsec = timeval.tv_usec * 1000;
	    timespec.tv_sec += timeout;

		if(direction == m_forwardDirection)
		{
			//Protection for the queue structure
			Lock lockQueue(&m_forwardQueueMutex);

			bool gotCorrectSerial = false;
			while(!gotCorrectSerial)
			{
				//While loop to protect against spurious wakeups
				while(m_forwardQueue.empty())
				{
					int errCondition = 	pthread_cond_timedwait(&m_readWakeupCondition, &m_forwardQueueMutex, &timespec);
					if (errCondition == ETIMEDOUT)
					{
						return new ErrorMessage(ERROR_TIMEOUT, m_forwardDirection);
					}
				}

				retMessage = m_forwardQueue.front();
				m_forwardQueue.pop();
				if(retMessage->m_serialNumber == m_forwardSerialNumber)
				{
					gotCorrectSerial = true;
				}
				else
				{
					//TODO: Must clear this message's internals or it will leak!
					delete retMessage;
				}
			}
		}
		else
		{
			//Protection for the queue structure
			Lock lockQueue(&m_callbackQueueMutex);

			bool gotCorrectSerial = false;
			while(!gotCorrectSerial)
			{
				//While loop to protect against spurious wakeups
				while(m_callbackQueue.empty())
				{
					int errCondition = 	pthread_cond_timedwait(&m_readWakeupCondition, &m_callbackQueueMutex, &timespec);
					if (errCondition == ETIMEDOUT)
					{
						return new ErrorMessage(ERROR_TIMEOUT, m_forwardDirection);
					}
				}

				retMessage = m_callbackQueue.front();
				m_callbackQueue.pop();
				if(retMessage->m_serialNumber == m_expectedcallbackSerial)
				{
					gotCorrectSerial = true;
				}
				else
				{
					//Discard this message and get a new one
					//TODO: Must clear this message's internals or it will leak!
					delete retMessage;
				}
			}
		}
	}

	return retMessage;
}

void *MessageQueue::StaticThreadHelper(void *ptr)
{
	return reinterpret_cast<MessageQueue*>(ptr)->ProducerThread();
}

void MessageQueue::PushMessage(Message *message)
{

	//If this is a callback message (not the forward direction)
	if(message->m_direction != m_forwardDirection)
	{
		{
			//Protection for the queue structure
			Lock lock(&m_callbackQueueMutex);
			m_callbackQueue.push(message);
		}

		//Protection for the m_callbackDoWakeup bool
		//Wake up anyone sleeping for a callback message!
		Lock condLock(&m_callbackCondMutex);
		pthread_cond_signal(&m_callbackWakeupCondition);
		pthread_cond_signal(&m_readWakeupCondition);
	}
	else
	{
		{
			//Protection for the queue structure
			Lock lock(&m_forwardQueueMutex);
			m_forwardQueue.push(message);
		}

		//If there are no sleeping threads, this simply does nothing
		pthread_cond_signal(&m_readWakeupCondition);
	}
}

bool MessageQueue::RegisterCallback()
{
	//Only one thread in this function at a time
	Lock lock(&m_callbackRegisterMutex);

	{
		//Protection for the m_callbackDoWakeup bool
		Lock condLock(&m_callbackQueueMutex);
		while(m_callbackQueue.empty())
		{
			pthread_cond_wait(&m_callbackWakeupCondition, &m_callbackQueueMutex);
		}
	}

	//This is the first message of the protocol. This message contains the serial number we will be expecting later
	{
		//Protection for the queue structure
		Lock lockQueue(&m_callbackQueueMutex);
		if(!m_callbackQueue.empty())
		{
			Message *nextMessage = m_callbackQueue.front();
			m_expectedcallbackSerial = nextMessage->m_serialNumber;
		}
	}

	Lock shutdownLock(&m_isShutdownMutex);
	return !m_isShutDown;
}

void MessageQueue::NextConversation()
{
	m_forwardSerialNumber++;
}

uint32_t MessageQueue::GetSerialNumber(enum ProtocolDirection direction)
{
	if(direction == m_forwardDirection)
	{
		return m_forwardSerialNumber;
	}
	else
	{
		return m_expectedcallbackSerial;
	}

}

void *MessageQueue::ProducerThread()
{
	while(true)
	{
		uint32_t length = 0;
		char buff[sizeof(length)];
		uint totalBytesRead = 0;
		int bytesRead = 0;

		// Read in the message length
		while( totalBytesRead < sizeof(length))
		{
			bytesRead = read(m_socketFD, buff + totalBytesRead, sizeof(length) - totalBytesRead);

			if(bytesRead <= 0)
			{
				//The socket died on us!
				{
					Lock shutdownLock(&m_isShutdownMutex);
					//Mark the queue as closed, put an error message on the queue, and quit reading
					m_isShutDown = true;
				}
				//Push an ERROR_SOCKET_CLOSED message into both queues. So that everyone knows we're closed
				PushMessage(new ErrorMessage(ERROR_SOCKET_CLOSED, DIRECTION_TO_CLIENT));
				PushMessage(new ErrorMessage(ERROR_SOCKET_CLOSED, DIRECTION_TO_SERVER));
				return NULL;
			}
			else
			{
				totalBytesRead += bytesRead;
			}
		}

		// Make sure the length appears valid
		// TODO: Assign some arbitrary max message size to avoid filling up memory by accident
		memcpy(&length, buff, sizeof(length));
		if (length == 0)
		{
			PushMessage(new ErrorMessage(ERROR_MALFORMED_MESSAGE, m_forwardDirection));
			continue;
		}

		char *buffer = (char*)malloc(length);

		if (buffer == NULL)
		{
			// This should never happen. If it does, probably because length is an absurd value (or we're out of memory)
			PushMessage(new ErrorMessage(ERROR_MALFORMED_MESSAGE, m_forwardDirection));
			free(buffer);
			continue;
		}

		// Read in the actual message
		totalBytesRead = 0;
		bytesRead = 0;
		while(totalBytesRead < length)
		{
			bytesRead = read(m_socketFD, buffer + totalBytesRead, length - totalBytesRead);

			if(bytesRead <= 0)
			{
				//The socket died on us!
				{
					Lock shutdownLock(&m_isShutdownMutex);
					//Mark the queue as closed, put an error message on the queue, and quit reading
					m_isShutDown = true;
				}
				//Push an ERROR_SOCKET_CLOSED message into both queues. So that everyone knows we're closed
				PushMessage(new ErrorMessage(ERROR_SOCKET_CLOSED, DIRECTION_TO_CLIENT));
				PushMessage(new ErrorMessage(ERROR_SOCKET_CLOSED, DIRECTION_TO_SERVER));
				free(buffer);
				return NULL;
			}
			else
			{
				totalBytesRead += bytesRead;
			}
		}


		if(length < MSG_HEADER_SIZE)
		{
			PushMessage(new ErrorMessage(ERROR_MALFORMED_MESSAGE, m_forwardDirection));
			free(buffer);
			continue;
		}

		PushMessage(Message::Deserialize(buffer, length, m_forwardDirection));
		free(buffer);
		continue;
	}

	return NULL;
}

}

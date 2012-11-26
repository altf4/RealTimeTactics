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

#include "../Lock.h"
#include "MessageQueue.h"
#include "messages/ErrorMessage.h"

#include <sys/time.h>
#include <pthread.h>
#include "errno.h"

namespace RTT
{

MessageQueue::MessageQueue(uint32_t ourSerial)
{
	pthread_mutex_init(&m_queueMutex, NULL);
	pthread_mutex_init(&m_theirSerialNumMutex, NULL);
	pthread_mutex_init(&m_ourSerialNumMutex, NULL);
	pthread_cond_init(&m_popWakeupCondition, NULL);

	m_theirSerialNum = 0;
	m_ourSerialNum = ourSerial;
	isShutdown = false;
}

MessageQueue::~MessageQueue()
{
	{
		Lock lock(&m_queueMutex);
		//When deleted, be sure to clear out any straggling messages which might have gotten left over
		while(!m_queue.empty())
		{
			m_queue.front()->DeleteContents();
			delete m_queue.front();
			m_queue.pop();
		}
	}
	pthread_mutex_destroy(&m_queueMutex);
	pthread_mutex_destroy(&m_theirSerialNumMutex);
	pthread_mutex_destroy(&m_ourSerialNumMutex);
	pthread_cond_destroy(&m_popWakeupCondition);
}

Message *MessageQueue::PopMessage(int timeout)
{
	Message *retMessage;

	//If indefinite read:
	if(timeout == 0)
	{
		//Protection for the queue structure
		Lock lock(&m_queueMutex);

		//While loop to protect against spurious wakeups
		while(m_queue.empty() && !isShutdown)
		{
			pthread_cond_wait(&m_popWakeupCondition, &m_queueMutex);
		}
		if(isShutdown)
		{
			return new ErrorMessage(ERROR_SOCKET_CLOSED);
		}

		retMessage = m_queue.front();
		m_queue.pop();
	}
	//Read with timeout
	else
	{
		struct timespec timespec;
		struct timeval timeval;
		gettimeofday(&timeval, NULL);
		timespec.tv_sec  = timeval.tv_sec;
		timespec.tv_nsec = timeval.tv_usec*1000;
		timespec.tv_sec += timeout;

		//Protection for the queue structure
		Lock lock(&m_queueMutex);

		//While loop to protect against spurious wakeups
		while(m_queue.empty() && !isShutdown)
		{
			if(pthread_cond_timedwait(&m_popWakeupCondition, &m_queueMutex, &timespec) == ETIMEDOUT)
			{
				return new ErrorMessage(ERROR_TIMEOUT);
			}
		}
		if(isShutdown)
		{
			return new ErrorMessage(ERROR_SOCKET_CLOSED);
		}

		retMessage = m_queue.front();
		m_queue.pop();
	}

	return retMessage;
}

bool MessageQueue::PushMessage(Message *message)
{
	if(message == NULL)
	{
		return false;
	}

	uint32_t theirSerial = GetTheirSerialNum();

	//If we're expecting a new serial number...
	if(theirSerial == 0)
	{
		SetTheirSerialNum(message->m_ourSerialNumber);
	}
	//Else, we should expect to see the same serial number here as before
	else
	{
		//Throw away the message if the serial number doesn't match up
		if(theirSerial != message->m_ourSerialNumber)
		{
			message->DeleteContents();
			delete message;
			return false;
		}
	}

	//Push the message
	{
		Lock lock(&m_queueMutex);
		m_queue.push(message);
	}

	//Wake up anyone sleeping for a message
	pthread_cond_signal(&m_popWakeupCondition);
	return true;
}

uint32_t MessageQueue::GetTheirSerialNum()
{
	Lock lock(&m_theirSerialNumMutex);
	return m_theirSerialNum;
}

uint32_t MessageQueue::GetOurSerialNum()
{
	Lock lock(&m_ourSerialNumMutex);
	return m_ourSerialNum;
}

void MessageQueue::SetTheirSerialNum(uint32_t serial)
{
	Lock lock(&m_theirSerialNumMutex);
	m_theirSerialNum = serial;
}

void MessageQueue::Shutdown()
{
	isShutdown = true;
	pthread_cond_signal(&m_popWakeupCondition);
}

}

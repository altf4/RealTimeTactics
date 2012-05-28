//============================================================================
// Name        : MessageManager.h
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
// Description : Manages all incoming messages on sockets
//============================================================================

#include "MessageManager.h"
#include "../Lock.h"
#include "messages/ErrorMessage.h"

#include <sys/socket.h>
#include <iostream>

namespace RTT
{

MessageManager *MessageManager::m_instance = NULL;

MessageManager::MessageManager(enum ProtocolDirection direction)
{
	pthread_mutex_init(&m_queuesLock, NULL);
	pthread_mutex_init(&m_protocolLock, NULL);
	m_forwardDirection = direction;
}

void MessageManager::Initialize(enum ProtocolDirection direction)
{
	if(m_instance == NULL)
	{
		m_instance = new MessageManager(direction);
	}
}

MessageManager &MessageManager::Instance()
{
	if (m_instance == NULL)
	{
		std::cerr << "Critical error in MessageManager: You must first initialize it with a direction"
				"before calling Instance()";
	}
	return *MessageManager::m_instance;
}

Message *MessageManager::PopMessage(int socketFD, enum ProtocolDirection direction, int timeout)
{
	MessageQueue *queue;

	{
		Lock lock(&m_queuesLock);
		if(m_queues.count(socketFD) > 0)
		{
			queue = m_queues[socketFD];
		}
		else
		{
			return new ErrorMessage(ERROR_SOCKET_CLOSED, direction);
		}
	}

	Message *message = queue->PopMessage(direction, timeout);
	if(message->m_messageType == MESSAGE_ERROR)
	{
		ErrorMessage *errorMessage = (ErrorMessage*)message;
		if(errorMessage->m_errorType == ERROR_SOCKET_CLOSED)
		{
			CloseSocket(socketFD);
		}
	}

	return message;
}

void MessageManager::StartSocket(int socketFD)
{
	//Initialize the MessageQueue if it doesn't yet exist
	{
		Lock lock(&m_queuesLock);
		if(m_queues.count(socketFD) == 0)
		{
			m_queues[socketFD] = new MessageQueue(socketFD, m_forwardDirection);
		}
	}
}

Lock MessageManager::UseSocket(int socketFD)
{
	//Creates a message queue, if one does not already exist
	StartSocket(socketFD);

	pthread_mutex_t *mutex;
	{
		//Initialize the protocol lock if it doesn't yet exist
		Lock lock(&m_protocolLock);
		if(m_protocolLocks.count(socketFD) == 0)
		{
			//If there is no lock object here yet, initialize it
			m_protocolLocks[socketFD] = new pthread_mutex_t;
			pthread_mutex_init(m_protocolLocks[socketFD], NULL);
		}
		mutex =  m_protocolLocks[socketFD];
	}

	//Increment the MessageQeueu's forward serial number
	{
		//Allows us to safely access the message queue
		Lock protocolLock(mutex);

		MessageQueue *queue;
		//Initialize the MessageQueue if it doesn't yet exist
		{
			Lock lock(&m_queuesLock);
			if(m_queues.count(socketFD) == 0)
			{
				m_queues[socketFD] = new MessageQueue(socketFD, m_forwardDirection);
			}
			queue = m_queues[socketFD];
		}

		queue->NextConversation();
	}

	return Lock(mutex);
}

void MessageManager::CloseSocket(int socketFD)
{
	Lock lock(&m_queuesLock);

	if(m_queues.count(socketFD) > 0)
	{
		shutdown(socketFD, SHUT_RDWR);
		close(socketFD);
	}
}

bool MessageManager::RegisterCallback(int socketFD)
{
	bool foundIt = false;
	MessageQueue *queue = NULL;
	{
		Lock lock(&m_queuesLock);

		if(m_queues.count(socketFD) > 0)
		{
			foundIt = true;
			queue = m_queues[socketFD];
		}
	}

	if(foundIt)
	{
		//If register comes back false, then we have to clean up the dead MessageQueue
		bool isQueueAlive = queue->RegisterCallback();
		if(!isQueueAlive)
		{

			//Destructor here is a blocking call. So we call that before locking the queues mutex
			{
				//Deleting the message queue requires that nobody else is using it! So lock the protocol mutex for this queue
				Lock protocolLock = UseSocket(socketFD);
				delete queue;

				Lock lock(&m_queuesLock);
				m_queues.erase(socketFD);
			}
		}
		return isQueueAlive;
	}
	return false;
}

std::vector <int>MessageManager::GetSocketList()
{
	Lock lock(&m_queuesLock);
	std::vector<int> sockets;

	std::map<int, MessageQueue*>::iterator it;
	for(it = m_queues.begin(); it != m_queues.end(); ++it)
	{
		sockets.push_back(it->first);
	}

	return sockets;
}

uint32_t MessageManager::GetSerialNumber(int socketFD,  enum ProtocolDirection direction)
{
	StartSocket(socketFD);

	MessageQueue *queue;
	//Initialize the MessageQueue if it doesn't yet exist
	{
		Lock lock(&m_queuesLock);
		if(m_queues.count(socketFD) == 0)
		{
			m_queues[socketFD] = new MessageQueue(socketFD, m_forwardDirection);
		}
		queue = m_queues[socketFD];
	}

	return queue->GetSerialNumber(direction);
}

}

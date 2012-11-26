//============================================================================
// Name        : Ticket.h
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
// Description : Manages unique IDs for a user of the MessageManager. Represents
//		a "ticket" as one might receive from the DMV. It holds your spot in line
//		and lets the system let you know when a new message has been received for you.
//============================================================================

#include "MessageManager.h"
#include "Ticket.h"

#include "iostream"

using namespace std;

namespace RTT
{

Ticket::Ticket()
{
	m_ourSerialNum = 0;
	m_theirSerialNum = 0;
	m_isCallback = true;
	m_hasInit = false;
	m_socketFD = -1;
	m_endpointLock = NULL;
}

Ticket::Ticket(uint32_t ourSerial, uint32_t theirSerial, bool isCallback, bool hasInit, int socketFD, pthread_rwlock_t *rwlock)
{
	m_ourSerialNum = ourSerial;
	m_theirSerialNum = theirSerial;
	m_isCallback = isCallback;
	m_hasInit = hasInit;
	m_socketFD = socketFD;
	m_endpointLock = rwlock;
	pthread_rwlock_rdlock(m_endpointLock);
}

Ticket::~Ticket()
{
	MessageEndpointLock endpointLock = MessageManager::Instance().GetEndpoint(m_socketFD);
	if(endpointLock.m_endpoint != NULL)
	{
		if(endpointLock.m_endpoint->RemoveMessageQueue(m_ourSerialNum) == false)
		{
			cerr << "Tried to delete MessageQueue from ticket, but failed.\n";
		}
	}
	if(m_endpointLock != NULL)
	{
		pthread_rwlock_unlock(m_endpointLock);
	}
}


}


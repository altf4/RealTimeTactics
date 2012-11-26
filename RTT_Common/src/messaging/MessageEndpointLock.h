//============================================================================
// Name        : MessageEndpointLock.h
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
// Description : Contains a pair of a MessageQueue and a r/w lock
//============================================================================

#ifndef MESSAGEENDPOINTLOCK_H_
#define MESSAGEENDPOINTLOCK_H_

#include "MessageEndpoint.h"
#include "../Lock.h"

#include "pthread.h"

namespace RTT
{

class MessageEndpointLock
{

public:

	MessageEndpointLock()
	{
		m_endpoint = NULL;
		m_rwLock = NULL;
	}

	MessageEndpointLock(MessageEndpoint *endpoint, pthread_rwlock_t *rwLock)
	{
		m_endpoint = endpoint;
		m_rwLock = rwLock;
	}
	~MessageEndpointLock()
	{
		if(m_rwLock != NULL)
		{
			pthread_rwlock_unlock(m_rwLock);
		}
	}

	MessageEndpoint *m_endpoint;

private:

	pthread_rwlock_t *m_rwLock;
};

}
#endif /* MESSAGEENDPOINTLOCK_H_ */

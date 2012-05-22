//============================================================================
// Name        : Lock.h
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
// Description : Simple wrapper class for easy use of pthread mutex locking
//============================================================================

#ifndef LOCK_H_
#define LOCK_H_

#include "pthread.h"

namespace RTT
{

class Lock
{

public:
	Lock(pthread_mutex_t *lock)
	{
		isMutex = true;
		m_mutex = lock;
		pthread_mutex_lock(m_mutex);
	}

	Lock(pthread_rwlock_t *lock, bool isReadLock)
	{
		isMutex = false;
		m_rwlock = lock;
		if(isReadLock)
		{
			pthread_rwlock_rdlock(m_rwlock);
		}
		else
		{
			pthread_rwlock_wrlock(m_rwlock);
		}
	}

	//Blank constructor meant to be used in conjunction with GetLock
	Lock()
	{
		isMutex = true;
	}

	//Allows the user to get a lock outside the scope of where the Lock object is declared
	void GetLock(pthread_mutex_t *lock)
	{
		m_mutex = lock;
		pthread_mutex_lock(m_mutex);
	}

	~Lock()
	{
		if(isMutex)
		{
			pthread_mutex_unlock(m_mutex);
		}
		else
		{
			pthread_rwlock_unlock(m_rwlock);
		}
	}

private:
	bool isMutex;
	pthread_mutex_t *m_mutex;
	pthread_rwlock_t *m_rwlock;
};

}

#endif /* LOCK_H_ */

//============================================================================
// Name        : MessageQueueBimap.cpp
// Copyright   : DataSoft Corporation 2012
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
// Description : A map with two keys (one mandatory, one optional) that
//		maintain pointers to MessageQueues.
//============================================================================

#include "MessageQueueBimap.h"
#include "../Lock.h"

#include "iostream"

using namespace std;

namespace RTT
{

MessageQueueBimap::MessageQueueBimap()
{
	pthread_mutex_init(&m_queuesMutex, NULL);
}

MessageQueueBimap::~MessageQueueBimap()
{
	//Delete any straggling messages in the "our" map
	//We DON'T need to also delete anything from the "their" map since that one is optional. Therefore, its contents
	//	is a subset of the "our" map. We only need to delete what's here.
	for(std::map<uint32_t, MessageQueue*>::iterator iterator = m_ourQueues.begin(); iterator != m_ourQueues.end(); iterator++)
	{
		if(iterator->second != NULL)
		{
			delete iterator->second;
		}
	}

	pthread_mutex_destroy(&m_queuesMutex);
}

MessageQueue *MessageQueueBimap::GetByOurSerial(uint32_t ourSerial)
{
	MessageQueue *queue = NULL;

	Lock lock(&m_queuesMutex);
	if(m_ourQueues.count(ourSerial) > 0)
	{
		queue = m_ourQueues[ourSerial];
	}

	return queue;
}

MessageQueue *MessageQueueBimap::GetByTheirSerial(uint32_t theirSerial)
{
	MessageQueue *queue = NULL;

	Lock lock(&m_queuesMutex);
	if(m_theirQueues.count(theirSerial) > 0)
	{
		queue = m_theirQueues[theirSerial];
	}

	return queue;
}

void MessageQueueBimap::AddQueue(uint32_t ourSerial)
{
	Lock lock(&m_queuesMutex);

	if(m_ourQueues.count(ourSerial) > 0)
	{
		//We've already got one
		return;
	}

	m_ourQueues[ourSerial] = new MessageQueue(ourSerial);
}

void MessageQueueBimap::AddQueue(uint32_t ourSerial, uint32_t theirSerial)
{
	//First, let's make sure a MessageQueue exists for the "our" serial
	Lock lock(&m_queuesMutex);

	MessageQueue *queue;
	if(m_ourQueues.count(ourSerial) > 0)
	{
		//We've already got one
		queue = m_ourQueues[ourSerial];
	}
	else
	{
		queue = new MessageQueue(ourSerial);
		m_ourQueues[ourSerial] = queue;
	}

	//Then let's hook up the "their" serial number to the same MessageQueue
	//If we've already got one...
	if(m_theirQueues.count(theirSerial) > 0)
	{
		//If the existing queue isn't the same as the new one!
		if(queue != m_theirQueues[theirSerial])
		{
			cerr << "Tried to add a new MessageQueue, but there was already one here. This shouldn't happen\n";
			delete m_theirQueues[theirSerial];
			m_theirQueues[theirSerial] = queue;
			return;
		}
	}
	else
	{
		m_theirQueues[theirSerial] = queue;
		return;
	}
}

bool MessageQueueBimap::RemoveQueue(uint32_t ourSerial)
{
	Lock lock(&m_queuesMutex);
	if(m_ourQueues.count(ourSerial) == 0)
	{
		return false;
	}

	//Get the serial number out of the MessageQueue, then delete it
	uint32_t theirSerial = m_ourQueues[ourSerial]->GetTheirSerialNum();
	delete m_ourQueues[ourSerial];
	m_ourQueues.erase(ourSerial);

	if(theirSerial == 0)
	{
		//Zero means "no serial number"
		//	So there's no need to do anything more here
		return true;
	}

	if(m_theirQueues.count(theirSerial) == 0)
	{
		cerr << "Missing MessageQueue in RemoveQueue(): Tried to remove MessageQueue but it didn't exist. Shouldn't happen.\n";
		return true;
	}

	m_theirQueues.erase(theirSerial);

	return true;
}

std::vector<uint32_t> MessageQueueBimap::GetUsedSerials()
{
	Lock lock(&m_queuesMutex);
	std::vector<uint32_t> serials;
	std::map<uint32_t, MessageQueue*>::iterator it;

	for(it = m_ourQueues.begin(); it != m_ourQueues.end(); ++it)
	{
		serials.push_back(it->first);
	}

	return serials;
}


}



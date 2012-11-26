//============================================================================
// Name        : MessageQueueBimap.h
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

#ifndef MESSAGEQUEUEBIMAP_H_
#define MESSAGEQUEUEBIMAP_H_

#include "MessageQueue.h"

#include <map>
#include "pthread.h"

namespace RTT
{

class MessageQueueBimap
{

public:

	MessageQueueBimap();
	~MessageQueueBimap();

	//Returns the MessageQueue for the given "our" serial number, NULL if it doesn't exist
	MessageQueue *GetByOurSerial(uint32_t ourSerial);

	//Returns the MessageQueue for the given "their" serial number, NULL if it doesn't exist
	//	NOTE: "Their" serial numbers are not always known, and therefore you should always prefer to call
	//		GetByOurSerial() if you know the "our" serial number. Only call this function if you really
	//		don't know what the "our" serial number that you want is.
	MessageQueue *GetByTheirSerial(uint32_t theirSerial);

	//Makes a new MessageQueue at the given serial number
	//	ourSerial - The "our" serial number to index by
	//	theirSerial - Optional parameter that will also index by the "their" serial number
	//	NOTE: You can call the latter AddQueue() a second time to add the new "their" serial number to the MessageQueue
	//	NOTE: Safely does nothing in the event that a MessageQueue already exists at the given serial number
	void AddQueue(uint32_t ourSerial);
	void AddQueue(uint32_t ourSerial, uint32_t theirSerial);

	//Removes and deletes the MessageQueue at the given "our" serial number
	//	ourSerial - The "our" serial number of the MessageQueue to delete
	//	returns - true on successful delettion, false on error or if the MessageQueue did not exist.
	//NOTE: Safely does nothing if no MessageQueue exists at ourSerial (but returns false)
	bool RemoveQueue(uint32_t ourSerial);

	std::vector<uint32_t> GetUsedSerials();

private:

	//Message queues, uniquely identified by their serial number
	std::map<uint32_t, MessageQueue*> m_ourQueues;
	std::map<uint32_t, MessageQueue*> m_theirQueues;

	pthread_mutex_t m_queuesMutex;
};

}

#endif /* MESSAGEQUEUEBIMAP_H_ */

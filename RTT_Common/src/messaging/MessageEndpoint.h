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

#ifndef MESSAGEENDPOINT_H_
#define MESSAGEENDPOINT_H_

#define MAX_CONSECUTIVE_MSG_TIMEOUTS 3

#include "MessageQueueBimap.h"
#include "messages/Message.h"
#include "Ticket.h"

#include "pthread.h"
#include <map>
#include <set>
#include <queue>
#include "event.h"

namespace RTT
{

class MessageEndpoint
{
public:

	//Constructor for MessageQueue
	//	socketFD - The socket file descriptor the queue will listen on
	//	direction - The protocol direction that is considered forward
	MessageEndpoint(int socketFD, struct bufferevent *bufferevent);

	//Destructor should only be called by the callback thread, and also only while
	//	the protocol lock in MessageManager is held. This is done to avoid
	//	race conditions in deleting the object.
	~MessageEndpoint();

	//Pop off a message from the specified direction
	//	ticket - Ticket containing necessary state information to retrieve a message
	//	timeout - How long (in seconds) to wait for the message before giving up
	// Returns - A pointer to a valid Message object. Never NULL. Caller is responsible for life cycle of this message
	//		On error, this function returns an ErrorMessage with the details of the error
	//		IE: Returns ErrorMessage of type ERROR_TIMEOUT if timeout has been exceeded
	//	NOTE: You must have the lock on the socket by calling UseSocket() prior to calling this
	//		(Or bad things will happen)
	//	NOTE: Blocking function
	//	NOTE: Will automatically call CloseSocket() for you if the message returned happens to be an ERROR_MESSAGE
	//		of type ERROR_SOCKET_CLOSED. So there is no need to call it again yourself
	//	NOTE: Due to physical constraints, this function may block for longer than timeout. Don't rely on it being very precise.
	Message *PopMessage(Ticket &ticket, int timeout);

	//Pushes a new message onto the appropriate message queue
	//	message - The Message to push
	//	NOTE: The direction of the message is read directly from the message itself
	bool PushMessage(Message *message);

	//Blocks until a new callback conversation has been started
	//	outTicket - Reference to an output Ticket object that will be set to the appropriate values to start a conversation
	//	returns true if a callback message is ready and waiting for us
	//			false if message queue has been closed and needs to be deleted
	bool RegisterCallback(Ticket &outTicket);

	//Starts a new conversation with the endpoint
	//	returns - The "our" serial number of the new conversation. 0 on error
	uint32_t StartConversation();

	//Shuts down all MessageQueues for this Endpoint, also wakes up any reading threads
	void Shutdown();

	//Deletes and removes the message queue indexed by the given ourSerial number
	bool RemoveMessageQueue(uint32_t ourSerial);

	bool m_isShutDown;

private:

	//Returns a new "our" serial number to use for a conversation. This is guaranteed to not currently be in use and not be 0
	//	This increments m_forwardSerialNumber
	//	NOTE: You must have the lock on the socket before using this. (Used inside UseSocket)
	uint32_t GetNextOurSerialNum();

	MessageQueueBimap m_queues;

	//The next serial number that will be given to a conversation
	uint32_t m_nextSerial;
	pthread_mutex_t m_nextSerialMutex;

	//A set of "their" serial numbers currently in use by the endpoint
	std::set<uint32_t> m_theirUsedSerials;
	pthread_mutex_t m_theirUsedSerialsMutex;

	//Maintains a queue of callback conversations that are ready to be processed
	std::queue<uint32_t> m_availableCBs;
	pthread_mutex_t m_availableCBsMutex;

	pthread_cond_t m_callbackWakeupCondition;

	//Marks the message queue as having been shut down. Just waiting to be destroyed properly
	pthread_mutex_t m_isShutdownMutex;

	int m_socketFD;
	uint8_t m_consecutiveTimeouts;

	pthread_mutex_t m_callbackRegisterMutex;
};

}

#endif /* MESSAGEENDPOINT_H_ */

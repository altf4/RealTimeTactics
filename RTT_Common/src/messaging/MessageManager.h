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

#ifndef MESSAGEMANAGER_H_
#define MESSAGEMANAGER_H_

#include "messages/Message.h"
#include "MessageQueue.h"
#include "MessageEndpoint.h"
#include "MessageEndpointLock.h"
#include "ServerCallback.h"
#include "../Lock.h"
#include "Ticket.h"

#include <map>
#include <vector>
#include "pthread.h"
#include "event.h"

namespace RTT
{

struct CallbackArg
{
	struct event_base *m_base;
	ServerCallback *m_callback;
};


class MessageManager
{

public:

	static MessageManager &Instance();

	//Grabs a message off of the given socket and returns a pointer to it
	//	ticket - Ticket object holding all the necessary conversation info to pop a new message
	//	timeout - How long (in seconds) to wait for the message before giving up
	// Returns - A pointer to a valid Message object. Never NULL. Caller is responsible for life cycle of this message
	//		On error, this function returns an ErrorMessage with the details of the error
	//		IE: Returns ErrorMessage of type ERROR_TIMEOUT if timeout has been exceeded
	//	NOTE: You can get a Ticket by calling StartConversation()
	//	NOTE: Blocking function
	//	NOTE: Due to physical constraints, this function may block for longer than timeout. Don't rely on it being very precise.
	RTT::Message *ReadMessage(Ticket &ticket, int timeout = REPLY_TIMEOUT);

	//Writes a given Message to the provided socket
	//	ticket - Ticket object holding all the necessary conversation info to send this message
	//	message - A pointer to the message object to send
	// Returns - true on successfully sending the object, false on error
	bool WriteMessage(const Ticket &ticket, Message *message);

	//Informs the message manager that you would like to use the specified socket.
	//	socketFD - The socket file descriptor to use
	//	returns - A Ticket object which contains all the information necessary to have a conversation
	//		on error, the Ticket object will be set with m_socketFD = -1
	Ticket StartConversation(int socketFD);

	//Initializes the socket and its underlying MessageQueues. Should be called prior to other socket operations on this socketFD
	//	Failing to call StartSocket prior to use will cause you to get get ErrorMessges (but not crash)
	//TODO: Maybe make the other functions automatically check and call this function for us. So we can make this private
	//	socketFD - The socket file descriptor for which to make the initialization
	//	bufferevent - The libevent structure used for i/o to this socket
	//NOTE: Safely does nothing if socketFD already exists in the manager
	void StartSocket(int socketFD, struct bufferevent *bufferevent);

	//Deletes the MessageQueue object to which socketFD belongs
	//	NOTE: Does not close the underlying socket.
	void DeleteEndpoint(int socketFD);

	//Waits for a new callback message to arrive on the given socketFD
	//	socketFD - The socket file descriptor to wait on
	//	outTicket - Output parameter which provides all the information necessary to talk on the new callback conversation
	//	NOTE: Blocking call
	bool RegisterCallback(int socketFD, Ticket &outTicket);

	//Gets a current list of all the open sockets in the manager
	//	Returns - A vector of socket file descriptors
	//	NOTE: No locks are provided for these sockets. You're just given the descriptors. This necessarily means that by the time you
	//		get around to trying to read/write to the socket, it might have been closed. Or a new socket might have appeared that won't
	//		be included in this list. You'll just have to deal with this fact.
	std::vector <int>GetSocketList();

	//Begins server accept() loop. Only run this function if you want to be a server (not a UI)
	//	callback - Pointer to a user defined ServerCallback object that contains the callback function to be run
	//	portNumber - The TCP port on which to bind the server
	//	NOTE: Blocking function. Begins the server main loop. Does not return.
	void StartServer(ServerCallback *callback, uint portNumber);

	//Function returns a read-locked MessageEndpoint
	//	socketFD - The socket file descriptor of the endpoint you want
	//	returns - An RAII MessageEndpointLock object that contains a read-locked MessageEndpoint
	//		on error, the m_endpint  pointer will be NULL
	MessageEndpointLock GetEndpoint(int socketFD);

	static void MessageDispatcher(struct bufferevent *bev, void *ctx);
	static void ErrorDispatcher(struct bufferevent *bev, short error, void *ctx);

private:

	static MessageManager *m_instance;

	//Constructor for MessageManager
	MessageManager();

	static void DoAccept(evutil_socket_t listener, short event, void *arg);

	static void *AcceptDispatcher(void *);

	std::map<int, std::pair<MessageEndpoint*, pthread_rwlock_t*>> m_endpoints;
	pthread_mutex_t m_endpointsMutex;

	pthread_mutex_t m_deleteEndpointMutex;;
};

}

#endif /* MESSAGEMANAGER_H_ */

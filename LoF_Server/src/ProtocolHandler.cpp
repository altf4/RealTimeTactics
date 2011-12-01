//============================================================================
// Name        : LoF_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Message class pertaining to client authentication
//============================================================================

#include "LoF_Server.h"
#include "ProtocolHandler.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>

using namespace std;
using namespace LoF;

bool LoF::AuthenticateNewClient(int ConnectFD)
{

	Message *message = ReadMessage(ConnectFD);

	if( message->type == CLIENT_HELLO )
	{

		//Send back a SERVER_HELLO with our options...
		AuthMessage *server_hello = new AuthMessage();
		server_hello->type = SERVER_HELLO;
		server_hello->serverVersion.major = SERVER_VERSION_MAJOR;
		server_hello->serverVersion.minor = SERVER_VERSION_MINOR;
		server_hello->serverVersion.rev = SERVER_VERSION_REV;

		if( WriteMessage(server_hello, ConnectFD) == false)
		{
			//Error in write
			delete server_hello;
			return false;
		}
		delete server_hello;

		//Wait for a CLIENT_AUTH
		AuthMessage *client_auth = (AuthMessage*)ReadMessage(ConnectFD);

		if( client_auth->type !=  CLIENT_AUTH)
		{
			//Error
			delete client_auth;
			return false;
		}

		//TODO: Do the "real" authentication here

		delete client_auth;

		//Send back a SERVER_AUTH_REPLY
		AuthMessage *server_auth_reply = new AuthMessage();
		server_auth_reply->type = SERVER_AUTH_REPLY;
		server_auth_reply->authSuccess = true;


		if( WriteMessage(server_auth_reply, ConnectFD) == false)
		{
			//Error in write
			delete server_auth_reply;
			return false;
		}
		delete server_auth_reply;

		return true;


	}
	else
	{
		cerr << "ERROR: Expected CLIENT_HELLO message, received: "
				<< message->type << "\n";
		delete message;
		shutdown(ConnectFD, SHUT_RDWR);
		close(ConnectFD);
		return false;
	}

	delete message;
	shutdown(ConnectFD, SHUT_RDWR);
	close(ConnectFD);

	//TODO: Right now, always return true (authentication success)
	return true;
}

Message *LoF::ReadMessage(int connectFD)
{
	//perform read operations ...
	char buff[4096];
	int bytesRead = 4096;
	vector <char> input;
	while( bytesRead < 4096)
	{
		bytesRead = read(connectFD, buff, 4096);
		if( bytesRead >= 0 )
		{
			input.insert(input.end(), buff, buff + bytesRead);
		}
		else
		{
			//Error in reading from socket
			cerr << "ERROR: Socket returned error...\n";
			return NULL;
		}
	}

	if(input.size() < MESSAGE_MIN_SIZE)
	{
		//Error, message too small
		cerr << "ERROR: Message received is too small, ignoring...\n";
		return NULL;
	}

	return Message::Deserialize(input.data(), input.size());

}

bool LoF::WriteMessage(Message *message, int connectFD)
{
	uint length;
	char *buffer = message->Serialize(&length);

	//TODO: Loop the write until it finishes?
	if( write(connectFD, buffer, length) == -1)
	{
		//Error
		cerr << "ERROR: Write function didn't finish...\n";
		free(buffer);
		return false;
	}
	free(buffer);
	return true;

}

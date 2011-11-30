//============================================================================
// Name        : LoF_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Code for handling socket IO on the server side
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

	Message *message = new Message(ConnectFD);

	if( message->type == CLIENT_HELLO )
	{

		//Send back a SERVER_HELLO with our options...
		Message *server_hello = new Message();
		server_hello->type = SERVER_HELLO;
		server_hello->serverVersion.major = SERVER_VERSION_MAJOR;
		server_hello->serverVersion.minor = SERVER_VERSION_MINOR;
		server_hello->serverVersion.rev = SERVER_VERSION_REV;

		char *outbuf;
		uint msg_size= server_hello->Serialize(&outbuf);
		write(ConnectFD, outbuf, msg_size);
		delete server_hello;
		free(outbuf);

		//Wait for a CLIENT_AUTH
		Message *client_auth = new Message(ConnectFD);

		if( client_auth->type !=  CLIENT_AUTH)
		{
			//Error
			return false;
			shutdown(ConnectFD, SHUT_RDWR);
			close(ConnectFD);
			return false;
		}

		//TODO: Do the "real" authentication here

		delete client_auth;

		//Send back a SERVER_AUTH_REPLY
		Message *server_auth_reply = new Message();
		server_auth_reply->type = SERVER_AUTH_REPLY;
		server_auth_reply->authSuccess = true;

		char *outbuf2;
		uint msg_size2= server_hello->Serialize(&outbuf2);
		write(ConnectFD, outbuf2, msg_size2);
		delete server_hello;
		free(outbuf2);

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


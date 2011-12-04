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
	//***************************
	// Read client Hello
	//***************************

	AuthMessage *message =(AuthMessage*) Message::ReadMessage(ConnectFD);

	if( message == NULL )
	{
		return false;
	}

	if( message->type != CLIENT_HELLO )
	{
		cerr << "ERROR: Expected CLIENT_HELLO message, received: "
				<< message->type << "\n";
		delete message;
		return false;
	}
	delete message;


	//***************************
	// Send Server Hello
	//***************************

	AuthMessage *server_hello = new AuthMessage();
	server_hello->type = SERVER_HELLO;
	server_hello->serverVersion.major = SERVER_VERSION_MAJOR;
	server_hello->serverVersion.minor = SERVER_VERSION_MINOR;
	server_hello->serverVersion.rev = SERVER_VERSION_REV;

	if(  Message::WriteMessage(server_hello, ConnectFD) == false)
	{
		//Error in write
		delete server_hello;
		return false;
	}
	delete server_hello;

	//***************************
	// Receive Client Auth
	//***************************

	AuthMessage *client_auth = (AuthMessage*)Message::ReadMessage(ConnectFD);
	if( client_auth == NULL )
	{
		//ERROR
		return false;
	}
	if( client_auth->type !=  CLIENT_AUTH)
	{
		//Error
		delete client_auth;
		return false;
	}

	//TODO: Do the "real" authentication here

	delete client_auth;


	//***************************
	// Send Server Auth Reply
	//***************************
	AuthMessage *server_auth_reply = new AuthMessage();
	server_auth_reply->type = SERVER_AUTH_REPLY;
	server_auth_reply->authSuccess = true;

	if(  Message::WriteMessage(server_auth_reply, ConnectFD) == false)
	{
		//Error in write
		delete server_auth_reply;
		return false;
	}
	delete server_auth_reply;
	//TODO: Right now, always return true (authentication success)
	return true;
}


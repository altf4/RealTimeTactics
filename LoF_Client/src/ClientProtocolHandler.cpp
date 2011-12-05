//============================================================================
// Name        : LoF_Client.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Code for handling socket IO on the server side
//============================================================================

#include "LoF_Client.h"
#include "ClientProtocolHandler.h"
#include "messages/AuthMessage.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>

using namespace std;
using namespace LoF;

bool LoF::AuthToServer(int connectFD)
{
	//***************************
	// Send client Hello
	//***************************

	AuthMessage *client_hello = new AuthMessage();
	client_hello->type = CLIENT_HELLO;
	client_hello->softwareVersion.major = CLIENT_VERSION_MAJOR;
	client_hello->softwareVersion.minor = CLIENT_VERSION_MINOR;
	client_hello->softwareVersion.rev = CLIENT_VERSION_REV;

	if( Message::WriteMessage(client_hello, connectFD) == false)
	{
		//Error in write
		delete client_hello;
		return false;
	}
	delete client_hello;

	//***************************
	// Receive Server Hello
	//***************************

	AuthMessage *server_hello = (AuthMessage*)Message::ReadMessage(connectFD);
	if( server_hello == NULL)
	{
		return false;
	}
	if( server_hello->type == SERVER_AUTH_REPLY)
	{
		if(server_hello->authSuccess == false)
		{
			//Rejected by server
			cout << "Rejected by server. Probably incompatible software versions.\n";
		}
		else
		{
			//ERROR: Shouldn't get here. So assume some kind of error happened
			cerr << "ERROR: Protocol error. Server replied with wrong message...\n";
		}
		delete server_hello;
		return false;
	}
	if( server_hello->type != SERVER_HELLO)
	{
		delete server_hello;
		return false;
	}

	//Check version compatibility
	if ((server_hello->softwareVersion.major != CLIENT_VERSION_MAJOR) ||
		(server_hello->softwareVersion.minor != CLIENT_VERSION_MINOR) ||
		(server_hello->softwareVersion.rev != CLIENT_VERSION_REV) )
	{
		//Incompatible software versions.
		//The server should have caught this, though.

		delete server_hello;
		return false;
	}
	delete server_hello;


	//***************************
	// Send Client Auth
	//***************************

	AuthMessage *client_auth = new AuthMessage();
	client_auth->type = CLIENT_AUTH;

	if( Message::WriteMessage(client_auth, connectFD) == false)
	{
		//Error in write
		delete client_auth;
		return false;
	}
	delete client_auth;

	//***************************
	// Receive Server Auth Reply
	//***************************

	AuthMessage *server_auth_reply = (AuthMessage*)Message::ReadMessage(connectFD);
	if( server_auth_reply == NULL)
	{
		return false;
	}
	if( server_auth_reply->type != SERVER_AUTH_REPLY)
	{
		delete server_auth_reply;
		return false;
	}

	return true;
}




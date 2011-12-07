//============================================================================
// Name        : LoF_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Code for handling socket IO on the server side
//============================================================================

#include "LoF_Server.h"
#include "ServerProtocolHandler.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>

using namespace std;
using namespace LoF;

bool LoF::GetNewClient(int ConnectFD)
{
	//***************************
	// Read client Hello
	//***************************

	AuthMessage *client_hello =(AuthMessage*) Message::ReadMessage(ConnectFD);

	if( client_hello == NULL )
	{
		return false;
	}

	if( client_hello->type != CLIENT_HELLO )
	{
		cerr << "ERROR: Expected CLIENT_HELLO message, received: "
				<< client_hello->type << "\n";
		delete client_hello;
		return false;
	}

	//Check version compatibility
	if ((client_hello->softwareVersion.major != SERVER_VERSION_MAJOR) ||
		(client_hello->softwareVersion.minor != SERVER_VERSION_MINOR) ||
		(client_hello->softwareVersion.rev != SERVER_VERSION_REV) )
	{
		//If versions are not the same, send an error message to the client
		delete client_hello;
		cout << "Client Connected with bad software version.\n";

		//*********************************
		// Send Server Auth Reply (Error)
		//*********************************
		AuthMessage *server_auth_reply = new AuthMessage();
		server_auth_reply->type = SERVER_AUTH_REPLY;
		server_auth_reply->authSuccess = INCOMPATIBLE_SOFTWARE_VERSIONS;

		if(  Message::WriteMessage(server_auth_reply, ConnectFD) == false)
		{
			//Error in write
			cerr << "Wasn't able to send AUTH_REPLY to client to tell it about "
					"the bad incompatible software versions.\n";
		}
		delete server_auth_reply;
		return false;
	}
	delete client_hello;


	//***************************
	// Send Server Hello
	//***************************

	AuthMessage *server_hello = new AuthMessage();
	server_hello->type = SERVER_HELLO;
	server_hello->softwareVersion.major = SERVER_VERSION_MAJOR;
	server_hello->softwareVersion.minor = SERVER_VERSION_MINOR;
	server_hello->softwareVersion.rev = SERVER_VERSION_REV;

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

	enum AuthResult authresult =
			AuthenticateClient(client_auth->username, client_auth->hashedPassword);

	if( authresult == AUTH_SUCCESS)
	{
		Player *player = new Player(client_auth->username);
		playerList[client_auth->username] = player;
	}

	delete client_auth;


	//***************************
	// Send Server Auth Reply
	//***************************
	AuthMessage *server_auth_reply = new AuthMessage();
	server_auth_reply->type = SERVER_AUTH_REPLY;
	server_auth_reply->authSuccess = authresult;

	if(  Message::WriteMessage(server_auth_reply, ConnectFD) == false)
	{
		//Error in write
		delete server_auth_reply;
		return false;
	}

	delete server_auth_reply;

	if( authresult == AUTH_SUCCESS)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//Authenticates the given username/password with the server
//Checks that:
//	a) The username exists in the system
//	b) The given password hash is correct for the specified username
//	c) The username is unique on the server
enum AuthResult LoF::AuthenticateClient(char *username, unsigned char *hashedPassword)
{
	//TODO: Authenticate!

	//Check if the username exists in the active list
	if( playerList[username] != NULL)
	{
		return USERNAME_ALREADY_EXISTS;
	}

	//TODO: Check if the username exists in the non-active list (from file on disk)

	return AUTH_SUCCESS;
}

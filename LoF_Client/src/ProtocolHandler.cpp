//============================================================================
// Name        : LoF_Client.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Code for handling socket IO on the server side
//============================================================================

#include "LoF_Client.h"
#include "ProtocolHandler.h"
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
	if( server_hello->type != SERVER_HELLO)
	{
		delete server_hello;
		return false;
	}

	//TODO: Check versions and stuff

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




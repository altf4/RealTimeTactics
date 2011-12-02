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
	AuthMessage *client_hello = new AuthMessage();
	client_hello->type = CLIENT_HELLO;

	if( Message::WriteMessage(client_hello, connectFD) == false)
	{
		//Error in write
		delete client_hello;
		return false;
	}
	delete client_hello;

	return true;
}




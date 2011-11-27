//============================================================================
// Name        : LoF_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Code for handling socket IO on the server side
//============================================================================

#include "ProtocolHandler.h"
#include <unistd.h>
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
	//perform read operations ...
	char buff[1024];
	int bytesRead = 1024;
	vector <char> input;
	while( bytesRead < 1024)
	{
		bytesRead = read(ConnectFD, buff, 1024);
		if( bytesRead >= 0 )
		{
			input.insert(input.end(), buff, buff + bytesRead);
		}
		else
		{
			//Error in reading from socket
		}
	}

	Message *message;
	if(input.size() > MESSAGE_MIN_SIZE)
	{
		message = new Message(input.data(), input.size());
	}
	else
	{
		//Error, message too small
		cerr << "ERROR: Message received is too small, ignoring...\n";
		shutdown(ConnectFD, SHUT_RDWR);
		close(ConnectFD);
		return false;
	}

	if( message->type == CLIENT_HELLO )
	{

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

	shutdown(ConnectFD, SHUT_RDWR);
	close(ConnectFD);
}


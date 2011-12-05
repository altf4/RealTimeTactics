//============================================================================
// Name        : LoF_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : LoF Game Server
//============================================================================

#include <iostream>
#include "Unit.h"
#include "LoF_Server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <algorithm>
#include <pthread.h>
#include "ServerProtocolHandler.h"

using namespace std;
using namespace LoF;

int main(int argc, char **argv)
{
	pthread_t threadID;
	int c;
	uint serverPortNumber;

	bool portEntered = false;

	while ((c = getopt(argc, argv, ":p:")) != -1)
	{
		switch (c)
		{
			case 'p':
			{
				char *errString;
				serverPortNumber = strtol(optarg, &errString, 10);
				if( *errString != '\0' || optarg == '\0')
				{
					//Error occurred
					cerr << "You entered an invalid port number\n";
					cerr << Usage();
					exit(-1);
				}
				portEntered = true;
				break;
			}
			case '?':
			{
				cerr << Usage();
				exit(-1);
				break;
			}
		}
	}

	//Check that all parameters were entered
	if( !portEntered )
	{
		cerr << "You did not enter a port number\n";
		cerr << Usage();
		exit(-1);
	}

	//Set up the TCP socket
	struct sockaddr_in stSockAddr;
	int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(-1 == SocketFD)
	{
		perror("can not create socket");
		exit(EXIT_FAILURE);
	}
	int optval = 1;
	setsockopt(SocketFD, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

	memset(&stSockAddr, 0, sizeof(stSockAddr));

	stSockAddr.sin_family = AF_INET;
	stSockAddr.sin_port = htons(serverPortNumber);
	stSockAddr.sin_addr.s_addr = INADDR_ANY;

	if(-1 == bind(SocketFD,(struct sockaddr *)&stSockAddr, sizeof(stSockAddr)))
	{
		perror("error bind failed");
		close(SocketFD);
		exit(EXIT_FAILURE);
	}

	if(-1 == listen(SocketFD, 10))
	{
		perror("error listen failed");
		close(SocketFD);
		exit(EXIT_FAILURE);
	}

	//Main loop, just listens for new TCP connections and sends them off to ClientThread
	for(;;)
	{
		int ConnectFD = accept(SocketFD, NULL, NULL);

		if(0 > ConnectFD)
		{
			perror("error accept failed");
			close(SocketFD);
			exit(EXIT_FAILURE);
		}

		//Send the new connection off to another thread for handling
		pthread_create(&threadID, NULL, ClientThread, (void *) ConnectFD );
	}

	return 0;
}

void *ClientThread(void * parm)
{
	int ConnectFD = (int)parm;

	//First, authenticate the client
	if( AuthenticateNewClient(ConnectFD) == false )
	{
		cout << "ERROR: Authentication Failure\n";
		shutdown(ConnectFD, SHUT_RDWR);
		close(ConnectFD);
		return NULL;
	}

	cout << "Client Authenticated!\n";

	return NULL;
}

//Processes one round of combat. (Can consist of many actions triggered)
void ProcessRound(Match *match)
{

	//Step 1: Increment all the charges on the charging actions
	for(uint i = 0; i < match->chargingActions.size(); i++)
	{
		match->chargingActions[i]->currentCharge += match->chargingActions[i]->speed;
	}

	//Step 2: Move any finished actions over to chargedActions
	for(uint i = 0; i < match->chargingActions.size(); i++)
	{
		if( match->chargingActions[i]->currentCharge >= CHARGE_MAX )
		{
			//Move the Action over
			match->chargedActions.push_back(match->chargingActions[i]);
			//Delete it from this list
			match->chargingActions[i] = NULL;
			match->chargingActions.erase( match->chargingActions.begin()+i );
			//Move the index back, since we just erased one elements
			i--;
		}
	}

	//Step 3: Sort the new charged list according to execution order
	sort(match->chargedActions.front(),
			match->chargedActions.back(), Action::CompareActions);

	//Step 4: Execute each of the charged actions, one by one
	while(match->chargedActions.size() > 0)
	{
		match->chargedActions[0]->Execute();
		match->chargingActions.erase( match->chargingActions.begin() );

		//Re-sort the actions, since new ones might have been added
		sort(match->chargedActions.front(),
				match->chargedActions.back(), Action::CompareActions);
		//TODO: This is probably inefficient. Find a better way than re-sorting every time
	}

}

//Prints usage tips
string Usage()
{
	string out;

	out += "Line of Fire Server Usage:\n";
	out += "\t LoF_Server -p PORT\n\n";
	out += "\t -p PORT == TCP Port number to listen for connections on.\n";
	return out;
}


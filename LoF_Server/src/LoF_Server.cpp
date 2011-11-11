//============================================================================
// Name        : LoF_Server.cpp
// Author      : AltF4
// Copyright   : GNU GPLv3
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

using namespace std;


int main(int argc, char **argv)
{

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


	//Main loop
	for(;;)
	{
		int ConnectFD = accept(SocketFD, NULL, NULL);

		if(0 > ConnectFD)
		{
			perror("error accept failed");
			close(SocketFD);
			exit(EXIT_FAILURE);
		}

		char buff[1024];
		uint size = 1024;
		//perform read write operations ...
		read(ConnectFD,buff,size);

		cout << "Hey! Check this out: " << buff << "\n";

		shutdown(ConnectFD, SHUT_RDWR);

		close(ConnectFD);
	}

	return 0;
}

//Increments actions
void ProcessRound()
{

}

//Prints usage tips
string Usage()
{
	return "";
}


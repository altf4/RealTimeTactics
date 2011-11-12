//============================================================================
// Name        : LoF_Client.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : LoF Game Client
//============================================================================

#include <iostream>
#include "Unit.h"
#include "LoF_Client.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

using namespace std;

int main(int argc, char **argv)
{

	int c;
	uint serverPortNumber;

	bool portEntered = false;
	bool serverAddrEntered = false;

	struct sockaddr_in stSockAddr;
	int SocketFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	while ((c = getopt(argc, argv, ":p:s:")) != -1)
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
					exit(EXIT_FAILURE);
				}
				portEntered = true;
				break;
			}
			case 's':
			{
				int Res = inet_pton(AF_INET, optarg, &stSockAddr.sin_addr);
				if (0 > Res)
				{
					perror("error: first parameter is not a valid address family");
					close(SocketFD);
					exit(EXIT_FAILURE);
				}
				else if (0 == Res)
				{
					perror("char string (second parameter does not contain valid ipaddress)");
					close(SocketFD);
					exit(EXIT_FAILURE);
				}
				serverAddrEntered = true;
				break;
			}
			case '?':
			{
				cerr << Usage();
				exit(EXIT_FAILURE);
				break;
			}
		}
	}

	//Check that all parameters were entered
	if( !portEntered )
	{
		cerr << "You did not enter a port number\n";
		cerr << Usage();
		exit(EXIT_FAILURE);
	}
	if( ! serverAddrEntered )
	{
		cerr << "You did not enter a server address\n";
		cerr << Usage();
		exit(EXIT_FAILURE);
	}


	if (-1 == SocketFD)
	{
		perror("cannot create socket");
		exit(EXIT_FAILURE);
	}

	memset(&stSockAddr, 0, sizeof(stSockAddr));

	stSockAddr.sin_family = AF_INET;
	stSockAddr.sin_port = htons(serverPortNumber);


	if (-1 == connect(SocketFD, (struct sockaddr *)&stSockAddr, sizeof(stSockAddr)))
	{
		perror("connect failed");
		close(SocketFD);
		exit(EXIT_FAILURE);
	}

	/* perform read write operations ... */
	write(SocketFD, "xXx Test Message xXx", 21);

	shutdown(SocketFD, SHUT_RDWR);

	close(SocketFD);
	return 0;
}


//Prints usage tips
string Usage()
{
	return "";
}

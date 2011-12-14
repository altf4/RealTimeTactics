//============================================================================
// Name        : LoF_Client.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : LoF Game Client
//============================================================================

#include <iostream>
#include "Unit.h"
#include "LoF_Client.h"
#include "ClientProtocolHandler.h"
#include "messages/Message.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

using namespace std;
using namespace LoF;

int main(int argc, char **argv)
{

	int c;
	uint serverPortNumber;
	string username;

	bool portEntered = false;
	bool serverAddrEntered = false;
	bool usernameEntered = false;

	struct sockaddr_in stSockAddr;
	int SocketFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	while ((c = getopt(argc, argv, ":p:s:u:")) != -1)
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
			case 'u':
			{
				username = string(optarg);
				if( (username.size() < 1) || (username.size() > USERNAME_MAX_LENGTH))
				{
					//Error occurred
					cerr << "You entered an invalid username. "
							"Length must be between 1 and " << USERNAME_MAX_LENGTH
							<< ".\n";
					cerr << Usage();
					exit(EXIT_FAILURE);
				}
				usernameEntered = true;
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
	if( ! usernameEntered )
	{
		cerr << "You did not enter a username\n";
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

	//Get password
	unsigned char hash[SHA256_DIGEST_LENGTH];
	if( GetPasswordTerminal(hash) == false)
	{
		cerr << "ERROR: Something went wrong with entering your password.\n";
		close(SocketFD);
		exit(EXIT_FAILURE);
	}

	if( AuthToServer(SocketFD, username, hash) == false)
	{
		//Error
		cerr << "ERROR: Authentication to server failed.\n";
		shutdown(SocketFD, SHUT_RDWR);
		close(SocketFD);
		exit(EXIT_FAILURE);
	}

	cout << "Authentication to server succeeded!\n";

	char input;

	//Main loop
	while(true)
	{
		cout << "What would you like to do?\n";
		cout << "\t(1) List matches on the server\n";
		cout << "\t(2) Create a new match\n";
		cout << "\t(3) Join an existing match\n";
		cout << "\t(4) Exit the server\n";

		cin >> input;

		switch (input)
		{
			case '1':
			{
				break;
			}
			case '2':
			{
				break;
			}
			case '3':
			{
				break;
			}
			case '4':
			{
				if( ExitServer(SocketFD) )
				{
					cout << "Successfully exited server. See you later!\n";
					//Ugly, I know. But you can't double-break
					goto quit;
				}
				cout << "Failed to exit server cleanly. Try again?\n";
				break;
			}
			default:
			{
				cout << "Not an accepted option.\n";
				break;
			}
		}
	}

	quit:
	shutdown(SocketFD, SHUT_RDWR);
	close(SocketFD);
	return 0;
}


//Asks the user for their password over terminal
//Sets *hash to the SHA256 hash of the input password
bool GetPasswordTerminal(unsigned char *hash)
{
	//Get Password:

	//Disables terminal echo on input
    termios oldt;
    tcgetattr(STDIN_FILENO, &oldt);
    termios newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

	string password;
	cout << "Please enter your password: \n";
	getline( cin, password );

	//Put the terminal settings back
	newt.c_lflag |= ECHO;
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);

	//Do the hash
	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	SHA256_Update(&sha256, password.c_str(), password.length());
	SHA256_Final(hash, &sha256);

	return true;
}

//Prints usage tips
string Usage()
{
	string out;

	out += "Line of Fire Client Usage:\n";
	out += "\t LoF_Client -s SERVER_IP -p PORT\n\n";
	out += "\t -s SERVER_IP == IP address of the game server\n";
	out += "\t\t IE: \"127.0.0.1\" or \"192.168.1.110\", etc...\n";
	out += "\t -p PORT      == TCP Port number to contact on the server.\n";
	return out;
}

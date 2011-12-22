//============================================================================
// Name        : RTT_Client.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : RTT Game Client
//============================================================================

#include <iostream>
#include "Unit.h"
#include "RTT_Client_Terminal.h"
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
#include <openssl/sha.h>

using namespace std;
using namespace RTT;

int main(int argc, char **argv)
{

	int c;
	uint serverPortNumber;
	string username;
	string server_IP;

	bool portEntered = false;
	bool serverAddrEntered = false;
	bool usernameEntered = false;

	while ((c = getopt(argc, argv, ":p:s:u:")) != -1)
	{
		switch (c)
		{
			case 'p':
			{
				char *errString;
				serverPortNumber = strtoul(optarg, &errString, 10);
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
				struct sockaddr_in stSockAddr;
				server_IP = string(optarg);
				int Res = inet_pton(AF_INET, optarg, &stSockAddr.sin_addr);
				if (0 > Res)
				{
					perror("error: first parameter is not a valid address family");
					exit(EXIT_FAILURE);
				}
				else if (0 == Res)
				{
					perror("char string (second parameter does not contain valid ipaddress)");
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

	//Get password
	unsigned char hash[SHA256_DIGEST_LENGTH];
	if( GetPasswordTerminal(hash) == false)
	{
		cerr << "ERROR: Something went wrong with entering your password.\n";
		exit(EXIT_FAILURE);
	}

	int SocketFD = AuthToServer(server_IP, serverPortNumber, username, hash);
	if( SocketFD < 0)
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
				uint page = 0;
				cout << "What page of matches would you like to see?\n";

				if ( !(cin >> page) )
				{
					cout << "ERROR: Bad input\n";
					break;
				}
				struct MatchDescription results[MATCHES_PER_PAGE];
				uint numResults = ListMatches(SocketFD, page, results);
				if( numResults == 0 )
				{
					cout << "There are no matches on the server.\n";
					break;
				}

				cout << "The following matches are available on the server:\n";
				for(uint i = 0; i < numResults; i++)
				{
					cout << "ID: " << results[i].ID << "\n";
					cout << "MaxPlayers " << results[i].maxPlayers << "\n";
					cout << "Status " << results[i].status << "\n";
					cout << "\n";
				}

				break;
			}
			case '2':
			{
				struct MatchOptions options;
				cout << "How many players (max) should be allowed in the match? (2-8)\n";

				if ( !(cin >> options.maxPlayers) )
				{
					cout << "ERROR: Bad input\n";
					break;
				}
				if( !CreateMatch(SocketFD , options) )
				{
					cout << "ERROR: Failed to create match\n";
					break;
				}

				//TODO: Enter game lobby?
				cout << "Yay! You're in a match now! Just pretend until there's "
						"something fun here...\n";
				break;
			}
			case '3':
			{
				uint matchID = 0;
				cout << "What match would you like to join? (It's ID)\n";

				if ( !(cin >> matchID) )
				{
					cout << "ERROR: Bad input\n";
					break;
				}
				if( !JoinMatch(SocketFD , matchID) )
				{
					cout << "ERROR: Failed to join match\n";
					break;
				}

				//TODO: Enter game lobby?
				cout << "Yay! You're in a match now! Just pretend until there's "
						"something fun here...\n";

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
	out += "\t RTT_Client -s SERVER_IP -p PORT\n\n";
	out += "\t -s SERVER_IP == IP address of the game server\n";
	out += "\t\t IE: \"127.0.0.1\" or \"192.168.1.110\", etc...\n";
	out += "\t -p PORT      == TCP Port number to contact on the server.\n";
	return out;
}
//============================================================================
// Name        : RTT_Client.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Code for handling socket IO on the server side
//============================================================================

#ifndef PROTOCOLHANDLER_H_
#define PROTOCOLHANDLER_H_

#define CLIENT_VERSION_MAJOR	0
#define CLIENT_VERSION_MINOR	0
#define CLIENT_VERSION_REV		1

//TODO: Read this from config file
#define MATCHES_PER_PAGE 10

#include "Match.h"
#include "messages/LobbyMessage.h"
#include "messages/ErrorMessage.h"

namespace RTT
{

//Authenticates to the game server and sets us up into the lobby
//	Returns socket descriptor for the TCP connection. -1
int AuthToServer(string IPAddress, uint port,
		string username, unsigned char *hashedPassword);


//********************************************
//				Lobby Commands
//********************************************

//Informs the server that we want to exit
//	connectFD: Socket File descriptor of the server
//	Returns true if we get a successful acknowledgment back
bool ExitServer(int connectFD);

//Get a page of match descriptions from the server
//	Writes the data into the matchArray array.
//	connectFD: Socket File descriptor of the server
//	page: What page of results to ask for? (>0)
//	matchArray: An array of MatchDescription's, of length MATCHES_PER_PAGE
//	Returns: The number of descriptions actually found
uint ListMatches(int connectFD, uint page, MatchDescription *matchArray);

//Create a new Match on the server, and join that Match
//	connectFD: Socket File descriptor of the server
//	Returns: true if the match is created successfully
bool CreateMatch(int connectFD, struct MatchOptions options);

//Joins the match at the given ID
//	connectFD: Socket File descriptor of the server
//	matchID: The server's unique ID for the chosen match
//	Returns: true if the match is joined successfully
bool JoinMatch(int connectFD, uint matchID);

//Leaves the match at the given ID
//	connectFD: Socket File descriptor of the server
//	matchID: The server's unique ID for the chosen match
//	Returns: true if the match is left successfully
bool LeaveMatch(int connectFD);

//Asks the server for stats about its current state
//	connectFD: Socket File descriptor of the server
//	Returns: A ServerStats struct containing
struct ServerStats GetServerStats(int connectFD);

//Send a message of type Error to the client
void SendError(int connectFD, enum ErrorType errorType);

}

#endif /* PROTOCOLHANDLER_H_ */

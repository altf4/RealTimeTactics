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
//TODO: Read this from config file
#define MAX_PLAYERS_IN_MATCH 8

#include "MatchTypes.h"
#include "Map.h"
#include "Enums.h"
#include "messaging/messages/LobbyMessage.h"
#include "messaging/messages/ErrorMessage.h"
#include "messaging/Ticket.h"
#include "GameEvents.h"
#include "MatchLobbyEvents.h"

#include <string>
#include <vector>

namespace RTT
{

//********************************************
//				Authentication Commands
//********************************************

//Authenticates to the game server and sets us up into the lobby
//	Writes out to the given PlayerDescription struct, which is your description
//	Returns socket descriptor for the TCP connection. -1
int AuthToServer(std::string IPAddress, uint port,
		std::string username, unsigned char *hashedPassword, struct PlayerDescription *outDescr);


//********************************************
//				Lobby Commands
//********************************************

//Informs the server that we want to exit
//	Returns true if we get a successful acknowledgment back
bool ExitServer();

//Get a page of match descriptions from the server
//	Writes the data into the matchArray array.
//	page: What page of results to ask for? (>0)
//	matchArray: An array of MatchDescription's, of length MATCHES_PER_PAGE
//	Returns: The number of descriptions actually found
uint ListMatches(uint page, MatchDescription *matchArray);

//Create a new Match on the server, and join that Match
//	options: The match options needed to create a game
//  outMatchDesc: Output variable for the match description that is created
//	Returns: true if the match is created successfully
//	Should immediately follow with InitializeCallback()
bool CreateMatch(struct MatchOptions options, struct MatchDescription *outMatchDesc);

//Joins the match at the given ID
//	matchID: The server's unique ID for the chosen match
//  outMatchDesc: Output variable for the match description that is created
//	Returns: A vector of the player descriptions that are currently in the match
//	NOTE: This vector includes us.
//	NOTE: On error or failure, the vector will be set to be empty
//	NOTE: Should immediately follow with InitializeCallback()
std::vector<PlayerDescription> JoinMatch(uint, struct MatchDescription &);

//Leaves the match at the given ID
//	matchID: The server's unique ID for the chosen match
//	Returns: true if the match is left successfully
bool LeaveMatch();

//Asks the server for stats about its current state
//	Returns: A ServerStats struct containing
struct ServerStats GetServerStats();


//********************************************
//			Match Lobby Commands
//********************************************

//Change the given player's team to another team
//	Must be the leader to change someone else's team
//	Returns true if successfully changed
bool ChangeTeam(uint playerID, enum TeamNumber team);

//Change the given player's color to another color
//	Must be the leader to change someone else's color
//	Returns true if successfully changed
bool ChangeColor(uint playerID, enum TeamColor color);

//Change the current map to a new one
//	Must be the leader
//	Returns true if successfully changed
bool ChangeMap(struct MapDescription map);

//Change the current GameSpeed to a new one
//	Must be the leader
//	Returns true if successfully changed
bool ChangeSpeed(enum GameSpeed speed);

//Change the match's victory condition to a new one
//	Must be the leader
//	Returns true if successfully changed
bool ChangeVictoryCondition(enum VictoryCondition victory);

//Give another player in the match the leader permissions
//	Must be the leader
//	Returns true if the leader status successfully given
bool ChangeLeader(uint newLeaderID);

//Kick the given player from the match
//	Must be the leader
//	Returns true if successfully kicked
bool KickPlayer(uint PlayerID);

//tell the server we wish to start the math we're currently in
//	Must be the leader
//	Returns true if the match successfully started
bool StartMatch();


//********************************************
//			Send Error Message
//********************************************

//Send a message of type Error to the client
void SendError(Ticket &ticket, enum ErrorType errorType);


//********************************************
//			Connection Commands
//********************************************

void Disconnect();


//********************************************
//			Callback Processing
//********************************************

//Processes and executes a single Main Lobby Event from the server
//	returns - The new state of the client as it leaves the function
enum LobbyReturn ProcessMainLobbyEvent(Ticket &ticket);

//Processes and executes a single Match Lobby Event from the server
//	returns - The new state of the client as it leaves the function
enum LobbyReturn ProcessMatchLobbyEvent(Ticket &ticket, MatchLobbyEvents *gameContext);

//Processes and executes a single Game Event from the server
//	returns - The new state of the client as it leaves the function
enum LobbyReturn ProcessGameEvent(Ticket &ticket, GameEvents *gameContext);

}

#endif /* PROTOCOLHANDLER_H_ */

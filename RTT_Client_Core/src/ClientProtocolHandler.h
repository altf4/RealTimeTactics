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

#include "Match.h"
#include "Map.h"
#include "messaging/messages/LobbyMessage.h"
#include "messaging/messages/ErrorMessage.h"

namespace RTT
{

enum CallbackType
{
	TEAM_CHANGE,
	COLOR_CHANGE,
	MAP_CHANGE,
	SPEED_CHANGE,
	VICTORY_CHANGE,
	PLAYER_LEFT,
	KICKED,
	PLAYER_JOINED,
	LEADER_CHANGE,
	MATCH_STARTED,
	CALLBACK_CLOSED,
	CALLBACK_ERROR,
};
struct CallbackChange
{
	//Type, which one of the following is used:
	enum CallbackType m_type;

	//Which of the following is used?

	uint m_playerID; //several

	//TEAM_CHANGE
	enum TeamNumber m_team;

	//COLOR_CHANGE
	enum TeamColor m_color;

	//MAP_CHANGE
	struct MapDescription m_mapDescription;

	//SPEED_CHANGE
	enum GameSpeed m_speed;

	//VICTORY_CHANGE
	enum VictoryCondition m_victory;

	//PLAYER_LEFT
	uint m_newLeaderID;

	//KICKED (nothing)

	//PLAYER_JOINED
	struct PlayerDescription m_playerDescription;

};

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
//	descPtr: The address of a pointer to PlayerDescription.
//    The current players in the match are given here
//  outMatchDesc: Output variable for the match description that is created

//	Returns: true if the match is joined successfully
//	Should immediately follow with InitializeCallback()
uint JoinMatch(uint matchID, PlayerDescription *descPtr,
		struct MatchDescription *outMatchDesc);

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
//			MatchLobby Callback
//********************************************

//Process a Callback command from the server
//	These are notifications sent by the server that an event has occurred
//	We listen for these messages on a different socket than
struct CallbackChange ProcessCallbackCommand();


//********************************************
//			Send Error Message
//********************************************

//Send a message of type Error to the client
void SendError(int socket, enum ErrorType errorType, enum ProtocolDirection direction);

//********************************************
//			Connection Commands
//********************************************

}

#endif /* PROTOCOLHANDLER_H_ */

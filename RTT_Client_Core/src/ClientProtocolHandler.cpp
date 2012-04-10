//============================================================================
// Name        : RTT_Client.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Code for handling socket IO on the server side
//============================================================================

#include "ClientProtocolHandler.h"
#include "messages/AuthMessage.h"
#include "messages/MatchLobbyMessage.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <linux/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <string.h>

#define SHA256_DIGEST_LENGTH 32
#define IPC_FILE_PATH "/usr/share/RTT/startingKey"

using namespace std;
using namespace RTT;

int connectFD, callbackSocket;
string serverIP;
struct PlayerDescription myPlayerDescription;
uint callbackPort = 0;

int RTT::AuthToServer(string IPAddress, uint port,
		string username, unsigned char *hashedPassword, struct PlayerDescription *outDescr)
{
	callbackPort = port + 1;
	struct sockaddr_in stSockAddr;
	serverIP = IPAddress;

	//Make a socket
	connectFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (-1 == connectFD)
	{
		perror("cannot create socket");
		return -1;
	}

	//Zero out the socket struct
	memset(&stSockAddr, 0, sizeof(stSockAddr));

	//Set sock type and port
	stSockAddr.sin_family = AF_INET;
	stSockAddr.sin_port = htons(port);

	//Set the IP address of the socket struct
	int Res = inet_pton(AF_INET, serverIP.c_str(), &stSockAddr.sin_addr);
	if (0 > Res)
	{
		perror("error: first parameter is not a valid address family");
		return -1;
	}
	else if (0 == Res)
	{
		perror("char string (second parameter does not contain valid ipaddress)");
		return -1;
	}

	if (-1 == connect(connectFD, (struct sockaddr *)&stSockAddr, sizeof(stSockAddr)))
	{
		perror("connect failed");
		close(connectFD);
		return -1;
	}


	//***************************
	// Send client Hello
	//***************************
	AuthMessage client_hello;
	client_hello.m_type = CLIENT_HELLO;
	client_hello.m_softwareVersion.m_major = CLIENT_VERSION_MAJOR;
	client_hello.m_softwareVersion.m_minor = CLIENT_VERSION_MINOR;
	client_hello.m_softwareVersion.m_rev = CLIENT_VERSION_REV;

	if( Message::WriteMessage(&client_hello, connectFD) == false)
	{
		//Error in write
		return -1;
	}

	//***************************
	// Receive Server Hello
	//***************************
	Message *server_hello_init = Message::ReadMessage(connectFD);
	if( server_hello_init == NULL)
	{
		SendError(connectFD, PROTOCOL_ERROR);
		return -1;
	}
	if( server_hello_init->m_type != SERVER_HELLO)
	{
		SendError(connectFD, PROTOCOL_ERROR);
		delete server_hello_init;
		return -1;
	}
	AuthMessage *server_hello = (AuthMessage*)server_hello_init;

	//Check version compatibility
	if ((server_hello->m_softwareVersion.m_major != CLIENT_VERSION_MAJOR) ||
		(server_hello->m_softwareVersion.m_minor != CLIENT_VERSION_MINOR) ||
		(server_hello->m_softwareVersion.m_rev != CLIENT_VERSION_REV) )
	{
		//Incompatible software versions.
		//The server should have caught this, though.

		SendError(connectFD, AUTHENTICATION_ERROR);
		delete server_hello_init;
		return -1;
	}
	delete server_hello_init;

	//***************************
	// Send Client Auth
	//***************************
	AuthMessage client_auth;
	client_auth.m_type = CLIENT_AUTH;
	strncpy(client_auth.m_username, username.data(), USERNAME_MAX_LENGTH);
	memcpy(client_auth.m_hashedPassword, hashedPassword, SHA256_DIGEST_LENGTH);

	if( Message::WriteMessage(&client_auth, connectFD) == false)
	{
		//Error in write
		return -1;
	}

	//***************************
	// Receive Server Auth Reply
	//***************************
	Message *server_auth_reply_init = Message::ReadMessage(connectFD);
	if( server_auth_reply_init == NULL)
	{
		return -1;
	}
	if( server_auth_reply_init->m_type != SERVER_AUTH_REPLY)
	{
		delete server_auth_reply_init;
		return -1;
	}
	AuthMessage *server_auth_reply = (AuthMessage*)server_auth_reply_init;

	if( server_auth_reply->m_authSuccess != AUTH_SUCCESS)
	{
		delete server_auth_reply;
		return -1;
	}

	myPlayerDescription = server_auth_reply->m_playerDescription;
	*outDescr = server_auth_reply->m_playerDescription;

	delete server_auth_reply;

	if( !InitializeCallback() )
	{
		cerr << "ERROR: Failed to initialize Callback\n";
		return -1;
	}

	return connectFD;
}

//Informs the server that we want to exit
//	connectFD: Socket File descriptor of the server
//	Returns true if we get a successful acknowledgment back
bool RTT::ExitServer()
{
	//********************************
	// Send Exit Server Notification
	//********************************
	LobbyMessage exit_server_notice;
	exit_server_notice.m_type = MATCH_EXIT_SERVER_NOTIFICATION;
	if( Message::WriteMessage(&exit_server_notice, connectFD) == false)
	{
		//Error in write
		return false;
	}

	//**********************************
	// Receive Exit Server Acknowledge
	//**********************************
	Message *exit_server_ack = Message::ReadMessage(connectFD);
	if( exit_server_ack == NULL)
	{
		return false;
	}
	if( exit_server_ack->m_type != MATCH_EXIT_SERVER_ACKNOWLEDGE)
	{
		delete exit_server_ack;
		return false;
	}

	return true;
}

//Get a page of match descriptions from the server
//	Writes the data into the matchArray array.
//	connectFD: Socket File descriptor of the server
//	page: What page of results to ask for? (>0)
//	matchArray: An array of MatchDescription's, of length MATCHES_PER_PAGE
//	Returns: The number of descriptions actually found
uint RTT::ListMatches(uint page, MatchDescription *matchArray)
{
	if(page < 1)
	{
		return 0;
	}

	//********************************
	// Send Match List Request
	//********************************
	LobbyMessage list_request;
	list_request.m_type = MATCH_LIST_REQUEST;
	list_request.m_requestedPage = page;
	if( Message::WriteMessage(&list_request, connectFD) == false)
	{
		//Error in write
		return 0;
	}

	//**********************************
	// Receive Match List Reply
	//**********************************
	Message *list_reply_init = Message::ReadMessage(connectFD);
	if( list_reply_init == NULL)
	{
		return 0;
	}
	if( list_reply_init->m_type != MATCH_LIST_REPLY)
	{
		delete list_reply_init;
		return 0;
	}
	LobbyMessage *list_reply = (LobbyMessage*)list_reply_init;
	if( list_reply->m_returnedMatchesCount > MATCHES_PER_PAGE)
	{
		delete list_reply;
		return 0;
	}

	//Copy each Match Description in
	for( uint i = 0; i < list_reply->m_returnedMatchesCount; i++ )
	{
		matchArray[i] = list_reply->m_matchDescriptions[i];
	}

	return list_reply->m_returnedMatchesCount;

}

//Create a new Match on the server, and join that Match
//	connectFD: Socket File descriptor of the server
//	Returns: true if the match is created successfully
bool RTT::CreateMatch(struct MatchOptions options, struct MatchDescription *outMatchDesc)
{
	//********************************
	// Send Match Create Request
	//********************************
	LobbyMessage create_request;
	create_request.m_type = MATCH_CREATE_REQUEST;
	if( Message::WriteMessage(&create_request, connectFD) == false)
	{
		//Error in write
		return false;
	}

	//**********************************
	// Receive Match Options Available
	//**********************************
	Message *ops_available_init = Message::ReadMessage(connectFD);
	if( ops_available_init == NULL)
	{
		return false;
	}
	if( ops_available_init->m_type != MATCH_CREATE_OPTIONS_AVAILABLE)
	{
		delete ops_available_init;
		return false;
	}

	LobbyMessage *ops_available = (LobbyMessage*)ops_available_init;
	if( (ops_available->m_options.m_maxPlayers < options.m_maxPlayers) ||
			(options.m_maxPlayers < 2))
	{
		return false;
	}

	//********************************
	// Send Match Create Request
	//********************************
	LobbyMessage ops_chosen;
	ops_chosen.m_type = MATCH_CREATE_OPTIONS_CHOSEN;
	ops_chosen.m_options = options;
	if( Message::WriteMessage(&ops_chosen, connectFD) == false)
	{
		//Error in write
		return false;
	}

	//**********************************
	// Receive Match Create Reply
	//**********************************
	Message *create_reply_init = Message::ReadMessage(connectFD);
	if( create_reply_init == NULL)
	{
		return false;
	}
	if( create_reply_init->m_type != MATCH_CREATE_REPLY)
	{
		delete create_reply_init;
		return false;
	}
	LobbyMessage *create_reply = (LobbyMessage*)create_reply_init;
	*outMatchDesc = create_reply->m_matchDescription;
	delete create_reply;
	return true;
}

//Joins the match at the given ID
//	connectFD: Socket File descriptor of the server
//	descPtr: The address of a pointer to PlayerDescription.
//		The current players in the match are given here
//	matchID: The server's unique ID for the chosen match
//	Returns: true if the match is joined successfully
uint RTT::JoinMatch(uint matchID, PlayerDescription *descPtr,
		struct MatchDescription *outMatchDesc)
{
	//********************************
	// Send Match Join Request
	//********************************
	LobbyMessage join_request;
	join_request.m_type = MATCH_JOIN_REQUEST;
	join_request.m_ID = matchID;
	if( Message::WriteMessage(&join_request, connectFD) == false)
	{
		//Error in write
		return 0;
	}

	//**********************************
	// Receive Match Join Reply
	//**********************************
	Message *join_reply_init = Message::ReadMessage(connectFD);
	if( join_reply_init == NULL)
	{
		return 0;
	}
	if( join_reply_init->m_type != MATCH_JOIN_REPLY)
	{
		delete join_reply_init;
		return 0;
	}
	LobbyMessage *join_reply = (LobbyMessage *)join_reply_init;
	uint count = join_reply->m_returnedPlayersCount;
	if(count > MAX_PLAYERS_IN_MATCH )
	{
		delete join_reply_init;
		return 0;
	}

	for(uint i = 0; i < count; i++ )
	{
		descPtr[i] = join_reply->m_playerDescriptions[i];
	}

	*outMatchDesc = join_reply->m_matchDescription;

	delete join_reply_init;
	return count;
}

//Leaves the match at the given ID
//	connectFD: Socket File descriptor of the server
//	matchID: The server's unique ID for the chosen match
//	Returns: true if the match is left cleanly
bool RTT::LeaveMatch()
{
	//********************************
	// Send Match Leave Notification
	//********************************
	MatchLobbyMessage leave_note;
	leave_note.m_type = MATCH_LEAVE_NOTIFICATION;
	if( Message::WriteMessage(&leave_note, connectFD) == false)
	{
		//Error in write
		return false;
	}

	//**********************************
	// Receive Match Leave Acknowledge
	//**********************************
	Message *leave_ack = Message::ReadMessage(connectFD);
	if( leave_ack == NULL)
	{
		return false;
	}
	if( leave_ack->m_type != MATCH_LEAVE_ACKNOWLEDGE)
	{
		delete leave_ack;
		return false;
	}
	delete leave_ack;
	return true;
}

//Asks the server for stats about its current state
//	connectFD: Socket File descriptor of the server
//	Returns: A ServerStats struct containing
struct ServerStats RTT::GetServerStats()
{
	struct ServerStats stats;
	stats.m_numPlayers = 0;
	stats.m_numMatches = 0;

	//********************************
	// Send Server Stats Request
	//********************************
	LobbyMessage server_stats_req;
	server_stats_req.m_type = SERVER_STATS_REQUEST;
	if( Message::WriteMessage(&server_stats_req, connectFD) == false)
	{
		//Error in write
		return stats;
	}

	//**********************************
	// Receive Server Stats Reply
	//**********************************
	Message *msg_init = Message::ReadMessage(connectFD);
	if( msg_init == NULL)
	{
		return stats;
	}
	if( msg_init->m_type != SERVER_STATS_REPLY)
	{
		delete msg_init;
		return stats;
	}
	LobbyMessage *server_stats_reply = (LobbyMessage*)msg_init;
	stats = server_stats_reply->m_serverStats;

	delete server_stats_reply;
	return stats;
}

//Change the given player's team to another team
//	Must be the leader to change someone else's team
//	Returns true if successfully changed
bool RTT::ChangeTeam(uint playerID, enum TeamNumber team)
{
	//********************************
	// Send Change Team Request
	//********************************
	MatchLobbyMessage change_team_req;
	change_team_req.m_type = CHANGE_TEAM_REQUEST;
	change_team_req.m_playerID = playerID;
	change_team_req.m_newTeam = team;
	if( Message::WriteMessage(&change_team_req, connectFD) == false)
	{
		//Error in write
		return false;
	}

	//**********************************
	// Receive Change Team Reply
	//**********************************
	Message *message = Message::ReadMessage(connectFD);
	if( message == NULL)
	{
		return false;
	}
	if( message->m_type != CHANGE_TEAM_REPLY)
	{
		delete message;
		return false;
	}
	MatchLobbyMessage *change_team_reply = (MatchLobbyMessage*)message;
	if( change_team_reply->m_changeAccepted )
	{
		delete change_team_reply;
		return true;
	}

	delete change_team_reply;
	return false;
}

//Change the given player's color to another color
//	Must be the leader to change someone else's color
//	Returns true if successfully changed
bool RTT::ChangeColor(uint playerID, enum TeamColor color)
{
	//********************************
	// Send Change Color Request
	//********************************
	MatchLobbyMessage change_color_req;
	change_color_req.m_type = CHANGE_COLOR_REQUEST;
	change_color_req.m_playerID = playerID;
	change_color_req.m_newColor = color;
	if( Message::WriteMessage(&change_color_req, connectFD) == false)
	{
		//Error in write
		return false;
	}

	//**********************************
	// Receive Change Color Reply
	//**********************************
	Message *message = Message::ReadMessage(connectFD);
	if( message == NULL)
	{
		return false;
	}
	if( message->m_type != CHANGE_COLOR_REPLY)
	{
		delete message;
		return false;
	}
	MatchLobbyMessage *change_color_reply = (MatchLobbyMessage*)message;
	if( change_color_reply->m_changeAccepted )
	{
		delete change_color_reply;
		return true;
	}

	delete change_color_reply;
	return false;
}

//Change the current map to a new one
//	Must be the leader
//	Returns true if successfully changed
bool RTT::ChangeMap(struct MapDescription map)
{
	//********************************
	// Send Change Map Request
	//********************************
	MatchLobbyMessage change_map_req;
	change_map_req.m_type = CHANGE_MAP_REQUEST;
	change_map_req.m_mapDescription = map;
	if( Message::WriteMessage(&change_map_req, connectFD) == false)
	{
		//Error in write
		return false;
	}

	//**********************************
	// Receive Change Map Reply
	//**********************************
	Message *message = Message::ReadMessage(connectFD);
	if( message == NULL)
	{
		return false;
	}
	if( message->m_type != CHANGE_MAP_REPLY)
	{
		delete message;
		return false;
	}
	MatchLobbyMessage *change_map_reply = (MatchLobbyMessage*)message;
	if( change_map_reply->m_changeAccepted )
	{
		delete change_map_reply;
		return true;
	}

	delete change_map_reply;
	return false;
}

//Change the current GameSpeed to a new one
//	Must be the leader
//	Returns true if successfully changed
bool RTT::ChangeSpeed(enum GameSpeed speed)
{
	//********************************
	// Send Change Speed Request
	//********************************
	MatchLobbyMessage change_speed_req;
	change_speed_req.m_type = CHANGE_GAME_SPEED_REQUEST;
	change_speed_req.m_newSpeed = speed;
	if( Message::WriteMessage(&change_speed_req, connectFD) == false)
	{
		//Error in write
		return false;
	}

	//**********************************
	// Receive Change Speed Reply
	//**********************************
	Message *message = Message::ReadMessage(connectFD);
	if( message == NULL)
	{
		return false;
	}
	if( message->m_type != CHANGE_GAME_SPEED_REPLY)
	{
		delete message;
		return false;
	}
	MatchLobbyMessage *change_speed_reply = (MatchLobbyMessage*)message;
	if( change_speed_reply->m_changeAccepted )
	{
		delete change_speed_reply;
		return true;
	}

	delete change_speed_reply;
	return false;
}

//Change the match's victory condition to a new one
//	Must be the leader
//	Returns true if successfully changed
bool RTT::ChangeVictoryCondition(enum VictoryCondition victory)
{
	//********************************
	// Send Change Victory Request
	//********************************
	MatchLobbyMessage change_victory_req;
	change_victory_req.m_type = CHANGE_VICTORY_COND_REQUEST;
	change_victory_req.m_newVictCond = victory;
	if( Message::WriteMessage(&change_victory_req, connectFD) == false)
	{
		//Error in write
		return false;
	}

	//**********************************
	// Receive Change Victory Reply
	//**********************************
	Message *message = Message::ReadMessage(connectFD);
	if( message == NULL)
	{
		return false;
	}
	if( message->m_type != CHANGE_VICTORY_COND_REPLY)
	{
		delete message;
		return false;
	}
	MatchLobbyMessage *change_victory_reply = (MatchLobbyMessage*)message;
	if( change_victory_reply->m_changeAccepted )
	{
		delete change_victory_reply;
		return true;
	}

	delete change_victory_reply;
	return false;
}

//Give another player in the match the leader permissions
//	Must be the leader
//	Returns true if the leader status successfully given
bool RTT::ChangeLeader(uint newLeaderID)
{
	//********************************
	// Send Change Leader Request
	//********************************
	MatchLobbyMessage change_leader_req;
	change_leader_req.m_type = CHANGE_LEADER_REQUEST;
	change_leader_req.m_playerID = newLeaderID;
	if( Message::WriteMessage(&change_leader_req, connectFD) == false)
	{
		//Error in write
		return false;
	}

	//**********************************
	// Receive Change Leader Reply
	//**********************************
	Message *message = Message::ReadMessage(connectFD);
	if( message == NULL)
	{
		return false;
	}
	if( message->m_type != CHANGE_LEADER_REPLY)
	{
		delete message;
		return false;
	}
	MatchLobbyMessage *change_leader_reply = (MatchLobbyMessage*)message;
	if( change_leader_reply->m_changeAccepted )
	{
		delete change_leader_reply;
		return true;
	}

	delete change_leader_reply;
	return false;
}

//Kick the given player from the match
//	Must be the leader
//	Returns true if successfully kicked
bool RTT::KickPlayer(uint PlayerID)
{
	//********************************
	// Send Kick Player Request
	//********************************
	MatchLobbyMessage kick_player_req;
	kick_player_req.m_type = KICK_PLAYER_REQUEST;
	kick_player_req.m_playerID = PlayerID;
	if( Message::WriteMessage(&kick_player_req, connectFD) == false)
	{
		//Error in write
		return false;
	}

	//**********************************
	// Receive Kick Player Reply
	//**********************************
	Message *message = Message::ReadMessage(connectFD);
	if( message == NULL)
	{
		return false;
	}
	if( message->m_type != KICK_PLAYER_REPLY)
	{
		delete message;
		return false;
	}
	MatchLobbyMessage *kick_player_reply = (MatchLobbyMessage*)message;
	if( kick_player_reply->m_changeAccepted )
	{
		delete kick_player_reply;
		return true;
	}

	delete kick_player_reply;
	return false;
}

//Start the match that we're currently in!
//	Must be the leader
//	Returns true if the match successfully started
bool RTT::StartMatch()
{
	//********************************
	// Send Start Match Request
	//********************************
	MatchLobbyMessage start_match_req;
	start_match_req.m_type = START_MATCH_REQUEST;
	if( Message::WriteMessage(&start_match_req, connectFD) == false)
	{
		//Error in write
		return false;
	}

	//**********************************
	// Receive Start Match Reply
	//**********************************
	Message *start_match_init = Message::ReadMessage(connectFD);
	if( start_match_init == NULL)
	{
		return false;
	}
	if( start_match_init->m_type != START_MATCH_REPLY)
	{
		delete start_match_init;
		return false;
	}
	MatchLobbyMessage *start_match_reply = (MatchLobbyMessage*)start_match_init;
	if( !start_match_reply->m_changeAccepted )
	{
		delete start_match_reply;
		return true;
	}
	delete start_match_reply;

	return RegisterForMatch();
}

bool RTT::RegisterForMatch()
{
	//********************************
	// Send Register for Match
	//********************************
	MatchLobbyMessage register_match;
	register_match.m_type = REGISTER_FOR_MATCH;
	if( Message::WriteMessage(&register_match, connectFD) == false)
	{
		//Error in write
		return false;
	}

	//**********************************
	// Receive Register Reply
	//**********************************
	Message *message = Message::ReadMessage(connectFD);
	if( message == NULL)
	{
		return false;
	}
	if( message->m_type != REGISTER_REPLY)
	{
		delete message;
		return false;
	}

	delete message;
	return true;
}


//********************************************
//			MatchLobby Callback
//********************************************

//Connect back to the server for Callback commands
//	Sets up the Callback socket, ready for commands
//	Returns if connected back successfully
//	Immediately follow with ProcessCallbackCommand()
bool RTT::InitializeCallback()
{
	//Make a new connection to the given port
	struct sockaddr_in stSockAddr;
	callbackSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (-1 == callbackSocket)
	{
		perror("cannot create socket");
		return false;
	}

	//Zero out the socket struct
	memset(&stSockAddr, 0, sizeof(stSockAddr));

	//Set sock type and port
	stSockAddr.sin_family = AF_INET;
	stSockAddr.sin_port = htons(callbackPort);

	//Set the IP address of the socket struct
	int Res = inet_pton(AF_INET, serverIP.c_str(), &stSockAddr.sin_addr);
	if (0 > Res)
	{
		perror("error: first parameter is not a valid address family");
		return false;
	}
	else if (0 == Res)
	{
		perror("char string (second parameter does not contain valid ipaddress)");
		return false;
	}

	if (-1 == connect(callbackSocket, (struct sockaddr *)&stSockAddr, sizeof(stSockAddr)))
	{
		perror("connect failed");
		close(callbackSocket);
		return false;
	}

	//***********************************
	// Send Callback Register
	//***********************************
	MatchLobbyMessage callback_register;
	callback_register.m_type = CALLBACK_REGISTER;
	callback_register.m_playerID = myPlayerDescription.m_ID;
	if( Message::WriteMessage(&callback_register, callbackSocket) == false)
	{
		//Error in write
		return false;
	}

	return true;
}

//Process a Callback command from the server
//	These are notifications sent by the server that an event has occurred
//	We listen for these messages on a different socket than
struct CallbackChange RTT::ProcessCallbackCommand()
{
	struct CallbackChange change;
	change.m_type = CALLBACK_ERROR;

	//**********************************
	// Receive Connect Back Ready
	//**********************************
	Message *message = Message::ReadMessage(callbackSocket);
	if( message == NULL)
	{
		return change;
	}
	MatchLobbyMessage *match_message = (MatchLobbyMessage*)message;

	switch(message->m_type)
	{
		case TEAM_CHANGED_NOTIFICATION:
		{
			//Get what we need from the message
			change.m_type = TEAM_CHANGE;
			change.m_playerID = match_message->m_playerID;
			change.m_team = match_message->m_newTeam;
			delete match_message;

			//***********************************
			// Send Team Changed Ack
			//***********************************
			MatchLobbyMessage team_change_ack;
			team_change_ack.m_type = TEAM_CHANGED_ACK;
			if( Message::WriteMessage(&team_change_ack, callbackSocket) == false)
			{
				//Error in write
				return change;
			}
			break;
		}
		case KICKED_FROM_MATCH_NOTIFICATION:
		{
			//Get what we need from the message
			change.m_type = KICKED;

			//***********************************
			// Send Kicked From Match Ack
			//***********************************
			MatchLobbyMessage kicked_ack;
			kicked_ack.m_type = KICKED_FROM_MATCH_ACK;
			delete match_message;
			if( Message::WriteMessage(&kicked_ack, callbackSocket) == false)
			{
				//Error in write
				return change;
			}

			break;
		}
		case PLAYER_LEFT_MATCH_NOTIFICATION:
		{
			//Get what we need from the message
			change.m_type = PLAYER_LEFT;
			change.m_playerID = match_message->m_playerID;
			change.m_newLeaderID = match_message->m_newLeaderID;
			delete match_message;

			//***********************************
			// Send Player Left Ack
			//***********************************
			MatchLobbyMessage player_left_ack;
			player_left_ack.m_type = PLAYER_LEFT_MATCH_ACK;
			if( Message::WriteMessage(&player_left_ack, callbackSocket) == false)
			{
				//Error in write
				return change;
			}
			break;
		}
		case PLAYER_JOINED_MATCH_NOTIFICATION:
		{
			//Get what we need from the message
			change.m_type = PLAYER_JOINED;
			change.m_playerDescription = match_message->m_playerDescription;
			delete match_message;

			//***********************************
			// Send Player Joined Ack
			//***********************************
			MatchLobbyMessage player_joined_ack;
			player_joined_ack.m_type = PLAYER_JOINED_MATCH_ACK;
			if( Message::WriteMessage(&player_joined_ack, callbackSocket) == false)
			{
				//Error in write
				return change;
			}
			break;
		}
		case COLOR_CHANGED_NOTIFICATION:
		{
			//Get what we need from the message
			change.m_type = COLOR_CHANGE;
			change.m_playerID = match_message->m_playerID;
			change.m_color = match_message->m_newColor;
			delete match_message;

			//***********************************
			// Send Color Changed Ack
			//***********************************
			MatchLobbyMessage color_change_ack;
			color_change_ack.m_type = COLOR_CHANGED_ACK;
			if( Message::WriteMessage(&color_change_ack, callbackSocket) == false)
			{
				//Error in write
				return change;
			}
			break;
		}
		case MAP_CHANGED_NOTIFICATION:
		{
			//Get what we need from the message
			change.m_type = MAP_CHANGE;
			change.m_mapDescription = match_message->m_mapDescription;
			delete match_message;

			//***********************************
			// Send Map Changed Ack
			//***********************************
			MatchLobbyMessage map_changed_ack;
			map_changed_ack.m_type = MAP_CHANGED_ACK;
			if( Message::WriteMessage(&map_changed_ack, callbackSocket) == false)
			{
				//Error in write
				return change;
			}
			break;
		}
		case GAME_SPEED_CHANGED_NOTIFICATION:
		{
			//Get what we need from the message
			change.m_type = SPEED_CHANGE;
			change.m_speed = match_message->m_newSpeed;
			delete match_message;

			//***********************************
			// Send Game Speed Changed Ack
			//***********************************
			MatchLobbyMessage speed_changed_ack;
			speed_changed_ack.m_type = GAME_SPEED_CHANGED_ACK;
			if( Message::WriteMessage(&speed_changed_ack, callbackSocket) == false)
			{
				//Error in write
				return change;
			}
			break;
		}
		case VICTORY_COND_CHANGED_NOTIFICATION:
		{
			//Get what we need from the message
			change.m_type = VICTORY_CHANGE;
			change.m_victory = match_message->m_newVictCond;
			delete match_message;

			//***********************************
			// Send Victory Condition Changed Ack
			//***********************************
			MatchLobbyMessage victory_changed_ack;
			victory_changed_ack.m_type = VICTORY_COND_CHANGED_ACK;
			if( Message::WriteMessage(&victory_changed_ack, callbackSocket) == false)
			{
				//Error in write
				return change;
			}
			break;
		}
		case CHANGE_LEADER_NOTIFICATION:
		{
			//Get what we need from the message
			change.m_type = LEADER_CHANGE;
			change.m_playerID = match_message->m_playerID;
			delete match_message;

			//***********************************
			// Send Victory Condition Changed Ack
			//***********************************
			MatchLobbyMessage leader_changed_ack;
			leader_changed_ack.m_type = CHANGE_LEADER_ACK;
			if( Message::WriteMessage(&leader_changed_ack, callbackSocket) == false)
			{
				//Error in write
				return change;
			}
			break;
		}
		case MATCH_START_NOTIFICATION:
		{
			//TODO: Must accept first

			//Get what we need from the message
			change.m_type = MATCH_STARTED;

			//***********************************
			// Send Match Started Ack
			//***********************************
			MatchLobbyMessage match_started_ack;
			match_started_ack.m_type = MATCH_START_ACK;
			match_started_ack.m_changeAccepted = true;
			delete match_message;

			if( Message::WriteMessage(&match_started_ack, callbackSocket) == false)
			{
				//Error in write
				return change;
			}

			//Indicates success of match starting
			change.m_type = MATCH_STARTED;

			break;
		}
		default:
		{
			delete match_message;
			cerr << "ERROR: Received a bad message on the callback socket\n";
			SendError(callbackSocket, AUTHENTICATION_ERROR);
			break;
		}
	}
	return change;
}


//********************************************
//			Send Error Message
//********************************************

//Send a message of type Error to the client
void  RTT::SendError(int socket, enum ErrorType errorType)
{
	ErrorMessage error_msg;
	error_msg.m_type = MESSAGE_ERROR;
	error_msg.m_errorType = errorType;
	if(  Message::WriteMessage(&error_msg, socket) == false)
	{
		cerr << "ERROR: Error message send returned failure.\n";
	}
}

//********************************************
//			Connection Commands
//********************************************

void RTT::ShutdownConnection()
{
	shutdown(connectFD, SHUT_RDWR);
	shutdown(callbackSocket, SHUT_RDWR);
	connectFD = -1;
	callbackSocket = -1;
}

int RTT::PassControlToGame()
{
	struct sockaddr_un address;
	int socket_fd;

	socket_fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if(socket_fd < 0)
	{
		cerr << "ERROR: socket() failed" << endl;
		return -1;
	}

	/* start with a clean address structure */
	memset(&address, 0, sizeof(struct sockaddr_un));

	address.sun_family = AF_UNIX;
	snprintf(address.sun_path, UNIX_PATH_MAX, IPC_FILE_PATH);

	if(connect(socket_fd,
			(struct sockaddr *) &address,
			sizeof(struct sockaddr_un)) != 0)
	{
		cerr << "ERROR: connect() failed" << endl;
		return -1;
	}

	struct msghdr socket_message;
	struct iovec io_vector[1];
	struct cmsghdr *control_message = NULL;
	char message_buffer[1];
	/* storage space needed for an ancillary element with a paylod of length is CMSG_SPACE(sizeof(length)) */
	char ancillary_element_buffer[CMSG_SPACE(sizeof(int))];

	/* at least one vector of one byte must be sent */
	message_buffer[0] = 'F';
	io_vector[0].iov_base = message_buffer;
	io_vector[0].iov_len = 1;

	/* initialize socket message */
	memset(&socket_message, 0, sizeof(struct msghdr));
	socket_message.msg_iov = io_vector;
	socket_message.msg_iovlen = 1;

	/* provide space for the ancillary data */
	int available_ancillary_element_buffer_space = CMSG_SPACE(sizeof(int));
	memset(ancillary_element_buffer, 0, available_ancillary_element_buffer_space);
	socket_message.msg_control = ancillary_element_buffer;
	socket_message.msg_controllen = available_ancillary_element_buffer_space;

	/* initialize a single ancillary data element for fd passing */
	control_message = CMSG_FIRSTHDR(&socket_message);
	control_message->cmsg_level = SOL_SOCKET;
	control_message->cmsg_type = SCM_RIGHTS;
	control_message->cmsg_len = CMSG_LEN(sizeof(int));
	*((int *) CMSG_DATA(control_message)) = connectFD;

	sendmsg(socket_fd, &socket_message, 0);
}

bool RTT::ReceiveControlFromSetup()
{
	struct sockaddr_un address;
	int socket_fd, connection_fd;
	socklen_t address_length;

	socket_fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if(socket_fd < 0)
	{
		printf("socket() failed\n");
		return 1;
	}

	unlink("./demo_socket");

	/* start with a clean address structure */
	memset(&address, 0, sizeof(struct sockaddr_un));

	address.sun_family = AF_UNIX;
	snprintf(address.sun_path, UNIX_PATH_MAX, IPC_FILE_PATH);

	if(bind(socket_fd, 	(struct sockaddr *) &address, 	sizeof(struct sockaddr_un)) != 0)
	{
		printf("bind() failed\n");
		return false;
	}

	if(listen(socket_fd, 5) != 0)
	{
		printf("listen() failed\n");
		return false;
	}

	connection_fd = accept(socket_fd, (struct sockaddr *) &address, &address_length);


	int sent_fd;
	struct msghdr socket_message;
	struct iovec io_vector[1];
	struct cmsghdr *control_message = NULL;
	char message_buffer[1];
	char ancillary_element_buffer[CMSG_SPACE(sizeof(int))];

	/* start clean */
	memset(&socket_message, 0, sizeof(struct msghdr));
	memset(ancillary_element_buffer, 0, CMSG_SPACE(sizeof(int)));

	/* setup a place to fill in message contents */
	io_vector[0].iov_base = message_buffer;
	io_vector[0].iov_len = 1;
	socket_message.msg_iov = io_vector;
	socket_message.msg_iovlen = 1;

	/* provide space for the ancillary data */
	socket_message.msg_control = ancillary_element_buffer;
	socket_message.msg_controllen = CMSG_SPACE(sizeof(int));

	if(recvmsg(connection_fd, &socket_message, MSG_CMSG_CLOEXEC) < 0)
	{
		return false;
	}

	if(message_buffer[0] != 'F')
	{
		/* this did not originate from the above function */
		return false;
	}

	if((socket_message.msg_flags & MSG_CTRUNC) == MSG_CTRUNC)
	{
		/* we did not provide enough space for the ancillary element array */
		return false;
	}

	/* iterate ancillary elements */
	for(control_message = CMSG_FIRSTHDR(&socket_message);
		control_message != NULL;
		control_message = CMSG_NXTHDR(&socket_message, control_message))
	{
		if( (control_message->cmsg_level == SOL_SOCKET) &&
				(control_message->cmsg_type == SCM_RIGHTS) )
		{
			sent_fd = *((int *) CMSG_DATA(control_message));
			return true;
		}
	}

	return false;

}

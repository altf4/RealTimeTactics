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
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <string.h>

#define SHA256_DIGEST_LENGTH 32

using namespace std;
using namespace RTT;

int connectFD, connectBackSocket;
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
	AuthMessage *client_hello = new AuthMessage();
	client_hello->type = CLIENT_HELLO;
	client_hello->softwareVersion.major = CLIENT_VERSION_MAJOR;
	client_hello->softwareVersion.minor = CLIENT_VERSION_MINOR;
	client_hello->softwareVersion.rev = CLIENT_VERSION_REV;

	if( Message::WriteMessage(client_hello, connectFD) == false)
	{
		//Error in write
		delete client_hello;
		return -1;
	}
	delete client_hello;

	//***************************
	// Receive Server Hello
	//***************************
	Message *server_hello_init = Message::ReadMessage(connectFD);
	if( server_hello_init == NULL)
	{
		SendError(connectFD, PROTOCOL_ERROR);
		return -1;
	}
	if( server_hello_init->type != SERVER_HELLO)
	{
		SendError(connectFD, PROTOCOL_ERROR);
		delete server_hello_init;
		return -1;
	}
	AuthMessage *server_hello = (AuthMessage*)server_hello_init;

	//Check version compatibility
	if ((server_hello->softwareVersion.major != CLIENT_VERSION_MAJOR) ||
		(server_hello->softwareVersion.minor != CLIENT_VERSION_MINOR) ||
		(server_hello->softwareVersion.rev != CLIENT_VERSION_REV) )
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
	AuthMessage *client_auth = new AuthMessage();
	client_auth->type = CLIENT_AUTH;
	strncpy( client_auth->username, username.data(), USERNAME_MAX_LENGTH);
	memcpy(client_auth->hashedPassword, hashedPassword, SHA256_DIGEST_LENGTH);

	if( Message::WriteMessage(client_auth, connectFD) == false)
	{
		//Error in write
		delete client_auth;
		return -1;
	}
	delete client_auth;

	//***************************
	// Receive Server Auth Reply
	//***************************
	Message *server_auth_reply_init = Message::ReadMessage(connectFD);
	if( server_auth_reply_init == NULL)
	{
		return -1;
	}
	if( server_auth_reply_init->type != SERVER_AUTH_REPLY)
	{
		delete server_auth_reply_init;
		return -1;
	}
	AuthMessage *server_auth_reply = (AuthMessage*)server_auth_reply_init;

	if( server_auth_reply->authSuccess != AUTH_SUCCESS)
	{
		delete server_auth_reply;
		return -1;
	}

	myPlayerDescription = server_auth_reply->playerDescription;
	*outDescr = server_auth_reply->playerDescription;

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
	LobbyMessage *exit_server_notice = new LobbyMessage();
	exit_server_notice->type = MATCH_EXIT_SERVER_NOTIFICATION;
	if( Message::WriteMessage(exit_server_notice, connectFD) == false)
	{
		//Error in write
		delete exit_server_notice;
		return false;
	}
	delete exit_server_notice;

	//**********************************
	// Receive Exit Server Acknowledge
	//**********************************
	Message *exit_server_ack = Message::ReadMessage(connectFD);
	if( exit_server_ack == NULL)
	{
		return false;
	}
	if( exit_server_ack->type != MATCH_EXIT_SERVER_ACKNOWLEDGE)
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
	LobbyMessage *list_request = new LobbyMessage();
	list_request->type = MATCH_LIST_REQUEST;
	list_request->requestedPage = page;
	if( Message::WriteMessage(list_request, connectFD) == false)
	{
		//Error in write
		delete list_request;
		return 0;
	}
	delete list_request;

	//**********************************
	// Receive Match List Reply
	//**********************************
	Message *list_reply_init = Message::ReadMessage(connectFD);
	if( list_reply_init == NULL)
	{
		return 0;
	}
	if( list_reply_init->type != MATCH_LIST_REPLY)
	{
		delete list_reply_init;
		return 0;
	}
	LobbyMessage *list_reply = (LobbyMessage*)list_reply_init;
	if( list_reply->returnedMatchesCount > MATCHES_PER_PAGE)
	{
		delete list_reply;
		return 0;
	}

	//Copy each Match Description in
	for( uint i = 0; i < list_reply->returnedMatchesCount; i++ )
	{
		matchArray[i] = list_reply->matchDescriptions[i];
	}

	return list_reply->returnedMatchesCount;

}

//Create a new Match on the server, and join that Match
//	connectFD: Socket File descriptor of the server
//	Returns: true if the match is created successfully
bool RTT::CreateMatch(struct MatchOptions options)
{
	//********************************
	// Send Match Create Request
	//********************************
	LobbyMessage *create_request = new LobbyMessage();
	create_request->type = MATCH_CREATE_REQUEST;
	if( Message::WriteMessage(create_request, connectFD) == false)
	{
		//Error in write
		delete create_request;
		return false;
	}
	delete create_request;

	//**********************************
	// Receive Match Options Available
	//**********************************
	Message *ops_available_init = Message::ReadMessage(connectFD);
	if( ops_available_init == NULL)
	{
		return false;
	}
	if( ops_available_init->type != MATCH_CREATE_OPTIONS_AVAILABLE)
	{
		delete ops_available_init;
		return false;
	}

	LobbyMessage *ops_available = (LobbyMessage*)ops_available_init;
	if( (ops_available->options.maxPlayers < options.maxPlayers) ||
			(options.maxPlayers < 2))
	{
		return false;
	}

	//********************************
	// Send Match Create Request
	//********************************
	LobbyMessage *ops_chosen = new LobbyMessage();
	ops_chosen->type = MATCH_CREATE_OPTIONS_CHOSEN;
	ops_chosen->options = options;
	if( Message::WriteMessage(ops_chosen, connectFD) == false)
	{
		//Error in write
		delete ops_chosen;
		return false;
	}
	delete ops_chosen;

	//**********************************
	// Receive Match Create Reply
	//**********************************
	Message *create_reply = Message::ReadMessage(connectFD);
	if( create_reply == NULL)
	{
		return false;
	}
	if( create_reply->type != MATCH_CREATE_REPLY)
	{
		delete create_reply;
		return false;
	}
	delete create_reply;

	return true;

}

//Joins the match at the given ID
//	connectFD: Socket File descriptor of the server
//	matchID: The server's unique ID for the chosen match
//	Returns: true if the match is joined successfully
bool RTT::JoinMatch(uint matchID)
{

	//********************************
	// Send Match Join Request
	//********************************
	LobbyMessage *join_request = new LobbyMessage();
	join_request->type = MATCH_JOIN_REQUEST;
	join_request->ID = matchID;
	if( Message::WriteMessage(join_request, connectFD) == false)
	{
		//Error in write
		delete join_request;
		return false;
	}
	delete join_request;

	//**********************************
	// Receive Match Join Reply
	//**********************************
	Message *join_reply = Message::ReadMessage(connectFD);
	if( join_reply == NULL)
	{
		return false;
	}
	if( join_reply->type != MATCH_JOIN_REPLY)
	{
		delete join_reply;
		return false;
	}
	delete join_reply;

	return true;
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
	MatchLobbyMessage *leave_note = new MatchLobbyMessage();
	leave_note->type = MATCH_LEAVE_NOTIFICATION;
	if( Message::WriteMessage(leave_note, connectFD) == false)
	{
		//Error in write
		delete leave_note;
		return false;
	}
	delete leave_note;

	//**********************************
	// Receive Match Leave Acknowledge
	//**********************************
	Message *leave_ack = Message::ReadMessage(connectFD);
	if( leave_ack == NULL)
	{
		return false;
	}
	if( leave_ack->type != MATCH_LEAVE_ACKNOWLEDGE)
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
	stats.numPlayers = 0;
	stats.numMatches = 0;

	//********************************
	// Send Server Stats Request
	//********************************
	LobbyMessage *server_stats_req = new LobbyMessage();
	server_stats_req->type = SERVER_STATS_REQUEST;
	if( Message::WriteMessage(server_stats_req, connectFD) == false)
	{
		//Error in write
		delete server_stats_req;
		return stats;
	}
	delete server_stats_req;

	//**********************************
	// Receive Server Stats Reply
	//**********************************
	Message *msg_init = Message::ReadMessage(connectFD);
	if( msg_init == NULL)
	{
		return stats;
	}
	if( msg_init->type != SERVER_STATS_REPLY)
	{
		delete msg_init;
		return stats;
	}
	LobbyMessage *server_stats_reply = (LobbyMessage*)msg_init;
	stats = server_stats_reply->serverStats;

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
	MatchLobbyMessage *change_team_req = new MatchLobbyMessage();
	change_team_req->type = CHANGE_TEAM_REQUEST;
	change_team_req->playerID = playerID;
	change_team_req->newTeam = team;
	if( Message::WriteMessage(change_team_req, connectFD) == false)
	{
		//Error in write
		delete change_team_req;
		return false;
	}
	delete change_team_req;

	//**********************************
	// Receive Change Team Reply
	//**********************************
	Message *message = Message::ReadMessage(connectFD);
	if( message == NULL)
	{
		return false;
	}
	if( message->type != CHANGE_TEAM_REPLY)
	{
		delete message;
		return false;
	}
	MatchLobbyMessage *change_team_reply = (MatchLobbyMessage*)message;
	if( change_team_reply->changeAccepted )
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
	MatchLobbyMessage *change_color_req = new MatchLobbyMessage();
	change_color_req->type = CHANGE_COLOR_REQUEST;
	change_color_req->playerID = playerID;
	change_color_req->newColor = color;
	if( Message::WriteMessage(change_color_req, connectFD) == false)
	{
		//Error in write
		delete change_color_req;
		return false;
	}
	delete change_color_req;

	//**********************************
	// Receive Change Color Reply
	//**********************************
	Message *message = Message::ReadMessage(connectFD);
	if( message == NULL)
	{
		return false;
	}
	if( message->type != CHANGE_COLOR_REPLY)
	{
		delete message;
		return false;
	}
	MatchLobbyMessage *change_color_reply = (MatchLobbyMessage*)message;
	if( change_color_reply->changeAccepted )
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
	MatchLobbyMessage *change_map_req = new MatchLobbyMessage();
	change_map_req->type = CHANGE_MAP_REQUEST;
	change_map_req->mapDescription = map;
	if( Message::WriteMessage(change_map_req, connectFD) == false)
	{
		//Error in write
		delete change_map_req;
		return false;
	}
	delete change_map_req;

	//**********************************
	// Receive Change Map Reply
	//**********************************
	Message *message = Message::ReadMessage(connectFD);
	if( message == NULL)
	{
		return false;
	}
	if( message->type != CHANGE_MAP_REPLY)
	{
		delete message;
		return false;
	}
	MatchLobbyMessage *change_map_reply = (MatchLobbyMessage*)message;
	if( change_map_reply->changeAccepted )
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
	MatchLobbyMessage *change_speed_req = new MatchLobbyMessage();
	change_speed_req->type = CHANGE_GAME_SPEED_REQUEST;
	change_speed_req->newSpeed = speed;
	if( Message::WriteMessage(change_speed_req, connectFD) == false)
	{
		//Error in write
		delete change_speed_req;
		return false;
	}
	delete change_speed_req;

	//**********************************
	// Receive Change Speed Reply
	//**********************************
	Message *message = Message::ReadMessage(connectFD);
	if( message == NULL)
	{
		return false;
	}
	if( message->type != CHANGE_GAME_SPEED_REPLY)
	{
		delete message;
		return false;
	}
	MatchLobbyMessage *change_speed_reply = (MatchLobbyMessage*)message;
	if( change_speed_reply->changeAccepted )
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
	MatchLobbyMessage *change_victory_req = new MatchLobbyMessage();
	change_victory_req->type = CHANGE_VICTORY_COND_REQUEST;
	change_victory_req->newVictCond = victory;
	if( Message::WriteMessage(change_victory_req, connectFD) == false)
	{
		//Error in write
		delete change_victory_req;
		return false;
	}
	delete change_victory_req;

	//**********************************
	// Receive Change Victory Reply
	//**********************************
	Message *message = Message::ReadMessage(connectFD);
	if( message == NULL)
	{
		return false;
	}
	if( message->type != CHANGE_VICTORY_COND_REPLY)
	{
		delete message;
		return false;
	}
	MatchLobbyMessage *change_victory_reply = (MatchLobbyMessage*)message;
	if( change_victory_reply->changeAccepted )
	{
		delete change_victory_reply;
		return true;
	}

	delete change_victory_reply;
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
	MatchLobbyMessage *kick_player_req = new MatchLobbyMessage();
	kick_player_req->type = KICK_PLAYER_REQUEST;
	kick_player_req->playerID = PlayerID;
	if( Message::WriteMessage(kick_player_req, connectFD) == false)
	{
		//Error in write
		delete kick_player_req;
		return false;
	}
	delete kick_player_req;

	//**********************************
	// Receive Kick Player Reply
	//**********************************
	Message *message = Message::ReadMessage(connectFD);
	if( message == NULL)
	{
		return false;
	}
	if( message->type != KICK_PLAYER_REPLY)
	{
		delete message;
		return false;
	}
	MatchLobbyMessage *kick_player_reply = (MatchLobbyMessage*)message;
	if( kick_player_reply->changeAccepted )
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
	MatchLobbyMessage *start_match_req = new MatchLobbyMessage();
	start_match_req->type = START_MATCH_REQUEST;
	if( Message::WriteMessage(start_match_req, connectFD) == false)
	{
		//Error in write
		delete start_match_req;
		return false;
	}
	delete start_match_req;

	//**********************************
	// Receive Start Match Reply
	//**********************************
	Message *message = Message::ReadMessage(connectFD);
	if( message == NULL)
	{
		return false;
	}
	if( message->type != START_MATCH_REPLY)
	{
		delete message;
		return false;
	}
	MatchLobbyMessage *start_match_reply = (MatchLobbyMessage*)message;
	if( start_match_reply->changeAccepted )
	{
		delete start_match_reply;
		return true;
	}

	delete start_match_reply;
	return false;
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
	connectBackSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (-1 == connectBackSocket)
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

	if (-1 == connect(connectBackSocket, (struct sockaddr *)&stSockAddr, sizeof(stSockAddr)))
	{
		perror("connect failed");
		close(connectBackSocket);
		return false;
	}

	//***********************************
	// Send Callback Register
	//***********************************
	MatchLobbyMessage *callback_register = new MatchLobbyMessage();
	callback_register->type = CALLBACK_REGISTER;
	callback_register->playerID = myPlayerDescription.ID;
	if( Message::WriteMessage(callback_register, connectBackSocket) == false)
	{
		//Error in write
		delete callback_register;
		return false;
	}
	delete callback_register;

	return true;
}

//Process a Callback command from the server
//	These are notifications sent by the server that an event has occurred
//	We listen for these messages on a different socket than
struct CallbackChange RTT::ProcessCallbackCommand()
{
	struct CallbackChange change;
	change.type = CALLBACK_ERROR;

	//**********************************
	// Receive Connect Back Ready
	//**********************************
	Message *message = Message::ReadMessage(connectBackSocket);
	if( message == NULL)
	{
		return change;
	}
	MatchLobbyMessage *match_message = (MatchLobbyMessage*)message;

	switch(message->type)
	{
		case TEAM_CHANGED_NOTIFICATION:
		{
			//Get what we need from the message
			change.type = TEAM_CHANGE;
			change.playerID = match_message->playerID;
			change.team = match_message->newTeam;

			//***********************************
			// Send Team Changed Ack
			//***********************************
			MatchLobbyMessage *team_change_ack = new MatchLobbyMessage();
			team_change_ack->type = TEAM_CHANGED_ACK;
			if( Message::WriteMessage(team_change_ack, connectBackSocket) == false)
			{
				//Error in write
				delete team_change_ack;
				delete match_message;
				return change;
			}
			delete team_change_ack;
			break;
		}
		case KICKED_FROM_MATCH_NOTIFICATION:
		{
			//Get what we need from the message
			change.type = KICKED;

			//***********************************
			// Send Kicked From Match Ack
			//***********************************
			MatchLobbyMessage *kicked_ack = new MatchLobbyMessage();
			kicked_ack->type = KICKED_FROM_MATCH_ACK;
			if( Message::WriteMessage(kicked_ack, connectBackSocket) == false)
			{
				//Error in write
				delete kicked_ack;
				delete match_message;
				return change;
			}

			delete kicked_ack;
			break;
		}
		case PLAYER_LEFT_MATCH_NOTIFICATION:
		{
			//Get what we need from the message
			change.type = PLAYER_LEFT;
			change.playerID = match_message->playerID;

			//***********************************
			// Send Player Left Ack
			//***********************************
			MatchLobbyMessage *player_left_ack = new MatchLobbyMessage();
			player_left_ack->type = PLAYER_LEFT_MATCH_ACK;
			if( Message::WriteMessage(player_left_ack, connectBackSocket) == false)
			{
				//Error in write
				delete player_left_ack;
				delete match_message;
				return change;
			}
			delete player_left_ack;
			break;
		}
		case PLAYER_JOINED_MATCH_NOTIFICATION:
		{
			//Get what we need from the message
			change.type = PLAYER_JOINED;
			change.playerDescription = match_message->playerDescription;

			//***********************************
			// Send Player Joined Ack
			//***********************************
			MatchLobbyMessage *player_joined_ack = new MatchLobbyMessage();
			player_joined_ack->type = PLAYER_JOINED_MATCH_ACK;
			if( Message::WriteMessage(player_joined_ack, connectBackSocket) == false)
			{
				//Error in write
				delete player_joined_ack;
				delete match_message;
				return change;
			}
			delete player_joined_ack;
			break;
		}
		case COLOR_CHANGED_NOTIFICATION:
		{
			//Get what we need from the message
			change.type = COLOR_CHANGE;
			change.playerID = match_message->playerID;
			change.color = match_message->newColor;

			//***********************************
			// Send Color Changed Ack
			//***********************************
			MatchLobbyMessage *color_change_ack = new MatchLobbyMessage();
			color_change_ack->type = COLOR_CHANGED_ACK;
			if( Message::WriteMessage(color_change_ack, connectBackSocket) == false)
			{
				//Error in write
				delete color_change_ack;
				delete match_message;
				return change;
			}
			delete color_change_ack;
			break;
		}
		case MAP_CHANGED_NOTIFICATION:
		{
			//Get what we need from the message
			change.type = MAP_CHANGE;
			change.mapDescription = match_message->mapDescription;

			//***********************************
			// Send Map Changed Ack
			//***********************************
			MatchLobbyMessage *map_changed_ack = new MatchLobbyMessage();
			map_changed_ack->type = MAP_CHANGED_ACK;
			if( Message::WriteMessage(map_changed_ack, connectBackSocket) == false)
			{
				//Error in write
				delete map_changed_ack;
				delete match_message;
				return change;
			}
			delete map_changed_ack;
			break;
		}
		case GAME_SPEED_CHANGED_NOTIFICATION:
		{
			//Get what we need from the message
			change.type = SPEED_CHANGE;
			change.speed = match_message->newSpeed;

			//***********************************
			// Send Game Speed Changed Ack
			//***********************************
			MatchLobbyMessage *speed_changed_ack = new MatchLobbyMessage();
			speed_changed_ack->type = GAME_SPEED_CHANGED_ACK;
			if( Message::WriteMessage(speed_changed_ack, connectBackSocket) == false)
			{
				//Error in write
				delete speed_changed_ack;
				delete match_message;
				return change;
			}
			delete speed_changed_ack;
			break;
		}
		case VICTORY_COND_CHANGED_NOTIFICATION:
		{
			//Get what we need from the message
			change.type = VICTORY_CHANGE;
			change.victory = match_message->newVictCond;

			//***********************************
			// Send Victory Condition Changed Ack
			//***********************************
			MatchLobbyMessage *victory_changed_ack = new MatchLobbyMessage();
			victory_changed_ack->type = VICTORY_COND_CHANGED_ACK;
			if( Message::WriteMessage(victory_changed_ack, connectBackSocket) == false)
			{
				//Error in write
				delete victory_changed_ack;
				delete match_message;
				return change;
			}
			delete victory_changed_ack;
			break;
		}
		case MATCH_START_NOTIFICATION:
		{
			//TODO: Must accept first

			//Get what we need from the message
			change.type = MATCH_STARTED;

			//***********************************
			// Send Match Started Ack
			//***********************************
			MatchLobbyMessage *match_started_ack = new MatchLobbyMessage();
			match_started_ack->type = MATCH_START_ACK;
			match_started_ack->changeAccepted = true;
			if( Message::WriteMessage(match_started_ack, connectBackSocket) == false)
			{
				//Error in write
				delete match_started_ack;
				delete match_message;
				return change;
			}
			delete match_started_ack;
			break;
		}
		default:
		{
			cerr << "ERROR: Received a bad message on the callback socket\n";
			SendError(connectBackSocket, AUTHENTICATION_ERROR);
			break;
		}
	}
	delete match_message;
	return change;
}


//********************************************
//			Send Error Message
//********************************************

//Send a message of type Error to the client
void  RTT::SendError(int socket, enum ErrorType errorType)
{
	ErrorMessage *error_msg = new ErrorMessage();
	error_msg->type = MESSAGE_ERROR;
	error_msg->errorType = errorType;
	if(  Message::WriteMessage(error_msg, socket) == false)
	{
		cerr << "ERROR: Error message send returned failure.\n";
	}
	delete error_msg;
}

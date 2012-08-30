//============================================================================
// Name        : RTT_Client.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Code for handling socket IO on the server side
//============================================================================

#include "ClientProtocolHandler.h"
#include "messaging/messages/AuthMessage.h"
#include "messaging/messages/MatchLobbyMessage.h"
#include "messaging/MessageManager.h"
#include "callback/MainLobbyCallbackChange.h"

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

int socketFD = -1;
string serverIP;
struct PlayerDescription myPlayerDescription;

int RTT::AuthToServer(string IPAddress, uint port,
		string username, unsigned char *hashedPassword, struct PlayerDescription *outDescr)
{
	struct sockaddr_in stSockAddr;
	serverIP = IPAddress;

	//Make a socket
	socketFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (-1 == socketFD)
	{
		perror("cannot create socket");
		return -1;
	}

	MessageManager::Instance().DeleteQueue(socketFD);
	Lock lock = MessageManager::Instance().UseSocket(socketFD);

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

	if (-1 == connect(socketFD, (struct sockaddr *)&stSockAddr, sizeof(stSockAddr)))
	{
		perror("connect failed");
		close(socketFD);
		return -1;
	}

	//***************************
	// Send client Hello
	//***************************
	AuthMessage client_hello(CLIENT_HELLO, DIRECTION_TO_SERVER);
	client_hello.m_softwareVersion.m_major = CLIENT_VERSION_MAJOR;
	client_hello.m_softwareVersion.m_minor = CLIENT_VERSION_MINOR;
	client_hello.m_softwareVersion.m_rev = CLIENT_VERSION_REV;

	if( Message::WriteMessage(&client_hello, socketFD) == false)
	{
		//Error in write
		return -1;
	}

	//***************************
	// Receive Server Hello
	//***************************
	Message *server_hello_init = Message::ReadMessage(socketFD, DIRECTION_TO_SERVER);
	if( server_hello_init->m_messageType != MESSAGE_AUTH)
	{
		SendError(socketFD, PROTOCOL_ERROR, DIRECTION_TO_SERVER);
		delete server_hello_init;
		return -1;
	}
	AuthMessage *server_hello = (AuthMessage*)server_hello_init;
	if(server_hello->m_authType != SERVER_HELLO)
	{
		SendError(socketFD, PROTOCOL_ERROR, DIRECTION_TO_SERVER);
		delete server_hello;
		return -1;
	}

	//Check version compatibility
	if ((server_hello->m_softwareVersion.m_major != CLIENT_VERSION_MAJOR) ||
		(server_hello->m_softwareVersion.m_minor != CLIENT_VERSION_MINOR) ||
		(server_hello->m_softwareVersion.m_rev != CLIENT_VERSION_REV) )
	{
		//Incompatible software versions.
		//The server should have caught this, though.

		SendError(socketFD, AUTHENTICATION_ERROR, DIRECTION_TO_SERVER);
		delete server_hello_init;
		return -1;
	}
	delete server_hello_init;

	//***************************
	// Send Client Auth
	//***************************
	AuthMessage client_auth(CLIENT_AUTH, DIRECTION_TO_SERVER);
	strncpy( client_auth.m_username, username.data(), USERNAME_MAX_LENGTH);
	memcpy(client_auth.m_hashedPassword, hashedPassword, SHA256_DIGEST_LENGTH);

	if( Message::WriteMessage(&client_auth, socketFD) == false)
	{
		//Error in write
		return -1;
	}

	//***************************
	// Receive Server Auth Reply
	//***************************
	Message *server_auth_reply_init = Message::ReadMessage(socketFD, DIRECTION_TO_SERVER);
	if( server_auth_reply_init->m_messageType != MESSAGE_AUTH)
	{
		delete server_auth_reply_init;
		return -1;
	}
	AuthMessage *server_auth_reply = (AuthMessage*)server_auth_reply_init;

	if( (server_auth_reply->m_authType != SERVER_AUTH_REPLY)
			|| (server_auth_reply->m_authSuccess != AUTH_SUCCESS))
	{
		delete server_auth_reply;
		return -1;
	}

	myPlayerDescription = server_auth_reply->m_playerDescription;
	*outDescr = server_auth_reply->m_playerDescription;

	delete server_auth_reply;

	return socketFD;
}

//Informs the server that we want to exit
//	connectFD: Socket File descriptor of the server
//	Returns true if we get a successful acknowledgment back
bool RTT::ExitServer()
{
	Lock lock = MessageManager::Instance().UseSocket(socketFD);

	//********************************
	// Send Exit Server Notification
	//********************************
	LobbyMessage exit_server_notice(MATCH_EXIT_SERVER_NOTIFICATION, DIRECTION_TO_SERVER);
	if( Message::WriteMessage(&exit_server_notice, socketFD) == false)
	{
		//Error in write
		return false;
	}

	//**********************************
	// Receive Exit Server Acknowledge
	//**********************************
	Message *exit_server_ack = Message::ReadMessage(socketFD, DIRECTION_TO_SERVER);
	if( exit_server_ack->m_messageType != MESSAGE_LOBBY)
	{
		delete exit_server_ack;
		return false;
	}
	LobbyMessage *lobby_ack = (LobbyMessage*)exit_server_ack;
	if(lobby_ack->m_lobbyType != MATCH_EXIT_SERVER_ACKNOWLEDGE)
	{
		delete lobby_ack;
		return false;
	}

	MessageManager::Instance().CloseSocket(socketFD);

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

	Lock lock = MessageManager::Instance().UseSocket(socketFD);

	//********************************
	// Send Match List Request
	//********************************
	LobbyMessage list_request(MATCH_LIST_REQUEST, DIRECTION_TO_SERVER);
	list_request.m_requestedPage = page;
	if( Message::WriteMessage(&list_request, socketFD) == false)
	{
		//Error in write
		return 0;
	}

	//**********************************
	// Receive Match List Reply
	//**********************************
	Message *list_reply_init = Message::ReadMessage(socketFD, DIRECTION_TO_SERVER);
	if( list_reply_init->m_messageType != MESSAGE_LOBBY)
	{
		delete list_reply_init;
		return 0;
	}
	LobbyMessage *list_reply = (LobbyMessage*)list_reply_init;
	if( list_reply->m_lobbyType != MATCH_LIST_REPLY)
	{
		delete list_reply;
		return 0;
	}
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
	Lock lock = MessageManager::Instance().UseSocket(socketFD);

	//********************************
	// Send Match Create Request
	//********************************
	LobbyMessage create_request(MATCH_CREATE_REQUEST, DIRECTION_TO_SERVER);
	if( Message::WriteMessage(&create_request, socketFD) == false)
	{
		//Error in write
		return false;
	}

	//**********************************
	// Receive Match Options Available
	//**********************************
	Message *ops_available_init = Message::ReadMessage(socketFD, DIRECTION_TO_SERVER);
	if( ops_available_init->m_messageType != MESSAGE_LOBBY)
	{
		delete ops_available_init;
		return false;
	}

	LobbyMessage *ops_available = (LobbyMessage*)ops_available_init;
	if(ops_available->m_lobbyType != MATCH_CREATE_OPTIONS_AVAILABLE)
	{
		delete ops_available;
		return false;
	}
	if( (ops_available->m_options.m_maxPlayers < options.m_maxPlayers) ||
			(options.m_maxPlayers < 2))
	{
		return false;
	}

	//********************************
	// Send Match Create Request
	//********************************
	LobbyMessage ops_chosen(MATCH_CREATE_OPTIONS_CHOSEN, DIRECTION_TO_SERVER);
	ops_chosen.m_options = options;
	if( Message::WriteMessage(&ops_chosen, socketFD) == false)
	{
		//Error in write
		return false;
	}

	//**********************************
	// Receive Match Create Reply
	//**********************************
	Message *create_reply_init = Message::ReadMessage(socketFD, DIRECTION_TO_SERVER);
	if( create_reply_init->m_messageType != MESSAGE_LOBBY)
	{
		delete create_reply_init;
		return false;
	}
	LobbyMessage *create_reply = (LobbyMessage*)create_reply_init;
	if(create_reply->m_lobbyType != MATCH_CREATE_REPLY)
	{
		delete create_reply;
		return false;
	}

	*outMatchDesc = create_reply->m_matchDescription;
	delete create_reply;
	return true;
}

//Joins the match at the given ID
//	matchID: The server's unique ID for the chosen match
//  outMatchDesc: Output variable for the match description that is created
//	Returns: A vector of the player descriptions that are currently in the match
//	NOTE: This vector includes us.
//	NOTE: On error or failure, the vector will be set to be empty
//	NOTE: Should immediately follow with InitializeCallback()
vector<PlayerDescription> RTT::JoinMatch(uint matchID,
		struct MatchDescription &outMatchDesc)
{
	Lock lock = MessageManager::Instance().UseSocket(socketFD);
	vector<PlayerDescription> retPlayers;
	retPlayers.clear();

	//********************************
	// Send Match Join Request
	//********************************
	LobbyMessage join_request(MATCH_JOIN_REQUEST, DIRECTION_TO_SERVER);
	join_request.m_ID = matchID;
	if( Message::WriteMessage(&join_request, socketFD) == false)
	{
		//Error in write
		return retPlayers;
	}

	//**********************************
	// Receive Match Join Reply
	//**********************************
	Message *join_reply_init = Message::ReadMessage(socketFD, DIRECTION_TO_SERVER);
	if( join_reply_init->m_messageType != MESSAGE_LOBBY)
	{
		delete join_reply_init;
		return retPlayers;
	}
	LobbyMessage *join_reply = (LobbyMessage *)join_reply_init;
	if(join_reply->m_lobbyType != MATCH_JOIN_REPLY)
	{
		delete join_reply;
		return retPlayers;
	}

	uint count = join_reply->m_returnedPlayersCount;
	if(count > MAX_PLAYERS_IN_MATCH )
	{
		delete join_reply_init;
		return retPlayers;
	}

	for(uint i = 0; i < count; i++ )
	{
		retPlayers.push_back(join_reply->m_playerDescriptions[i]);
	}

	outMatchDesc = join_reply->m_matchDescription;

	delete join_reply_init;
	return retPlayers;
}

//Leaves the match at the given ID
//	connectFD: Socket File descriptor of the server
//	matchID: The server's unique ID for the chosen match
//	Returns: true if the match is left cleanly
bool RTT::LeaveMatch()
{
	Lock lock = MessageManager::Instance().UseSocket(socketFD);

	//********************************
	// Send Match Leave Notification
	//********************************
	MatchLobbyMessage leave_note(MATCH_LEAVE_NOTIFICATION, DIRECTION_TO_SERVER);
	if( Message::WriteMessage(&leave_note, socketFD) == false)
	{
		//Error in write
		return false;
	}

	//**********************************
	// Receive Match Leave Acknowledge
	//**********************************
	Message *leave_ack = Message::ReadMessage(socketFD, DIRECTION_TO_SERVER);
	if( leave_ack->m_messageType != MESSAGE_MATCH_LOBBY)
	{
		delete leave_ack;
		return false;
	}
	MatchLobbyMessage *match_lobby_ack = (MatchLobbyMessage*)leave_ack;
	if(match_lobby_ack->m_matchLobbyType != MATCH_LEAVE_ACKNOWLEDGE)
	{
		delete match_lobby_ack;
		return false;
	}
	delete match_lobby_ack;
	return true;
}

//Asks the server for stats about its current state
//	connectFD: Socket File descriptor of the server
//	Returns: A ServerStats struct containing
struct ServerStats RTT::GetServerStats()
{
	Lock lock = MessageManager::Instance().UseSocket(socketFD);

	struct ServerStats stats;
	stats.m_numPlayers = 0;
	stats.m_numMatches = 0;

	//********************************
	// Send Server Stats Request
	//********************************
	LobbyMessage server_stats_req(SERVER_STATS_REQUEST, DIRECTION_TO_SERVER);
	if( Message::WriteMessage(&server_stats_req, socketFD) == false)
	{
		//Error in write
		return stats;
	}

	//**********************************
	// Receive Server Stats Reply
	//**********************************
	Message *msg_init = Message::ReadMessage(socketFD, DIRECTION_TO_SERVER);
	if( msg_init->m_messageType != MESSAGE_LOBBY)
	{
		delete msg_init;
		return stats;
	}
	LobbyMessage *server_stats_reply = (LobbyMessage*)msg_init;
	if(server_stats_reply->m_lobbyType != SERVER_STATS_REPLY)
	{
		delete server_stats_reply;
		return stats;
	}
	stats = server_stats_reply->m_serverStats;

	delete server_stats_reply;
	return stats;
}

//Change the given player's team to another team
//	Must be the leader to change someone else's team
//	Returns true if successfully changed
bool RTT::ChangeTeam(uint playerID, enum TeamNumber team)
{
	Lock lock = MessageManager::Instance().UseSocket(socketFD);

	//********************************
	// Send Change Team Request
	//********************************
	MatchLobbyMessage change_team_req(CHANGE_TEAM_REQUEST, DIRECTION_TO_SERVER);
	change_team_req.m_playerID = playerID;
	change_team_req.m_newTeam = team;
	if( Message::WriteMessage(&change_team_req, socketFD) == false)
	{
		//Error in write
		return false;
	}

	//**********************************
	// Receive Change Team Reply
	//**********************************
	Message *message = Message::ReadMessage(socketFD, DIRECTION_TO_SERVER);
	if( message->m_messageType != MESSAGE_MATCH_LOBBY)
	{
		delete message;
		return false;
	}
	MatchLobbyMessage *change_team_reply = (MatchLobbyMessage*)message;
	if(change_team_reply->m_matchLobbyType != CHANGE_TEAM_REPLY)
	{
		delete change_team_reply;
		return false;
	}
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
	Lock lock = MessageManager::Instance().UseSocket(socketFD);

	//********************************
	// Send Change Color Request
	//********************************
	MatchLobbyMessage change_color_req(CHANGE_COLOR_REQUEST, DIRECTION_TO_SERVER);
	change_color_req.m_playerID = playerID;
	change_color_req.m_newColor = color;
	if( Message::WriteMessage(&change_color_req, socketFD) == false)
	{
		//Error in write
		return false;
	}

	//**********************************
	// Receive Change Color Reply
	//**********************************
	Message *message = Message::ReadMessage(socketFD, DIRECTION_TO_SERVER);
	if( message->m_messageType != MESSAGE_MATCH_LOBBY)
	{
		delete message;
		return false;
	}
	MatchLobbyMessage *change_color_reply = (MatchLobbyMessage*)message;
	if(change_color_reply->m_matchLobbyType != CHANGE_COLOR_REPLY)
	{
		delete change_color_reply;
		return false;
	}
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
	Lock lock = MessageManager::Instance().UseSocket(socketFD);

	//********************************
	// Send Change Map Request
	//********************************
	MatchLobbyMessage change_map_req(CHANGE_MAP_REQUEST, DIRECTION_TO_SERVER);
	change_map_req.m_mapDescription = map;
	if( Message::WriteMessage(&change_map_req, socketFD) == false)
	{
		//Error in write
		return false;
	}

	//**********************************
	// Receive Change Map Reply
	//**********************************
	Message *message = Message::ReadMessage(socketFD, DIRECTION_TO_SERVER);
	if( message->m_messageType != MESSAGE_MATCH_LOBBY)
	{
		delete message;
		return false;
	}
	MatchLobbyMessage *change_map_reply = (MatchLobbyMessage*)message;
	if(change_map_reply->m_matchLobbyType != CHANGE_MAP_REPLY)
	{
		delete change_map_reply;
		return false;
	}
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
	Lock lock = MessageManager::Instance().UseSocket(socketFD);

	//********************************
	// Send Change Speed Request
	//********************************
	MatchLobbyMessage change_speed_req(CHANGE_GAME_SPEED_REQUEST, DIRECTION_TO_SERVER);
	change_speed_req.m_newSpeed = speed;
	if( Message::WriteMessage(&change_speed_req, socketFD) == false)
	{
		//Error in write
		return false;
	}

	//**********************************
	// Receive Change Speed Reply
	//**********************************
	Message *message = Message::ReadMessage(socketFD, DIRECTION_TO_SERVER);
	if( message->m_messageType != MESSAGE_MATCH_LOBBY)
	{
		delete message;
		return false;
	}
	MatchLobbyMessage *change_speed_reply = (MatchLobbyMessage*)message;
	if(change_speed_reply->m_matchLobbyType != CHANGE_GAME_SPEED_REPLY)
	{
		delete change_speed_reply;
		return false;
	}
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
	Lock lock = MessageManager::Instance().UseSocket(socketFD);

	//********************************
	// Send Change Victory Request
	//********************************
	MatchLobbyMessage change_victory_req(CHANGE_VICTORY_COND_REQUEST, DIRECTION_TO_SERVER);
	change_victory_req.m_newVictCond = victory;
	if( Message::WriteMessage(&change_victory_req, socketFD) == false)
	{
		//Error in write
		return false;
	}

	//**********************************
	// Receive Change Victory Reply
	//**********************************
	Message *message = Message::ReadMessage(socketFD, DIRECTION_TO_SERVER);
	if( message->m_messageType != MESSAGE_MATCH_LOBBY)
	{
		delete message;
		return false;
	}
	MatchLobbyMessage *change_victory_reply = (MatchLobbyMessage*)message;
	if(change_victory_reply->m_matchLobbyType != CHANGE_VICTORY_COND_REPLY)
	{
		delete change_victory_reply;
		return false;
	}
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
	Lock lock = MessageManager::Instance().UseSocket(socketFD);

	//********************************
	// Send Change Leader Request
	//********************************
	MatchLobbyMessage change_leader_req(CHANGE_LEADER_REQUEST, DIRECTION_TO_SERVER);
	change_leader_req.m_playerID = newLeaderID;
	if( Message::WriteMessage(&change_leader_req, socketFD) == false)
	{
		//Error in write
		return false;
	}

	//**********************************
	// Receive Change Leader Reply
	//**********************************
	Message *message = Message::ReadMessage(socketFD, DIRECTION_TO_SERVER);
	if( message->m_messageType != MESSAGE_MATCH_LOBBY)
	{
		delete message;
		return false;
	}
	MatchLobbyMessage *change_leader_reply = (MatchLobbyMessage*)message;
	if(change_leader_reply->m_matchLobbyType != CHANGE_LEADER_REPLY)
	{
		delete change_leader_reply;
		return false;
	}
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
	Lock lock = MessageManager::Instance().UseSocket(socketFD);

	//********************************
	// Send Kick Player Request
	//********************************
	MatchLobbyMessage kick_player_req(KICK_PLAYER_REQUEST, DIRECTION_TO_SERVER);
	kick_player_req.m_playerID = PlayerID;
	if( Message::WriteMessage(&kick_player_req, socketFD) == false)
	{
		//Error in write
		return false;
	}

	//**********************************
	// Receive Kick Player Reply
	//**********************************
	Message *message = Message::ReadMessage(socketFD, DIRECTION_TO_SERVER);
	if( message->m_messageType != MESSAGE_MATCH_LOBBY)
	{
		delete message;
		return false;
	}
	MatchLobbyMessage *kick_player_reply = (MatchLobbyMessage*)message;
	if(kick_player_reply->m_matchLobbyType != KICK_PLAYER_REPLY)
	{
		delete kick_player_reply;
		return false;
	}
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
	Lock lock = MessageManager::Instance().UseSocket(socketFD);

	//********************************
	// Send Start Match Request
	//********************************
	MatchLobbyMessage start_match_req(START_MATCH_REQUEST, DIRECTION_TO_SERVER);
	if( Message::WriteMessage(&start_match_req, socketFD) == false)
	{
		//Error in write
		return false;
	}

	//**********************************
	// Receive Start Match Reply
	//**********************************
	Message *message = Message::ReadMessage(socketFD, DIRECTION_TO_SERVER);
	if( message->m_messageType != MESSAGE_MATCH_LOBBY)
	{
		delete message;
		return false;
	}
	MatchLobbyMessage *start_match_reply = (MatchLobbyMessage*)message;
	if(start_match_reply->m_matchLobbyType != START_MATCH_REPLY)
	{
		delete start_match_reply;
		return false;
	}
	if( start_match_reply->m_changeAccepted )
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

//Process a Callback command from the server
//	These are notifications sent by the server that an event has occurred
//	We listen for these messages on a different socket than
CallbackChange *RTT::ProcessCallbackCommand()
{
	if(!MessageManager::Instance().RegisterCallback(socketFD))
	{
		return new CallbackChange(CALLBACK_CLOSED);
	}

	Message *message = Message::ReadMessage(socketFD, DIRECTION_TO_CLIENT);
	//TODO: Accept more than just MatchLobby callbacks!!!
	if( message->m_messageType != MESSAGE_MATCH_LOBBY)
	{
		delete message;
		return new CallbackChange(CALLBACK_ERROR);
	}
	MatchLobbyMessage *match_message = (MatchLobbyMessage*)message;

	switch(match_message->m_matchLobbyType)
	{
		case TEAM_CHANGED_NOTIFICATION:
		{
			//Get what we need from the message
			MainLobbyCallbackChange *change = new MainLobbyCallbackChange(TEAM_CHANGE);
			change->m_playerID = match_message->m_playerID;
			change->m_team = match_message->m_newTeam;

			//***********************************
			// Send Team Changed Ack
			//***********************************
			MatchLobbyMessage team_change_ack(TEAM_CHANGED_ACK, DIRECTION_TO_CLIENT);
			if( Message::WriteMessage(&team_change_ack, socketFD) == false)
			{
				//Error in write
				delete match_message;
				delete change;
				return new CallbackChange(CALLBACK_ERROR);
			}
			delete match_message;
			return change;
		}
		case KICKED_FROM_MATCH_NOTIFICATION:
		{
			//Get what we need from the message
			MainLobbyCallbackChange *change = new MainLobbyCallbackChange(KICKED);

			//***********************************
			// Send Kicked From Match Ack
			//***********************************
			MatchLobbyMessage kicked_ack(KICKED_FROM_MATCH_ACK, DIRECTION_TO_CLIENT);
			if( Message::WriteMessage(&kicked_ack, socketFD) == false)
			{
				//Error in write
				delete match_message;
				delete change;
				return new CallbackChange(CALLBACK_ERROR);
			}

			delete match_message;
			return change;
		}
		case PLAYER_LEFT_MATCH_NOTIFICATION:
		{
			//Get what we need from the message
			MainLobbyCallbackChange *change = new MainLobbyCallbackChange(PLAYER_LEFT);
			change->m_playerID = match_message->m_playerID;
			change->m_newLeaderID = match_message->m_newLeaderID;

			//***********************************
			// Send Player Left Ack
			//***********************************
			MatchLobbyMessage player_left_ack(PLAYER_LEFT_MATCH_ACK, DIRECTION_TO_CLIENT);
			if( Message::WriteMessage(&player_left_ack, socketFD) == false)
			{
				//Error in write
				delete match_message;
				delete change;
				return new CallbackChange(CALLBACK_ERROR);
			}
			delete match_message;
			return change;
		}
		case PLAYER_JOINED_MATCH_NOTIFICATION:
		{
			//Get what we need from the message
			MainLobbyCallbackChange *change = new MainLobbyCallbackChange(PLAYER_JOINED);
			change->m_playerDescription = match_message->m_playerDescription;

			//***********************************
			// Send Player Joined Ack
			//***********************************
			MatchLobbyMessage player_joined_ack(PLAYER_JOINED_MATCH_ACK, DIRECTION_TO_CLIENT);
			if( Message::WriteMessage(&player_joined_ack, socketFD) == false)
			{
				//Error in write
				delete match_message;
				delete change;
				return new CallbackChange(CALLBACK_ERROR);
			}
			delete match_message;
			return change;
		}
		case COLOR_CHANGED_NOTIFICATION:
		{
			//Get what we need from the message
			MainLobbyCallbackChange *change = new MainLobbyCallbackChange(COLOR_CHANGE);
			change->m_playerID = match_message->m_playerID;
			change->m_color = match_message->m_newColor;

			//***********************************
			// Send Color Changed Ack
			//***********************************
			MatchLobbyMessage color_change_ack(COLOR_CHANGED_ACK, DIRECTION_TO_CLIENT);
			if( Message::WriteMessage(&color_change_ack, socketFD) == false)
			{
				//Error in write
				delete match_message;
				delete change;
				return new CallbackChange(CALLBACK_ERROR);
			}
			delete match_message;
			return change;
		}
		case MAP_CHANGED_NOTIFICATION:
		{
			//Get what we need from the message
			MainLobbyCallbackChange *change = new MainLobbyCallbackChange(MAP_CHANGE);
			change->m_mapDescription = match_message->m_mapDescription;

			//***********************************
			// Send Map Changed Ack
			//***********************************
			MatchLobbyMessage map_changed_ack(MAP_CHANGED_ACK, DIRECTION_TO_CLIENT);
			if( Message::WriteMessage(&map_changed_ack, socketFD) == false)
			{
				//Error in write
				delete match_message;
				delete change;
				return new CallbackChange(CALLBACK_ERROR);
			}
			delete match_message;
			return change;
		}
		case GAME_SPEED_CHANGED_NOTIFICATION:
		{
			//Get what we need from the message
			MainLobbyCallbackChange *change = new MainLobbyCallbackChange(SPEED_CHANGE);
			change->m_speed = match_message->m_newSpeed;

			//***********************************
			// Send Game Speed Changed Ack
			//***********************************
			MatchLobbyMessage speed_changed_ack(GAME_SPEED_CHANGED_ACK, DIRECTION_TO_CLIENT);
			if( Message::WriteMessage(&speed_changed_ack, socketFD) == false)
			{
				//Error in write
				delete match_message;
				delete change;
				return new CallbackChange(CALLBACK_ERROR);
			}
			delete match_message;
			return change;
		}
		case VICTORY_COND_CHANGED_NOTIFICATION:
		{
			//Get what we need from the message
			MainLobbyCallbackChange *change = new MainLobbyCallbackChange(VICTORY_CHANGE);
			change->m_victory = match_message->m_newVictCond;

			//***********************************
			// Send Victory Condition Changed Ack
			//***********************************
			MatchLobbyMessage victory_changed_ack(VICTORY_COND_CHANGED_ACK, DIRECTION_TO_CLIENT);
			if( Message::WriteMessage(&victory_changed_ack, socketFD) == false)
			{
				//Error in write
				delete match_message;
				delete change;
				return new CallbackChange(CALLBACK_ERROR);
			}
			delete match_message;
			return change;
		}
		case CHANGE_LEADER_NOTIFICATION:
		{
			//Get what we need from the message
			MainLobbyCallbackChange *change = new MainLobbyCallbackChange(LEADER_CHANGE);
			change->m_playerID = match_message->m_playerID;

			//***********************************
			// Send Victory Condition Changed Ack
			//***********************************
			MatchLobbyMessage leader_changed_ack(CHANGE_LEADER_ACK, DIRECTION_TO_CLIENT);
			if( Message::WriteMessage(&leader_changed_ack, socketFD) == false)
			{
				//Error in write
				delete match_message;
				delete change;
				return new CallbackChange(CALLBACK_ERROR);
			}
			delete match_message;
			return change;
		}
		case MATCH_START_NOTIFICATION:
		{
			//TODO: Must accept first

			//Get what we need from the message
			MainLobbyCallbackChange *change = new MainLobbyCallbackChange(MATCH_STARTED);

			//***********************************
			// Send Match Started Ack
			//***********************************
			MatchLobbyMessage match_started_ack(MATCH_START_ACK, DIRECTION_TO_CLIENT);
			match_started_ack.m_changeAccepted = true;
			if( Message::WriteMessage(&match_started_ack, socketFD) == false)
			{
				//Error in write
				delete match_message;
				delete change;
				return new CallbackChange(CALLBACK_ERROR);
			}
			delete match_message;
			return change;
		}
		default:
		{
			cerr << "ERROR: Received a bad message on the callback socket\n";
			SendError(socketFD, AUTHENTICATION_ERROR, DIRECTION_TO_CLIENT);
			return new CallbackChange(CALLBACK_ERROR);
		}
	}
	delete match_message;
	return new CallbackChange(CALLBACK_ERROR);
}


//********************************************
//			Send Error Message
//********************************************

//Send a message of type Error to the client
void  RTT::SendError(int socket, enum ErrorType errorType, enum ProtocolDirection direction)
{
	ErrorMessage error_msg(errorType, direction);
	if( Message::WriteMessage(&error_msg, socket) == false)
	{
		cerr << "ERROR: Error message send returned failure.\n";
	}
}

//********************************************
//			Connection Commands
//********************************************

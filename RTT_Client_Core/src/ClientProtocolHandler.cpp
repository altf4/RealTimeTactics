//============================================================================
// Name        : RTT_Client.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Code for handling socket IO on the server side
//============================================================================

#include "ClientProtocolHandler.h"
#include "ClientGameState.h"
#include "messaging/messages/AuthMessage.h"
#include "messaging/messages/MatchLobbyMessage.h"
#include "messaging/messages/GameMessage.h"
#include "messaging/MessageManager.h"
#include "Tile.h"

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <string.h>
#include "event.h"
#include "event2/thread.h"

#define SHA256_DIGEST_LENGTH 32

using namespace std;
using namespace RTT;

int socketFD = -1;
static struct event_base *libeventBase = NULL;
static struct bufferevent *bufferevent = NULL;
pthread_t eventDispatchThread;

string serverIP;
struct PlayerDescription myPlayerDescription;

void *EventDispatcherThread(void *arg)
{
	event_base_dispatch(libeventBase);
	Disconnect();
	return NULL;
}

int RTT::AuthToServer(string IPAddress, uint port,
		string username, unsigned char *hashedPassword, struct PlayerDescription *outDescr)
{
	struct sockaddr_in stSockAddr;
	serverIP = IPAddress;

	Disconnect();

	//Create new base
	if(libeventBase == NULL)
	{
		evthread_use_pthreads();
		libeventBase = event_base_new();
	}

	bufferevent = bufferevent_socket_new(libeventBase, -1, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE);
	if(bufferevent == NULL)
	{
		cerr << "ERROR: Unable to create a socket\n";
		return -1;
	}

	bufferevent_setcb(bufferevent, MessageManager::MessageDispatcher, NULL, MessageManager::ErrorDispatcher, NULL);

	if(bufferevent_enable(bufferevent, EV_READ) == -1)
	{
		cerr << "ERROR: Unable to enable socket events\n";
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

	if(bufferevent_socket_connect(bufferevent, (struct sockaddr *)&stSockAddr, sizeof(stSockAddr)) == -1)
	{
		bufferevent = NULL;
		return -1;
	}

	socketFD = bufferevent_getfd(bufferevent);
	if(socketFD == -1)
	{
		bufferevent_free(bufferevent);
		bufferevent = NULL;
		return -1;
	}

	MessageManager::Instance().DeleteEndpoint(socketFD);
	MessageManager::Instance().StartSocket(socketFD, bufferevent);

	pthread_create(&eventDispatchThread, NULL, EventDispatcherThread, NULL);

	Ticket ticket = MessageManager::Instance().StartConversation(socketFD);

	//***************************
	// Send client Hello
	//***************************
	AuthMessage client_hello(CLIENT_HELLO);
	client_hello.m_softwareVersion.m_major = CLIENT_VERSION_MAJOR;
	client_hello.m_softwareVersion.m_minor = CLIENT_VERSION_MINOR;
	client_hello.m_softwareVersion.m_rev = CLIENT_VERSION_REV;

	if(MessageManager::Instance().WriteMessage(ticket, &client_hello) == false)
	{
		//Error in write
		return -1;
	}

	//***************************
	// Receive Server Hello
	//***************************
	Message *server_hello_init = MessageManager::Instance().ReadMessage(ticket);
	if( server_hello_init->m_messageType != MESSAGE_AUTH)
	{
		SendError(ticket, PROTOCOL_ERROR);
		delete server_hello_init;
		return -1;
	}
	AuthMessage *server_hello = (AuthMessage*)server_hello_init;
	if(server_hello->m_authType != SERVER_HELLO)
	{
		SendError(ticket, PROTOCOL_ERROR);
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

		SendError(ticket, AUTHENTICATION_ERROR);
		delete server_hello_init;
		return -1;
	}
	delete server_hello_init;

	//***************************
	// Send Client Auth
	//***************************
	AuthMessage client_auth(CLIENT_AUTH);
	strncpy( client_auth.m_username, username.data(), USERNAME_MAX_LENGTH);
	memcpy(client_auth.m_hashedPassword, hashedPassword, SHA256_DIGEST_LENGTH);

	if(MessageManager::Instance().WriteMessage(ticket, &client_auth) == false)
	{
		//Error in write
		return -1;
	}

	//***************************
	// Receive Server Auth Reply
	//***************************
	Message *server_auth_reply_init = MessageManager::Instance().ReadMessage(ticket);
	if( server_auth_reply_init->m_messageType != MESSAGE_AUTH)
	{
		delete server_auth_reply_init;
		return -1;
	}
	AuthMessage *server_auth_reply = (AuthMessage*)server_auth_reply_init;

	if((server_auth_reply->m_authType != SERVER_AUTH_REPLY)
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

void RTT::Disconnect()
{
	//Close out any possibly remaining socket artifacts
	if(libeventBase != NULL)
	{
		if(eventDispatchThread != 0)
		{
			if(event_base_loopbreak(libeventBase) == -1)
			{
				cerr << "Unable to exit event loop\n";
			}
			pthread_join(eventDispatchThread, NULL);
			eventDispatchThread = 0;
		}
	}

	if(bufferevent != NULL)
	{
		bufferevent_free(bufferevent);
		bufferevent = NULL;
	}

	MessageManager::Instance().DeleteEndpoint(socketFD);

	socketFD = -1;
}

//Informs the server that we want to exit
//	connectFD: Socket File descriptor of the server
//	Returns true if we get a successful acknowledgment back
bool RTT::ExitServer()
{
	{
		Ticket ticket = MessageManager::Instance().StartConversation(socketFD);

		//********************************
		// Send Exit Server Notification
		//********************************
		LobbyMessage exit_server_notice(MATCH_EXIT_SERVER_NOTIFICATION);
		if(MessageManager::Instance().WriteMessage(ticket, &exit_server_notice) == false)
		{
			//Error in write
			return false;
		}

		//**********************************
		// Receive Exit Server Acknowledge
		//**********************************
		Message *exit_server_ack = MessageManager::Instance().ReadMessage(ticket);
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
	}

	Disconnect();

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

	Ticket ticket = MessageManager::Instance().StartConversation(socketFD);

	//********************************
	// Send Match List Request
	//********************************
	LobbyMessage list_request(MATCH_LIST_REQUEST);
	list_request.m_requestedPage = page;
	if( MessageManager::Instance().WriteMessage(ticket, &list_request) == false)
	{
		//Error in write
		return 0;
	}

	//**********************************
	// Receive Match List Reply
	//**********************************
	Message *list_reply_init = MessageManager::Instance().ReadMessage(ticket);
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
	Ticket ticket = MessageManager::Instance().StartConversation(socketFD);

	//********************************
	// Send Match Create Request
	//********************************
	LobbyMessage create_request(MATCH_CREATE_REQUEST);
	if( MessageManager::Instance().WriteMessage(ticket, &create_request) == false)
	{
		//Error in write
		return false;
	}

	//**********************************
	// Receive Match Options Available
	//**********************************
	Message *ops_available_init = MessageManager::Instance().ReadMessage(ticket);
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
	LobbyMessage ops_chosen(MATCH_CREATE_OPTIONS_CHOSEN);
	ops_chosen.m_options = options;
	if( MessageManager::Instance().WriteMessage(ticket, &ops_chosen) == false)
	{
		//Error in write
		return false;
	}

	//**********************************
	// Receive Match Create Reply
	//**********************************
	Message *create_reply_init = MessageManager::Instance().ReadMessage(ticket);
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
	Ticket ticket = MessageManager::Instance().StartConversation(socketFD);

	vector<PlayerDescription> retPlayers;
	retPlayers.clear();

	//********************************
	// Send Match Join Request
	//********************************
	LobbyMessage join_request(MATCH_JOIN_REQUEST);
	join_request.m_ID = matchID;
	if( MessageManager::Instance().WriteMessage(ticket, &join_request) == false)
	{
		//Error in write
		return retPlayers;
	}

	//**********************************
	// Receive Match Join Reply
	//**********************************
	Message *join_reply_init = MessageManager::Instance().ReadMessage(ticket);
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
	Ticket ticket = MessageManager::Instance().StartConversation(socketFD);

	//********************************
	// Send Match Leave Notification
	//********************************
	MatchLobbyMessage leave_note(MATCH_LEAVE_NOTIFICATION);
	if( MessageManager::Instance().WriteMessage(ticket, &leave_note) == false)
	{
		//Error in write
		return false;
	}

	//**********************************
	// Receive Match Leave Acknowledge
	//**********************************
	Message *leave_ack = MessageManager::Instance().ReadMessage(ticket);
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
	Ticket ticket = MessageManager::Instance().StartConversation(socketFD);

	struct ServerStats stats;
	stats.m_numPlayers = 0;
	stats.m_numMatches = 0;

	//********************************
	// Send Server Stats Request
	//********************************
	LobbyMessage server_stats_req(SERVER_STATS_REQUEST);
	if( MessageManager::Instance().WriteMessage(ticket, &server_stats_req) == false)
	{
		//Error in write
		return stats;
	}

	//**********************************
	// Receive Server Stats Reply
	//**********************************
	Message *msg_init = MessageManager::Instance().ReadMessage(ticket);
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
	Ticket ticket = MessageManager::Instance().StartConversation(socketFD);

	//********************************
	// Send Change Team Request
	//********************************
	MatchLobbyMessage change_team_req(CHANGE_TEAM_REQUEST);
	change_team_req.m_playerID = playerID;
	change_team_req.m_newTeam = team;
	if( MessageManager::Instance().WriteMessage(ticket, &change_team_req) == false)
	{
		//Error in write
		return false;
	}

	//**********************************
	// Receive Change Team Reply
	//**********************************
	Message *message = MessageManager::Instance().ReadMessage(ticket);
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
	Ticket ticket = MessageManager::Instance().StartConversation(socketFD);

	//********************************
	// Send Change Color Request
	//********************************
	MatchLobbyMessage change_color_req(CHANGE_COLOR_REQUEST);
	change_color_req.m_playerID = playerID;
	change_color_req.m_newColor = color;
	if( MessageManager::Instance().WriteMessage(ticket, &change_color_req) == false)
	{
		//Error in write
		return false;
	}

	//**********************************
	// Receive Change Color Reply
	//**********************************
	Message *message = MessageManager::Instance().ReadMessage(ticket);
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
	Ticket ticket = MessageManager::Instance().StartConversation(socketFD);

	//********************************
	// Send Change Map Request
	//********************************
	MatchLobbyMessage change_map_req(CHANGE_MAP_REQUEST);
	change_map_req.m_mapDescription = map;
	if( MessageManager::Instance().WriteMessage(ticket, &change_map_req) == false)
	{
		//Error in write
		return false;
	}

	//**********************************
	// Receive Change Map Reply
	//**********************************
	Message *message = MessageManager::Instance().ReadMessage(ticket);
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
	Ticket ticket = MessageManager::Instance().StartConversation(socketFD);

	//********************************
	// Send Change Speed Request
	//********************************
	MatchLobbyMessage change_speed_req(CHANGE_GAME_SPEED_REQUEST);
	change_speed_req.m_newSpeed = speed;
	if( MessageManager::Instance().WriteMessage(ticket, &change_speed_req) == false)
	{
		//Error in write
		return false;
	}

	//**********************************
	// Receive Change Speed Reply
	//**********************************
	Message *message = MessageManager::Instance().ReadMessage(ticket);
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
	Ticket ticket = MessageManager::Instance().StartConversation(socketFD);

	//********************************
	// Send Change Victory Request
	//********************************
	MatchLobbyMessage change_victory_req(CHANGE_VICTORY_COND_REQUEST);
	change_victory_req.m_newVictCond = victory;
	if( MessageManager::Instance().WriteMessage(ticket, &change_victory_req) == false)
	{
		//Error in write
		return false;
	}

	//**********************************
	// Receive Change Victory Reply
	//**********************************
	Message *message = MessageManager::Instance().ReadMessage(ticket);
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
	Ticket ticket = MessageManager::Instance().StartConversation(socketFD);

	//********************************
	// Send Change Leader Request
	//********************************
	MatchLobbyMessage change_leader_req(CHANGE_LEADER_REQUEST);
	change_leader_req.m_playerID = newLeaderID;
	if( MessageManager::Instance().WriteMessage(ticket, &change_leader_req) == false)
	{
		//Error in write
		return false;
	}

	//**********************************
	// Receive Change Leader Reply
	//**********************************
	Message *message = MessageManager::Instance().ReadMessage(ticket);
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
	Ticket ticket = MessageManager::Instance().StartConversation(socketFD);

	//********************************
	// Send Kick Player Request
	//********************************
	MatchLobbyMessage kick_player_req(KICK_PLAYER_REQUEST);
	kick_player_req.m_playerID = PlayerID;
	if( MessageManager::Instance().WriteMessage(ticket, &kick_player_req) == false)
	{
		//Error in write
		return false;
	}

	//**********************************
	// Receive Kick Player Reply
	//**********************************
	Message *message = MessageManager::Instance().ReadMessage(ticket);
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
	Ticket ticket = MessageManager::Instance().StartConversation(socketFD);

	//********************************
	// Send Start Match Request
	//********************************
	MatchLobbyMessage start_match_req(START_MATCH_REQUEST);
	if( MessageManager::Instance().WriteMessage(ticket, &start_match_req) == false)
	{
		//Error in write
		return false;
	}

	//**********************************
	// Receive Start Match Reply
	//**********************************
	Message *message = MessageManager::Instance().ReadMessage(ticket);
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
//			Send Error Message
//********************************************

//Send a message of type Error to the client
void  RTT::SendError(Ticket &ticket, enum ErrorType errorType)
{
	ErrorMessage error_msg(errorType);
	if( MessageManager::Instance().WriteMessage(ticket, &error_msg) == false)
	{
		cerr << "ERROR: Error message send returned failure.\n";
	}
}

//********************************************
//			Connection Commands
//********************************************



//********************************************
//			Callback Processing
//********************************************

enum LobbyReturn RTT::ProcessMainLobbyEvent(Ticket &ticket, MainLobbyEvents *lobbyContext)
{
	return IN_MAIN_LOBBY;
}

//Processes and executes a single Match Lobby Event from the server
//	returns - The new state of the client as it leaves the function
enum LobbyReturn RTT::ProcessMatchLobbyEvent(Ticket &ticket, MatchLobbyEvents *gameContext)
{
	Message *event_message = MessageManager::Instance().ReadMessage(ticket);
	if(event_message->m_messageType != MESSAGE_MATCH_LOBBY)
	{
		cerr << "ERROR: Message read from server failed. Did it die?\n";
		delete event_message;
		return IN_MATCH_LOBBY;
	}

	enum LobbyReturn ret = IN_MATCH_LOBBY;

	MatchLobbyMessage *match_message = (MatchLobbyMessage*)event_message;
	switch(match_message->m_matchLobbyType)
	{
		case TEAM_CHANGED_NOTIFICATION:
		{
			gameContext->UI_TeamChangedSignal(match_message->m_playerID, match_message->m_newTeam);

			//***********************************
			// Send Team Changed Ack
			//***********************************
			MatchLobbyMessage team_change_ack(TEAM_CHANGED_ACK);
			if( MessageManager::Instance().WriteMessage(ticket, &team_change_ack) == false)
			{
				//Error in write
				ret = IN_MATCH_LOBBY;
			}
			ret = IN_MATCH_LOBBY;
			break;
		}
		case KICKED_FROM_MATCH_NOTIFICATION:
		{

			gameContext->UI_KickFromMatchSignal();

			//***********************************
			// Send Kicked From Match Ack
			//***********************************
			MatchLobbyMessage kicked_ack(KICKED_FROM_MATCH_ACK);
			if( MessageManager::Instance().WriteMessage(ticket, &kicked_ack) == false)
			{
				//Error in write
			}

			ret = IN_MAIN_LOBBY;
			break;
		}
		case PLAYER_LEFT_MATCH_NOTIFICATION:
		{
			gameContext->UI_PlayerLeftSignal(match_message->m_playerID, match_message->m_newLeaderID);

			//***********************************
			// Send Player Left Ack
			//***********************************
			MatchLobbyMessage player_left_ack(PLAYER_LEFT_MATCH_ACK);
			if( MessageManager::Instance().WriteMessage(ticket, &player_left_ack) == false)
			{
				//Error in write
			}
			ret = IN_MATCH_LOBBY;
			break;
		}
		case PLAYER_JOINED_MATCH_NOTIFICATION:
		{
			gameContext->UI_PlayerJoinedSignal(match_message->m_playerDescription);

			//***********************************
			// Send Player Joined Ack
			//***********************************
			MatchLobbyMessage player_joined_ack(PLAYER_JOINED_MATCH_ACK);
			if( MessageManager::Instance().WriteMessage(ticket, &player_joined_ack) == false)
			{
				//Error in write
			}
			ret = IN_MATCH_LOBBY;
			break;
		}
		case COLOR_CHANGED_NOTIFICATION:
		{
			gameContext->UI_ColorChangedSignal( match_message->m_playerID, match_message->m_newColor);

			//***********************************
			// Send Color Changed Ack
			//***********************************
			MatchLobbyMessage color_change_ack(COLOR_CHANGED_ACK);
			if( MessageManager::Instance().WriteMessage(ticket, &color_change_ack) == false)
			{
				//Error in write
			}
			ret = IN_MATCH_LOBBY;
			break;
		}
		case MAP_CHANGED_NOTIFICATION:
		{
			gameContext->UI_MapChangedSignal(match_message->m_mapDescription);

			//***********************************
			// Send Map Changed Ack
			//***********************************
			MatchLobbyMessage map_changed_ack(MAP_CHANGED_ACK);
			if( MessageManager::Instance().WriteMessage(ticket, &map_changed_ack) == false)
			{
				//Error in write
			}
			ret = IN_MATCH_LOBBY;
			break;
		}
		case GAME_SPEED_CHANGED_NOTIFICATION:
		{
			gameContext->UI_GamespeedChangedSignal(match_message->m_newSpeed);

			//***********************************
			// Send Game Speed Changed Ack
			//***********************************
			MatchLobbyMessage speed_changed_ack(GAME_SPEED_CHANGED_ACK);
			if( MessageManager::Instance().WriteMessage(ticket, &speed_changed_ack) == false)
			{
				//Error in write
			}
			ret = IN_MATCH_LOBBY;
			break;
		}
		case VICTORY_COND_CHANGED_NOTIFICATION:
		{
			gameContext->UI_VictoryCondChangedSignal(match_message->m_newVictCond);

			//***********************************
			// Send Victory Condition Changed Ack
			//***********************************
			MatchLobbyMessage victory_changed_ack(VICTORY_COND_CHANGED_ACK);
			if( MessageManager::Instance().WriteMessage(ticket, &victory_changed_ack) == false)
			{
				//Error in write
			}
			ret = IN_MATCH_LOBBY;
			break;
		}
		case CHANGE_LEADER_NOTIFICATION:
		{
			gameContext->UI_ChangeLeaderSignal(match_message->m_playerID);

			//***********************************
			// Send Change Leader Ack
			//***********************************
			MatchLobbyMessage leader_changed_ack(CHANGE_LEADER_ACK);
			if( MessageManager::Instance().WriteMessage(ticket, &leader_changed_ack) == false)
			{
				//Error in write
			}
			ret = IN_MATCH_LOBBY;
			break;
		}
		case MATCH_START_NOTIFICATION:
		{
			//TODO: Must accept first
			gameContext->UI_MatchStartedSignal();

			//***********************************
			// Send Match Started Ack
			//***********************************
			MatchLobbyMessage match_started_ack(MATCH_START_ACK);
			match_started_ack.m_changeAccepted = true;
			if(MessageManager::Instance().WriteMessage(ticket, &match_started_ack) == false)
			{
				//Error in write
			}
			ret = IN_GAME;
			break;
		}
		default:
		{
			cerr << "ERROR: Received a bad message on the callback socket\n";
			SendError(ticket, AUTHENTICATION_ERROR);
			ret = IN_MATCH_LOBBY;
			break;
		}
	}
	delete match_message;
	return ret;
}

//Processes and executes a single Game Event from the server
//	returns - The new state of the client as it leaves the function
enum LobbyReturn RTT::ProcessGameEvent(Ticket &ticket, GameEvents *gameContext)
{
	Message *event_message = MessageManager::Instance().ReadMessage(ticket);
	if(event_message->m_messageType != MESSAGE_GAME)
	{
		cerr << "ERROR: Message read from server failed. Did it die?\n";
		cout << "xxxDEBUGxxx Message Type: " << event_message->m_messageType << " Subtype: "<< ((ErrorMessage*)event_message)->m_errorType << endl;
		delete event_message;
		return IN_GAME;
	}

	enum LobbyReturn ret = IN_GAME;

	GameMessage *game_message = (GameMessage*)event_message;
	switch(game_message->m_gameMessageType)
	{
		case UNIT_MOVED_DIRECTION_NOTICE:
		{
			//TODO: Specify a separate direction to face
			struct Coordinate source = {game_message->m_xOld, game_message->m_yOld};

			ClientGameState::Instance().MoveUnitDirection(game_message->m_unitID,
					source,	game_message->m_unitDirection, game_message->m_unitDirection);

			//Call the UI's movement code (IE: Move the unit on the screen)
			gameContext->UI_UnitMovedDirectionSignal(game_message->m_unitID,
					source,	game_message->m_unitDirection, game_message->m_unitDirection);

			//Send back an acknowledgment of the move
			GameMessage unit_moved_reply(UNIT_MOVED_DIRECTION_ACK);
			MessageManager::Instance().WriteMessage(ticket, &unit_moved_reply);
			ret = IN_GAME;
			break;
		}
		case UNIT_MOVED_DISTANT_NOTICE:
		{
			struct Coordinate source = {game_message->m_xOld, game_message->m_yOld};
			struct Coordinate dest = {game_message->m_xNew, game_message->m_yNew};

			ClientGameState::Instance().MoveUnitDistant(game_message->m_unitID, source,	dest);

			//Call the UI's movement code (IE: Move the unit on the screen)
			gameContext->UI_UnitMovedDistantSignal(game_message->m_unitID, source, dest);

			GameMessage unit_moved_reply(UNIT_MOVED_DISTANT_ACK);
			MessageManager::Instance().WriteMessage(ticket, &unit_moved_reply);
			ret = IN_GAME;
			break;
		}
		default:
		{
			break;
		}
	}

	delete game_message;
	return ret;
}

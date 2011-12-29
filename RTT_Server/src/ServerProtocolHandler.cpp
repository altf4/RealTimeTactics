//============================================================================
// Name        : ServerProtocolHandler.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Code for handling socket IO on the server side
//============================================================================

#include "RTT_Server.h"
#include "ServerProtocolHandler.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>

using namespace std;
using namespace RTT;

extern PlayerList playerList;
extern MatchList matchList;
extern ConnectBackWaitPool connectBackWaitPool;

extern pthread_rwlock_t matchListLock;
extern pthread_rwlock_t playerListLock;

extern pthread_rwlock_t playerIDLock;
extern pthread_rwlock_t waitPoolLock;
extern uint lastPlayerID;

//Negotiates the hello messages and authentication to a new client
//	Returns a new Player object, NULL on error
Player *RTT::GetNewClient(int ConnectFD)
{
	//***************************
	// Read client Hello
	//***************************
	Message *client_hello_init = Message::ReadMessage(ConnectFD);
	if( client_hello_init == NULL )
	{
		SendError(ConnectFD, PROTOCOL_ERROR);
		return NULL;
	}
	if( client_hello_init->type != CLIENT_HELLO )
	{
		cerr << "ERROR: Expected CLIENT_HELLO message, received: "
				<< client_hello_init->type << "\n";
		SendError(ConnectFD, PROTOCOL_ERROR);
		delete client_hello_init;
		return NULL;
	}

	AuthMessage *client_hello = (AuthMessage*)client_hello_init;
	//Check version compatibility
	if ((client_hello->softwareVersion.major != SERVER_VERSION_MAJOR) ||
		(client_hello->softwareVersion.minor != SERVER_VERSION_MINOR) ||
		(client_hello->softwareVersion.rev != SERVER_VERSION_REV) )
	{
		//If versions are not the same, send an error message to the client
		delete client_hello;
		cout << "Client Connected with bad software version.\n";
		SendError(ConnectFD, INCOMPATIBLE_SOFTWARE_VERSION);
		return NULL;
	}
	delete client_hello;


	//***************************
	// Send Server Hello
	//***************************
	AuthMessage *server_hello = new AuthMessage();
	server_hello->type = SERVER_HELLO;
	server_hello->softwareVersion.major = SERVER_VERSION_MAJOR;
	server_hello->softwareVersion.minor = SERVER_VERSION_MINOR;
	server_hello->softwareVersion.rev = SERVER_VERSION_REV;

	if(  Message::WriteMessage(server_hello, ConnectFD) == false)
	{
		//Error in write
		delete server_hello;
		return NULL;
	}
	delete server_hello;

	//***************************
	// Receive Client Auth
	//***************************
	Message *client_auth_init = Message::ReadMessage(ConnectFD);
	if( client_auth_init == NULL )
	{
		//ERROR
		SendError(ConnectFD, PROTOCOL_ERROR);
		return NULL;
	}
	if( client_auth_init->type != CLIENT_AUTH)
	{
		//Error
		SendError(ConnectFD, PROTOCOL_ERROR);
		delete client_auth_init;
		return NULL;
	}

	AuthMessage *client_auth = (AuthMessage*)client_auth_init;
	enum AuthResult authresult =
			AuthenticateClient(client_auth->username, client_auth->hashedPassword);

	Player *player = NULL;
	if( authresult == AUTH_SUCCESS)
	{
		pthread_rwlock_wrlock(&playerIDLock);
		uint ID = lastPlayerID++;
		pthread_rwlock_unlock(&playerIDLock);

		player = new Player(client_auth->username, ID);
		pthread_rwlock_wrlock(&playerListLock);
		playerList[ID] = player;
		pthread_rwlock_unlock(&playerListLock);
	}

	delete client_auth;


	//***************************
	// Send Server Auth Reply
	//***************************
	AuthMessage *server_auth_reply = new AuthMessage();
	server_auth_reply->type = SERVER_AUTH_REPLY;
	server_auth_reply->authSuccess = authresult;

	if(  Message::WriteMessage(server_auth_reply, ConnectFD) == false)
	{
		//Error in write
		delete server_auth_reply;
		return NULL;
	}

	delete server_auth_reply;
	return player;
}

//Processes one Lobby command
//	Starts out by listening on the given socket for a LobbyMessage
//	Executes the Lobby protocol
//	Returns a enum LobbyReturn to describe the end state
enum LobbyReturn RTT::ProcessLobbyCommand(int ConnectFD, Player *player)
{
	//********************************
	// Receive Initial Lobby Message
	//********************************
	Message *lobby_message_init = Message::ReadMessage(ConnectFD);
	if( lobby_message_init == NULL )
	{
		//ERROR
		cerr << "ERROR: Lobby message came back NULL\n";
		SendError(ConnectFD, PROTOCOL_ERROR);
		return EXITING_SERVER;
	}

	LobbyMessage *lobby_message = (LobbyMessage*)lobby_message_init;
	switch (lobby_message->type)
	{
		case MATCH_LIST_REQUEST:
		{
			struct MatchDescription matches[MATCHES_PER_PAGE];
			uint matchCount = GetMatchDescriptions(lobby_message->requestedPage, matches);

			//***************************
			// Send Query Reply
			//***************************
			LobbyMessage *query_reply = new LobbyMessage();
			query_reply->type = MATCH_LIST_REPLY;
			query_reply->returnedMatchesCount = matchCount;
			query_reply->matchDescriptions = (MatchDescription*)
					malloc(sizeof(struct MatchDescription) * matchCount);
			for(uint i = 0; i < matchCount; i++ )
			{
				query_reply->matchDescriptions[i] = matches[i];
			}
			if(  Message::WriteMessage(query_reply, ConnectFD) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
			}
			delete query_reply;

			return IN_MAIN_LOBBY;
		}
		case MATCH_CREATE_REQUEST:
		{
			//***************************
			// Send Options Available
			//***************************
			LobbyMessage *options_available = new LobbyMessage();
			options_available->type = MATCH_CREATE_OPTIONS_AVAILABLE;
			options_available->options.maxPlayers = MAX_PLAYERS_IN_MATCH;
			if(  Message::WriteMessage(options_available, ConnectFD) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
				delete options_available;
				return IN_MAIN_LOBBY;
			}
			delete options_available;

			//********************************
			// Receive Options Chosen
			//********************************
			Message *options_chosen_init = Message::ReadMessage(ConnectFD);
			if( options_chosen_init == NULL )
			{
				//Error
				cerr << "ERROR: Reading from client failed.\n";
				SendError(ConnectFD, PROTOCOL_ERROR);
				return IN_MAIN_LOBBY;
			}
			if( options_chosen_init->type !=  MATCH_CREATE_OPTIONS_CHOSEN)
			{
				//Error
				cerr << "ERROR: Client gave us the wrong message type.\n";
				SendError(ConnectFD, PROTOCOL_ERROR);
				delete options_chosen_init;
				return IN_MAIN_LOBBY;
			}

			LobbyMessage *options_chosen = (LobbyMessage*)options_chosen_init;
			if((options_chosen->options.maxPlayers > MAX_PLAYERS_IN_MATCH) ||
					(options_chosen->options.maxPlayers < 2))
			{
				cerr << "ERROR: Client asked an invalid number of max playersb.\n";
				SendError(ConnectFD, INVALID_MAX_PLAYERS);
				return IN_MAIN_LOBBY;
			}

			uint matchID = RegisterNewMatch(player, options_chosen->options);

			if( matchID == 0 )
			{
				//TODO: Find a better error message hereb
				SendError(ConnectFD, TOO_BUSY);
				return IN_MAIN_LOBBY;
			}
			delete options_chosen;

			//***************************
			// Send Match Create Reply
			//***************************
			LobbyMessage *create_reply = new LobbyMessage();
			create_reply->type = MATCH_CREATE_REPLY;
			create_reply->ID = matchID;
			if(  Message::WriteMessage(create_reply, ConnectFD) == false)
			{
				//Error in write, do something?
				//TODO: This case is awkward. Should probably
				//	remove player from the match?
				cerr << "ERROR: Message send returned failure.\n";
				return IN_MAIN_LOBBY;
			}
			delete create_reply;

			return IN_MATCH_LOBBY;
		}
		case MATCH_JOIN_REQUEST:
		{
			//Try to join the match, process result
			enum LobbyResult joinResult = JoinMatch(player, lobby_message->ID);
			switch(joinResult)
			{
				enum ErrorType errorType;
				case LOBBY_SUCCESS:
				{
					//***************************
					// Send Match Join Reply
					//***************************
					LobbyMessage *match_join = new LobbyMessage();
					match_join->type = MATCH_JOIN_REPLY;
					pthread_rwlock_rdlock(&matchListLock);
					match_join->matchDescription = matchList[lobby_message->ID]->description;
					pthread_rwlock_unlock(&matchListLock);
					if(  Message::WriteMessage(match_join, ConnectFD) == false)
					{
						//Error in write, do something?
						cerr << "ERROR: Message send returned failure.\n";
					}
					delete match_join;
					return IN_MATCH_LOBBY;
				}
				case LOBBY_MATCH_IS_FULL:
				{
					errorType = MATCH_IS_FULL;
					break;
				}
				case LOBBY_MATCH_DOESNT_EXIST:
				{
					errorType = MATCH_DOESNT_EXIST;
					break;
				}
				case LOBBY_NOT_ALLOWED_IN:
				{
					errorType = NOT_ALLOWED_IN;
					break;
				}
				case LOBBY_ALREADY_IN_MATCH:
				{
					errorType = ALREADY_IN_MATCH;
					break;
				}
				default:
				{
					errorType = PROTOCOL_ERROR;
					break;
				}
				SendError(ConnectFD, errorType);
				return IN_MAIN_LOBBY;
			}
		}
		case MATCH_LEAVE_NOTIFICATION:
		{
			if( player->currentMatch == NULL )
			{
				SendError(ConnectFD, NOT_IN_THAT_MATCH);
				return IN_MAIN_LOBBY;
			}
			if( LeaveMatch(player) )
			{
				//*******************************
				// Send Match Leave Acknowledge
				//*******************************
				LobbyMessage *leave_ack = new LobbyMessage();
				leave_ack->type = MATCH_LEAVE_ACKNOWLEDGE;
				if(  Message::WriteMessage(leave_ack, ConnectFD) == false)
				{
					//Error in write, do something?
					cerr << "ERROR: Message send returned failure.\n";
				}
				delete leave_ack;
				return IN_MAIN_LOBBY;
			}
			else
			{
				SendError(ConnectFD, NOT_IN_THAT_MATCH);
				return IN_MAIN_LOBBY;
			}
		}
		case SERVER_STATS_REQUEST:
		{
			//*******************************
			// Send Server Stats Reply
			//*******************************
			LobbyMessage *stats_reply = new LobbyMessage();
			stats_reply->type = SERVER_STATS_REPLY;

			pthread_rwlock_rdlock(&matchListLock);
			stats_reply->serverStats.numMatches = matchList.size();
			pthread_rwlock_unlock(&matchListLock);

			pthread_rwlock_rdlock(&playerListLock);
			stats_reply->serverStats.numPlayers = playerList.size();
			pthread_rwlock_unlock(&playerListLock);

			if( Message::WriteMessage(stats_reply, ConnectFD) == false )
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
			}
			delete stats_reply;
			return IN_MAIN_LOBBY;
		}
		case MATCH_EXIT_SERVER_NOTIFICATION:
		{
			//*******************************
			// Send Exit Server Acknowledge
			//*******************************
			LobbyMessage *exit_server = new LobbyMessage();
			exit_server->type = MATCH_EXIT_SERVER_ACKNOWLEDGE;
			if(  Message::WriteMessage(exit_server, ConnectFD) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
			}
			delete exit_server;
			return EXITING_SERVER;
		}
		default:
		{
			SendError(ConnectFD, PROTOCOL_ERROR);
			return IN_MAIN_LOBBY;
		}
	}
	delete lobby_message;
}

//Authenticates the given username/password with the server
//Checks that:
//	a) The username exists in the system
//	b) The given password hash is correct for the specified username
//	c) The username is unique on the server
enum AuthResult RTT::AuthenticateClient(char *username, unsigned char *hashedPassword)
{
	//TODO: Authenticate!

	//Check if the username exists in the active list
	pthread_rwlock_rdlock(&playerListLock);
	PlayerList::iterator it = playerList.begin();
	for(; it != playerList.end(); ++it)
	{
		if( it->second->GetName().compare(username) == 0)
		{
			pthread_rwlock_unlock(&playerListLock);
			return USERNAME_ALREADY_EXISTS;
		}
	}

	//TODO: Check if the username exists in the non-active list (from file on disk)
	pthread_rwlock_unlock(&playerListLock);
	return AUTH_SUCCESS;
}

//Processes one MatchLobby command
//	Starts out by listening on the given socket for a MatchLobbyMessage
//	Executes the MatchLobby protocol
//	Returns a enum LobbyReturn to describe the end state
enum LobbyReturn RTT::ProcessMatchLobbyCommand(int connectFD, Player *player)
{
	//********************************
	// Receive Initial Lobby Message
	//********************************
	Message *match_lobby_message_init = Message::ReadMessage(connectFD);
	if( match_lobby_message_init == NULL )
	{
		//ERROR
		cerr << "ERROR: Lobby message came back NULL\n";
		SendError(connectFD, PROTOCOL_ERROR);
		return EXITING_SERVER;
	}

	MatchLobbyMessage *match_lobby_message = (MatchLobbyMessage*)match_lobby_message_init;
	switch (match_lobby_message->type)
	{
		case CHANGE_TEAM_REQUEST:
		{
			if( player->currentMatch == NULL )
			{
				SendError(connectFD, PROTOCOL_ERROR);
				return IN_MAIN_LOBBY;
			}

			pthread_rwlock_wrlock(&matchListLock);
			if( matchList.count(player->currentMatch->GetID()) == 0)
			{
				//Error, there is no such Match
				SendError(connectFD, MATCH_DOESNT_EXIST);
				return IN_MAIN_LOBBY;
			}
			//Is this not a self change?
			if( player->GetID() != match_lobby_message->playerID )
			{
				//Is this this not the leader?
				if( matchList[player->currentMatch->GetID()]->leader != player)
				{
					//Error, there is no such Match
					SendError(connectFD, NOT_ALLOWED_TO_CHANGE_THAT);
					return IN_MAIN_LOBBY;
				}
			}

			bool changed = matchList[player->currentMatch->GetID()]->ChangeTeam(
					match_lobby_message->playerID, match_lobby_message->newTeam);
			pthread_rwlock_unlock(&matchListLock);

			//*******************************
			// Send CHANGE TEAM REPLY
			//*******************************
			MatchLobbyMessage *change_team_reply = new MatchLobbyMessage();
			change_team_reply->type = CHANGE_TEAM_REPLY;
			change_team_reply->changeAccepted = changed;
			if(  Message::WriteMessage(change_team_reply, connectFD) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
			}
			delete change_team_reply;

			if( changed )
			{
				//*******************************
				// Send Client Notifications
				//*******************************
				MatchLobbyMessage *notification = new MatchLobbyMessage();
				notification->type = TEAM_CHANGED_NOTIFICATION;
				notification->newTeam = match_lobby_message->newTeam;
				notification->playerID = match_lobby_message->playerID;
				pthread_rwlock_rdlock(&matchListLock);
				NotifyClients(matchList[player->currentMatch->GetID()],
						notification);
				pthread_rwlock_unlock(&matchListLock);
				delete notification;
			}

			break;
		}
		case START_MATCH_REQUEST:
		{
			if( player->currentMatch == NULL )
			{
				SendError(connectFD, PROTOCOL_ERROR);
				return IN_MAIN_LOBBY;
			}

			pthread_rwlock_wrlock(&matchListLock);
			if( matchList.count(player->currentMatch->GetID()) == 0)
			{
				//Error, there is no such Match
				SendError(connectFD, MATCH_DOESNT_EXIST);
				return IN_MAIN_LOBBY;
			}
			//Is this not a self change?
			if( player->GetID() != match_lobby_message->playerID )
			{
				//Is this this not the leader?
				if( matchList[player->currentMatch->GetID()]->leader != player)
				{
					//Error, there is no such Match
					SendError(connectFD, NOT_ALLOWED_TO_CHANGE_THAT);
					return IN_MAIN_LOBBY;
				}
			}

			bool started = matchList[player->currentMatch->GetID()]->StartMatch();
			pthread_rwlock_unlock(&matchListLock);

			//*******************************
			// Send START MATCH REPLY
			//*******************************
			MatchLobbyMessage *start_match_reply = new MatchLobbyMessage();
			start_match_reply->type = START_MATCH_REPLY;
			start_match_reply->changeAccepted = started;
			if(  Message::WriteMessage(start_match_reply, connectFD) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
			}
			delete start_match_reply;

			if( started )
			{
				//*******************************
				// Send Client Notifications
				//*******************************
				//TODO: Make sure each client is ready. IE: Listed for replies
				MatchLobbyMessage *notification = new MatchLobbyMessage();
				notification->type = MATCH_START_NOTIFICATION;
				pthread_rwlock_rdlock(&matchListLock);
				NotifyClients(matchList[player->currentMatch->GetID()],
						notification);
				pthread_rwlock_unlock(&matchListLock);
				delete notification;
			}

			break;
		}
		case CHANGE_COLOR_REQUEST:
		{
			if( player->currentMatch == NULL )
			{
				SendError(connectFD, PROTOCOL_ERROR);
				return IN_MAIN_LOBBY;
			}

			pthread_rwlock_wrlock(&matchListLock);
			if( matchList.count(player->currentMatch->GetID()) == 0)
			{
				//Error, there is no such Match
				SendError(connectFD, MATCH_DOESNT_EXIST);
				return IN_MAIN_LOBBY;
			}
			//Is this not a self change?
			if( player->GetID() != match_lobby_message->playerID )
			{
				//Is this this not the leader?
				if( matchList[player->currentMatch->GetID()]->leader != player)
				{
					//Error, there is no such Match
					SendError(connectFD, NOT_ALLOWED_TO_CHANGE_THAT);
					return IN_MAIN_LOBBY;
				}
			}
			pthread_rwlock_unlock(&matchListLock);

			pthread_rwlock_wrlock(&playerListLock);
			if( playerList.count(match_lobby_message->playerID) != 0 )
			{
				playerList[match_lobby_message->playerID]->SetColor(
						match_lobby_message->newColor);
			}
			else
			{
				//Error, there is no such player
				SendError(connectFD, NO_SUCH_PLAYER);
				return IN_MAIN_LOBBY;
			}
			pthread_rwlock_unlock(&playerListLock);

			//*******************************
			// Send CHANGE COLOR REPLY
			//*******************************
			MatchLobbyMessage *change_color_reply = new MatchLobbyMessage();
			change_color_reply->type = CHANGE_COLOR_REPLY;
			change_color_reply->changeAccepted = true;
			if(  Message::WriteMessage(change_color_reply, connectFD) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
			}
			delete change_color_reply;

			//*******************************
			// Send Client Notifications
			//*******************************
			MatchLobbyMessage *notification = new MatchLobbyMessage();
			notification->type = COLOR_CHANGED_NOTIFICATION;
			notification->newColor = match_lobby_message->newColor;
			pthread_rwlock_rdlock(&matchListLock);
			NotifyClients(matchList[player->currentMatch->GetID()],
					notification);
			pthread_rwlock_unlock(&matchListLock);
			delete notification;

			break;
		}
		case CHANGE_MAP_REQUEST:
		{
			if( player->currentMatch == NULL )
			{
				SendError(connectFD, PROTOCOL_ERROR);
				return IN_MAIN_LOBBY;
			}

			pthread_rwlock_wrlock(&matchListLock);
			if( matchList.count(player->currentMatch->GetID()) == 0)
			{
				//Error, there is no such Match
				SendError(connectFD, MATCH_DOESNT_EXIST);
				return IN_MAIN_LOBBY;
			}
			//Is this this not the leader?
			if( matchList[player->currentMatch->GetID()]->leader != player)
			{
				//Error, there is no such Match
				SendError(connectFD, NOT_ALLOWED_TO_CHANGE_THAT);
				return IN_MAIN_LOBBY;
			}

			matchList[player->currentMatch->GetID()]->map =
					match_lobby_message->mapDescription;

			pthread_rwlock_unlock(&matchListLock);

			//*******************************
			// Send CHANGE MAP REPLY
			//*******************************
			MatchLobbyMessage *change_map_reply = new MatchLobbyMessage();
			change_map_reply->type = MAP_CHANGED_NOTIFICATION;
			change_map_reply->changeAccepted = true;
			if(  Message::WriteMessage(change_map_reply, connectFD) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
			}
			delete change_map_reply;

			//*******************************
			// Send Client Notifications
			//*******************************
			MatchLobbyMessage *notification = new MatchLobbyMessage();
			notification->type = MAP_CHANGED_NOTIFICATION;
			notification->mapDescription = match_lobby_message->mapDescription;
			pthread_rwlock_rdlock(&matchListLock);
			NotifyClients(matchList[player->currentMatch->GetID()],
					notification);
			pthread_rwlock_unlock(&matchListLock);
			delete notification;

			break;
		}
		case CHANGE_VICTORY_COND_REQUEST:
		{
			if( player->currentMatch == NULL )
			{
				SendError(connectFD, PROTOCOL_ERROR);
				return IN_MAIN_LOBBY;
			}

			pthread_rwlock_wrlock(&matchListLock);
			if( matchList.count(player->currentMatch->GetID()) == 0)
			{
				//Error, there is no such Match
				SendError(connectFD, MATCH_DOESNT_EXIST);
				return IN_MAIN_LOBBY;
			}
			//Is this this not the leader?
			if( matchList[player->currentMatch->GetID()]->leader != player)
			{
				//Error, there is no such Match
				SendError(connectFD, NOT_ALLOWED_TO_CHANGE_THAT);
				return IN_MAIN_LOBBY;
			}

			matchList[player->currentMatch->GetID()]->victoryCondition =
					match_lobby_message->newVictCond;

			pthread_rwlock_unlock(&matchListLock);

			//*************************************
			// Send Change Victory Condition Reply
			//**************************************
			MatchLobbyMessage *change_victory_reply = new MatchLobbyMessage();
			change_victory_reply->type = CHANGE_VICTORY_COND_REPLY;
			change_victory_reply->changeAccepted = true;
			if(  Message::WriteMessage(change_victory_reply, connectFD) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
			}
			delete change_victory_reply;

			//*******************************
			// Send Client Notifications
			//*******************************
			MatchLobbyMessage *notification = new MatchLobbyMessage();
			notification->type = VICTORY_COND_CHANGED_NOTIFICATION;
			notification->newVictCond = match_lobby_message->newVictCond;
			pthread_rwlock_rdlock(&matchListLock);
			NotifyClients(matchList[player->currentMatch->GetID()],
					notification);
			pthread_rwlock_unlock(&matchListLock);
			delete notification;

			break;
		}
		case CHANGE_GAME_SPEED_REQUEST:
		{
			if( player->currentMatch == NULL )
			{
				SendError(connectFD, PROTOCOL_ERROR);
				return IN_MAIN_LOBBY;
			}

			pthread_rwlock_wrlock(&matchListLock);
			if( matchList.count(player->currentMatch->GetID()) == 0)
			{
				//Error, there is no such Match
				SendError(connectFD, MATCH_DOESNT_EXIST);
				return IN_MAIN_LOBBY;
			}
			//Is this this not the leader?
			if( matchList[player->currentMatch->GetID()]->leader != player)
			{
				//Error, there is no such Match
				SendError(connectFD, NOT_ALLOWED_TO_CHANGE_THAT);
				return IN_MAIN_LOBBY;
			}

			matchList[player->currentMatch->GetID()]->gameSpeed =
					match_lobby_message->newSpeed;

			pthread_rwlock_unlock(&matchListLock);

			//*******************************
			// Send Change Game Speed Reply
			//*******************************
			MatchLobbyMessage *change_speed_reply = new MatchLobbyMessage();
			change_speed_reply->type = CHANGE_GAME_SPEED_REPLY;
			change_speed_reply->changeAccepted = true;
			if(  Message::WriteMessage(change_speed_reply, connectFD) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
			}
			delete change_speed_reply;

			//*******************************
			// Send Client Notifications
			//*******************************
			MatchLobbyMessage *notification = new MatchLobbyMessage();
			notification->type = GAME_SPEED_CHANGED_NOTIFICATION;
			notification->newSpeed = match_lobby_message->newSpeed;
			pthread_rwlock_rdlock(&matchListLock);
			NotifyClients(matchList[player->currentMatch->GetID()],
					notification);
			pthread_rwlock_unlock(&matchListLock);
			delete notification;

			break;
		}
		case KICK_PLAYER_REQUEST:
		{
			if( player->currentMatch == NULL )
			{
				SendError(connectFD, PROTOCOL_ERROR);
				return IN_MAIN_LOBBY;
			}

			pthread_rwlock_wrlock(&matchListLock);
			if( matchList.count(player->currentMatch->GetID()) == 0)
			{
				//Error, there is no such Match
				SendError(connectFD, MATCH_DOESNT_EXIST);
				return IN_MAIN_LOBBY;
			}
			//Is this this not the leader?
			if( matchList[player->currentMatch->GetID()]->leader != player)
			{
				//Error, there is no such Match
				SendError(connectFD, NOT_ALLOWED_TO_CHANGE_THAT);
				return IN_MAIN_LOBBY;
			}

			bool removed = matchList[player->currentMatch->GetID()]->RemovePlayer(
					match_lobby_message->playerID);

			pthread_rwlock_unlock(&matchListLock);

			//*******************************
			// Send Kick Player Reply
			//*******************************
			MatchLobbyMessage *change_speed_reply = new MatchLobbyMessage();
			change_speed_reply->type = KICK_PLAYER_REPLY;
			change_speed_reply->changeAccepted = removed;
			if(  Message::WriteMessage(change_speed_reply, connectFD) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
			}
			delete change_speed_reply;

			if( removed )
			{
				//*******************************
				// Send Client Notifications
				//*******************************
				MatchLobbyMessage *notification = new MatchLobbyMessage();
				notification->type = PLAYER_LEFT_MATCH_NOTIFICATION;
				notification->playerID = match_lobby_message->playerID;
				pthread_rwlock_rdlock(&matchListLock);
				NotifyClients(matchList[player->currentMatch->GetID()],
						notification);
				pthread_rwlock_unlock(&matchListLock);
				delete notification;
			}

			break;
		}
		default:
		{
			SendError(connectFD, PROTOCOL_ERROR);
			return IN_MATCH_LOBBY;
		}
	}
	return IN_MATCH_LOBBY;
}

//Send a message of type Error to the client
void  RTT::SendError(int connectFD, enum ErrorType errorType)
{
	ErrorMessage *error_msg = new ErrorMessage();
	error_msg->type = MESSAGE_ERROR;
	error_msg->errorType = errorType;
	if(  Message::WriteMessage(error_msg, connectFD) == false)
	{
		cerr << "ERROR: Error message send returned failure.\n";
	}
	delete error_msg;
}

bool RTT::NotifyClients(Match *match, MatchLobbyMessage *message)
{
	bool fullSuccess = true;
	if( match == NULL)
	{
		return false;
	}
	for(uint i = 0; i < MAX_TEAMS; i++)
	{
		vector<Player*>::iterator it = match->teams[i]->players.begin();
		for( ; it != match->teams[i]->players.end(); it++ )
		{
			int recvSocket = (*it)->receiveSocket;
			if(  Message::WriteMessage(message, recvSocket == false) )
			{
				cerr << "ERROR: Message send returned failure.\n";
			}
			Message *message_ack = Message::ReadMessage(recvSocket);
			if( message_ack == NULL )
			{
				cerr << "ERROR: Failed to receive an ack from: "
						<< (*it)->GetName() << "\n";
			}
			//TODO: Not strictly correct. We only want to allow ACKs
			if( message_ack->type < CHANGE_TEAM_REQUEST ||
					message_ack->type > MATCH_START_ACK)
			{
				//Got a bad return message. Should have been an ack
				fullSuccess = false;
			}
			delete message_ack;
		}
	}
	return fullSuccess;
}

int RTT::MatchLobbyConnectBack(int oldSocket, uint portNum, Player *player)
{
	//Set up a new server on the ConnectBack port
	struct sockaddr_in stSockAddr;
	int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(-1 == SocketFD)
	{
		perror("can not create socket");
		return -1;
	}
	int optval = 1;
	setsockopt(SocketFD, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

	memset(&stSockAddr, 0, sizeof(stSockAddr));

	stSockAddr.sin_family = AF_INET;
	stSockAddr.sin_port = htons(portNum);
	stSockAddr.sin_addr.s_addr = INADDR_ANY;

	if(-1 == bind(SocketFD,(struct sockaddr *)&stSockAddr, sizeof(stSockAddr)))
	{
		perror("error bind failed");
		close(SocketFD);
		return -1;
	}

	if(-1 == listen(SocketFD, 10))
	{
		perror("error listen failed");
		close(SocketFD);
		return -1;
	}

	//Tell client we're ready
	MatchLobbyMessage *connect_back_ready = new MatchLobbyMessage();
	connect_back_ready->type = CONNECT_BACK_SERVER_READY;
	connect_back_ready->portNum = portNum;
	if(  Message::WriteMessage(connect_back_ready, oldSocket) == false)
	{
		//Error in write, do something?
		cerr << "ERROR: Message send returned failure.\n";
	}
	delete connect_back_ready;

	//Listen for our client to connect back on the new port...
	int connectBackSocket = accept(SocketFD, NULL, NULL);

	//Read ready-to-go message
	Message *connect_back_reply = Message::ReadMessage(connectBackSocket);
	if( connect_back_reply == NULL )
	{
		//ERROR
		cerr << "ERROR: ConnectBack message came back NULL\n";
		return -1;
	}
	if( connect_back_reply->type != CONNECT_BACK_CLIENT_REQUEST )
	{
		//ERROR
		cerr << "ERROR: ConnectBack message was wrong type\n";
		return -1;
	}
	MatchLobbyMessage *match_connect_back_reply =
			(MatchLobbyMessage*)connect_back_reply;

	//We got the correct player on the first try. Yay!
	if( match_connect_back_reply->playerID == player->GetID())
	{
		//The client should now be listening for a message on this socket
		return connectBackSocket;
	}
	//This was the wrong player!
	//	Store this into the ConnectBack player pool then
	//	Wait for our player to appear in the pool
	else
	{
		int returnSocket;
		//Store player into waitPool:
		connectBackWaitPool[match_connect_back_reply->playerID] = connectBackSocket;

		//Try again for CALLBACK_WAIT_TIME seconds
		pthread_rwlock_wrlock(&waitPoolLock);
		for(uint i = 0; i < CALLBACK_WAIT_TIME; i++ )
		{
			if( connectBackWaitPool.count( player->GetID() ) == 0 )
			{
				//Player not in the pool. Wait and try again
				pthread_rwlock_unlock(&waitPoolLock);
				sleep(1); //Unlock for the sleep
				pthread_rwlock_wrlock(&waitPoolLock);
			}
			else
			{
				//Found our player in the pool!
				//Get the socket value and erase the record
				returnSocket = connectBackWaitPool[player->GetID()];
				connectBackWaitPool.erase(player->GetID());
				pthread_rwlock_unlock(&waitPoolLock);
				return returnSocket;
			}
		}
		pthread_rwlock_unlock(&waitPoolLock);
		cerr << "ERROR: Player never called back\n";
		return -1;
	}
}

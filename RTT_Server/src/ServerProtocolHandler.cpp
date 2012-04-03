//============================================================================
// Name        : ServerProtocolHandler.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Code for handling socket IO on the server side
//============================================================================

#include "RTT_Server.h"
#include "ServerProtocolHandler.h"
#include "MatchLoop.h"
#include "messages/GameMessage.h"

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
	if( client_hello_init->m_type != CLIENT_HELLO )
	{
		cerr << "ERROR: Expected CLIENT_HELLO message, received: "
				<< client_hello_init->m_type << "\n";
		SendError(ConnectFD, PROTOCOL_ERROR);
		delete client_hello_init;
		return NULL;
	}

	AuthMessage *client_hello = (AuthMessage*)client_hello_init;
	//Check version compatibility
	if ((client_hello->m_softwareVersion.m_major != SERVER_VERSION_MAJOR) ||
		(client_hello->m_softwareVersion.m_minor != SERVER_VERSION_MINOR) ||
		(client_hello->m_softwareVersion.m_rev != SERVER_VERSION_REV) )
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
	AuthMessage server_hello;
	server_hello.m_type = SERVER_HELLO;
	server_hello.m_softwareVersion.m_major = SERVER_VERSION_MAJOR;
	server_hello.m_softwareVersion.m_minor = SERVER_VERSION_MINOR;
	server_hello.m_softwareVersion.m_rev = SERVER_VERSION_REV;

	if(  Message::WriteMessage(&server_hello, ConnectFD) == false)
	{
		//Error in write
		return NULL;
	}

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
	if( client_auth_init->m_type != CLIENT_AUTH)
	{
		//Error
		SendError(ConnectFD, PROTOCOL_ERROR);
		delete client_auth_init;
		return NULL;
	}

	AuthMessage *client_auth = (AuthMessage*)client_auth_init;
	enum AuthResult authresult =
			AuthenticateClient(client_auth->m_username, client_auth->m_hashedPassword);

	Player *player = NULL;
	if( authresult == AUTH_SUCCESS)
	{
		pthread_rwlock_wrlock(&playerIDLock);
		uint ID = ++lastPlayerID;
		pthread_rwlock_unlock(&playerIDLock);

		player = new Player(client_auth->m_username, ID);
		pthread_rwlock_wrlock(&playerListLock);
		playerList[ID] = player;
		pthread_rwlock_unlock(&playerListLock);
	}

	delete client_auth;


	//***************************
	// Send Server Auth Reply
	//***************************
	AuthMessage server_auth_reply;
	server_auth_reply.m_type = SERVER_AUTH_REPLY;
	server_auth_reply.m_authSuccess = authresult;
	if(player != NULL)
	{
		server_auth_reply.m_playerDescription = player->GetDescription();
	}
	if(  Message::WriteMessage(&server_auth_reply, ConnectFD) == false)
	{
		//Error in write
		return NULL;
	}

	return player;
}

//Processes one Lobby command
//	Starts out by listening on the given socket for a LobbyMessage
//	Executes the Lobby protocol
//	Returns a enum LobbyReturn to describe the end state
enum LobbyReturn RTT::ProcessLobbyCommand(int ConnectFD, Player *player)
{
	if( player == NULL )
	{
		return EXITING_SERVER;
	}

	uint playerMatchID = player->GetCurrentMatchID();

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
	switch (lobby_message->m_type)
	{
		case MATCH_LIST_REQUEST:
		{
			struct MatchDescription matches[MATCHES_PER_PAGE];
			uint matchCount = GetMatchDescriptions(lobby_message->m_requestedPage, matches);

			//***************************
			// Send Query Reply
			//***************************
			LobbyMessage query_reply;
			query_reply.m_type = MATCH_LIST_REPLY;
			query_reply.m_returnedMatchesCount = matchCount;
			query_reply.m_matchDescriptions = (MatchDescription*)
					malloc(sizeof(struct MatchDescription) * matchCount);
			for(uint i = 0; i < matchCount; i++ )
			{
				query_reply.m_matchDescriptions[i] = matches[i];
			}
			if(  Message::WriteMessage(&query_reply, ConnectFD) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
			}

			delete lobby_message;
			return IN_MAIN_LOBBY;
		}
		case MATCH_CREATE_REQUEST:
		{
			//***************************
			// Send Options Available
			//***************************
			LobbyMessage options_available;
			options_available.m_type = MATCH_CREATE_OPTIONS_AVAILABLE;
			options_available.m_options.m_maxPlayers = MAX_PLAYERS_IN_MATCH;
			if(  Message::WriteMessage(&options_available, ConnectFD) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
				delete lobby_message;
				return IN_MAIN_LOBBY;
			}

			//********************************
			// Receive Options Chosen
			//********************************
			Message *options_chosen_init = Message::ReadMessage(ConnectFD);
			if( options_chosen_init == NULL )
			{
				//Error
				cerr << "ERROR: Reading from client failed.\n";
				SendError(ConnectFD, PROTOCOL_ERROR);
				delete lobby_message;
				return IN_MAIN_LOBBY;
			}
			if( options_chosen_init->m_type !=  MATCH_CREATE_OPTIONS_CHOSEN)
			{
				//Error
				cerr << "ERROR: Client gave us the wrong message type.\n";
				SendError(ConnectFD, PROTOCOL_ERROR);
				delete options_chosen_init;
				delete lobby_message;
				return IN_MAIN_LOBBY;
			}

			LobbyMessage *options_chosen = (LobbyMessage*)options_chosen_init;
			if((options_chosen->m_options.m_maxPlayers > MAX_PLAYERS_IN_MATCH) ||
					(options_chosen->m_options.m_maxPlayers < 2))
			{
				cerr << "ERROR: Client asked an invalid number of max playersb.\n";
				SendError(ConnectFD, INVALID_MAX_PLAYERS);
				delete lobby_message;
				return IN_MAIN_LOBBY;
			}

			uint matchID = RegisterNewMatch(player, options_chosen->m_options);

			if( matchID == 0 )
			{
				//TODO: Find a better error message hereb
				SendError(ConnectFD, TOO_BUSY);
				delete lobby_message;
				return IN_MAIN_LOBBY;
			}
			delete options_chosen;

			//***************************
			// Send Match Create Reply
			//***************************
			LobbyMessage create_reply;
			create_reply.m_type = MATCH_CREATE_REPLY;
			pthread_rwlock_rdlock(&matchListLock);
			Match *joinedMatch = matchList[matchID];
			create_reply.m_matchDescription = joinedMatch->GetDescription();
			pthread_rwlock_unlock(&matchListLock);
			if(  Message::WriteMessage(&create_reply, ConnectFD) == false)
			{
				//Error in write, do something?
				//TODO: This case is awkward. Should probably
				//	remove player from the match?
				cerr << "ERROR: Message send returned failure.\n";
				delete lobby_message;
				return IN_MAIN_LOBBY;
			}
			delete lobby_message;
			return IN_MATCH_LOBBY;
		}
		case MATCH_JOIN_REQUEST:
		{
			//Try to join the match, process result
			enum LobbyResult joinResult = JoinMatch(player, lobby_message->m_ID);
			switch(joinResult)
			{
				enum ErrorType errorType;
				case LOBBY_SUCCESS:
				{
					//***************************
					// Send Match Join Reply
					//***************************
					LobbyMessage match_join;
					match_join.m_type = MATCH_JOIN_REPLY;

					pthread_rwlock_rdlock(&matchListLock);
					Match *joinedMatch = matchList[lobby_message->m_ID];
					pthread_rwlock_unlock(&matchListLock);

					match_join.m_matchDescription = joinedMatch->GetDescription();

					//Put in the player descriptions of current members
					match_join.m_playerDescriptions = (struct PlayerDescription*)
						malloc(sizeof(struct PlayerDescription) * MAX_PLAYERS_IN_MATCH);

					uint count = GetPlayerDescriptions(joinedMatch->GetID(),
						match_join.m_playerDescriptions);
					match_join.m_returnedPlayersCount = count;



					if(  Message::WriteMessage(&match_join, ConnectFD) == false)
					{
						//Error in write, do something?
						cerr << "ERROR: Message send returned failure.\n";
					}

					//*******************************
					// Send Client Notifications
					//*******************************
					MatchLobbyMessage notification;
					notification.m_type = PLAYER_JOINED_MATCH_NOTIFICATION;
					notification.m_playerDescription = player->GetDescription();

					NotifyClients(joinedMatch, &notification);
					delete lobby_message;
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
				delete lobby_message;
				return IN_MAIN_LOBBY;
			}
			break;
		}
		case SERVER_STATS_REQUEST:
		{
			//*******************************
			// Send Server Stats Reply
			//*******************************
			LobbyMessage stats_reply;
			stats_reply.m_type = SERVER_STATS_REPLY;

			pthread_rwlock_rdlock(&matchListLock);
			stats_reply.m_serverStats.m_numMatches = matchList.size();
			pthread_rwlock_unlock(&matchListLock);

			pthread_rwlock_rdlock(&playerListLock);
			stats_reply.m_serverStats.m_numPlayers = playerList.size();
			pthread_rwlock_unlock(&playerListLock);

			if( Message::WriteMessage(&stats_reply, ConnectFD) == false )
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
			}
			delete lobby_message;
			return IN_MAIN_LOBBY;
		}
		case MATCH_EXIT_SERVER_NOTIFICATION:
		{
			//*******************************
			// Send Exit Server Acknowledge
			//*******************************
			LobbyMessage exit_server;
			exit_server.m_type = MATCH_EXIT_SERVER_ACKNOWLEDGE;
			if(  Message::WriteMessage(&exit_server, ConnectFD) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
			}
			delete lobby_message;
			return EXITING_SERVER;
		}
		default:
		{
			SendError(ConnectFD, PROTOCOL_ERROR);
			delete lobby_message_init;
			lobby_message_init = NULL;
			return IN_MAIN_LOBBY;
		}
	}
	delete lobby_message;
	return IN_MAIN_LOBBY;	//TODO: IS this the right place to return?
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
	if( player == NULL )
	{
		cerr << "ERROR: Tried to process MatchLobby command for NULL player\n";
		return EXITING_SERVER;
	}

	uint matchID = player->GetCurrentMatchID();
	uint playerID = player->GetID();

	pthread_rwlock_rdlock(&matchListLock);
	Match *playersMatch = NULL;
	if( matchList.count(matchID) != 0)
	{
		playersMatch = matchList[matchID];
	}
	pthread_rwlock_unlock(&matchListLock);

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
	switch (match_lobby_message->m_type)
	{
		case MATCH_LEAVE_NOTIFICATION:
		{
			if( matchID == 0 )
			{
				SendError(connectFD, NOT_IN_THAT_MATCH);
				delete match_lobby_message;
				return IN_MAIN_LOBBY;
			}
			if( LeaveMatch(player) )
			{
				//*******************************
				// Send Match Leave Acknowledge
				//*******************************
				MatchLobbyMessage leave_ack;
				leave_ack.m_type = MATCH_LEAVE_ACKNOWLEDGE;
				if(  Message::WriteMessage(&leave_ack, connectFD) == false)
				{
					//Error in write, do something?
					cerr << "ERROR: Message send returned failure.\n";
				}
				delete match_lobby_message;
				return IN_MAIN_LOBBY;
			}
			else
			{
				SendError(connectFD, NOT_IN_THAT_MATCH);
				delete match_lobby_message;
				if( matchID == 0)
				{
					return IN_MAIN_LOBBY;
				}
				return IN_MATCH_LOBBY;
			}
		}
		case CHANGE_TEAM_REQUEST:
		{
			if( matchID == 0 )
			{
				SendError(connectFD, PROTOCOL_ERROR);
				delete match_lobby_message;
				return IN_MAIN_LOBBY;
			}

			if( playersMatch == NULL)
			{
				//Error, there is no such Match
				SendError(connectFD, MATCH_DOESNT_EXIST);
				delete match_lobby_message;
				return IN_MAIN_LOBBY;
			}

			//Is this not a self change?
			if( playerID != match_lobby_message->m_playerID )
			{
				//Is this this not the leader?
				if( playersMatch->GetLeaderID() != playerID)
				{
					//Error, there is no such Match
					SendError(connectFD, NOT_ALLOWED_TO_CHANGE_THAT);
					delete match_lobby_message;
					return IN_MATCH_LOBBY;
				}
			}

			Player *changingPlayer = NULL;
			bool changed = false;

			pthread_rwlock_wrlock(&playerListLock);
			if( playerList.count(match_lobby_message->m_playerID) > 0 )
			{
				changingPlayer = playerList[match_lobby_message->m_playerID];
				pthread_rwlock_unlock(&playerListLock);
				changed = playersMatch->ChangeTeam(changingPlayer
					, match_lobby_message->m_newTeam);
			}
			else
			{
				pthread_rwlock_unlock(&playerListLock);
			}

			//*******************************
			// Send CHANGE TEAM REPLY
			//*******************************
			MatchLobbyMessage change_team_reply;
			change_team_reply.m_type = CHANGE_TEAM_REPLY;
			change_team_reply.m_changeAccepted = changed;
			if(  Message::WriteMessage(&change_team_reply, connectFD) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
			}

			if( changed )
			{
				//*******************************
				// Send Client Notifications
				//*******************************
				MatchLobbyMessage notification;
				notification.m_type = TEAM_CHANGED_NOTIFICATION;
				notification.m_newTeam = match_lobby_message->m_newTeam;
				notification.m_playerID = match_lobby_message->m_playerID;
				NotifyClients(playersMatch, &notification);
			}

			break;
		}
		case START_MATCH_REQUEST:
		{
			if( matchID == 0 )
			{
				SendError(connectFD, PROTOCOL_ERROR);
				delete match_lobby_message;
				return IN_MAIN_LOBBY;
			}

			if(playersMatch == NULL)
			{
				//Error, there is no such Match
				SendError(connectFD, MATCH_DOESNT_EXIST);
				delete match_lobby_message;
				return IN_MAIN_LOBBY;
			}
			//Is this this not the leader?
			if( playersMatch->GetLeaderID() != playerID)
			{
				//Error, there is no such Match
				SendError(connectFD, NOT_ALLOWED_TO_CHANGE_THAT);
				delete match_lobby_message;
				return IN_MATCH_LOBBY;
			}

			bool started = playersMatch->StartMatch();

			//*******************************
			// Send START MATCH REPLY
			//*******************************
			MatchLobbyMessage start_match_reply;
			start_match_reply.m_type = START_MATCH_REPLY;
			start_match_reply.m_changeAccepted = started;
			if(  Message::WriteMessage(&start_match_reply, connectFD) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
			}

			if( started )
			{
				//*******************************
				// Send Client Notifications
				//*******************************
				//TODO: Make sure each client is ready. IE: Listed for replies
				MatchLobbyMessage notification;
				notification.m_type = MATCH_START_NOTIFICATION;
				NotifyClients(playersMatch, &notification);
			}

			break;
		}
		case REGISTER_FOR_MATCH:
		{
			if( matchID == 0 )
			{
				SendError(connectFD, PROTOCOL_ERROR);
				delete match_lobby_message;
				return IN_MAIN_LOBBY;
			}

			if(playersMatch == NULL)
			{
				//Error, there is no such Match
				SendError(connectFD, MATCH_DOESNT_EXIST);
				delete match_lobby_message;
				return IN_MAIN_LOBBY;
			}
			//*******************************
			// Send Register Reply
			//*******************************
			MatchLobbyMessage register_reply;
			register_reply.m_type = REGISTER_REPLY;
			if(  Message::WriteMessage(&register_reply, connectFD) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
				break;
			}

			if(playersMatch->RegisterPlayer(playerID))
			{
				pthread_t matchLoopThread;
				pthread_create(&matchLoopThread, NULL, MatchLoop, (void*)playersMatch);
			}

			delete match_lobby_message;
			return IN_GAME;
		}
		case CHANGE_COLOR_REQUEST:
		{
			if( matchID == 0 )
			{
				SendError(connectFD, PROTOCOL_ERROR);
				delete match_lobby_message;
				return IN_MAIN_LOBBY;
			}

			if( playersMatch == NULL)
			{
				//Error, there is no such Match
				SendError(connectFD, MATCH_DOESNT_EXIST);
				delete match_lobby_message;
				return IN_MAIN_LOBBY;
			}
			//Is this not a self change?
			if( playerID != match_lobby_message->m_playerID )
			{
				//Is this this not the leader?
				if(playersMatch->GetLeaderID() != playerID)
				{
					//Error, not allowed
					SendError(connectFD, NOT_ALLOWED_TO_CHANGE_THAT);
					delete match_lobby_message;
					return IN_MATCH_LOBBY;
				}
			}

			pthread_rwlock_rdlock(&playerListLock);
			if( playerList.count(match_lobby_message->m_playerID) != 0 )
			{
				playerList[match_lobby_message->m_playerID]->SetColor(
						match_lobby_message->m_newColor);
			}
			else
			{
				//Error, there is no such player
				pthread_rwlock_unlock(&playerListLock);
				SendError(connectFD, NO_SUCH_PLAYER);
				delete match_lobby_message;
				return IN_MATCH_LOBBY;
			}
			pthread_rwlock_unlock(&playerListLock);

			//*******************************
			// Send CHANGE COLOR REPLY
			//*******************************
			MatchLobbyMessage change_color_reply;
			change_color_reply.m_type = CHANGE_COLOR_REPLY;
			change_color_reply.m_changeAccepted = true;
			if(  Message::WriteMessage(&change_color_reply, connectFD) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
			}

			//*******************************
			// Send Client Notifications
			//*******************************
			MatchLobbyMessage notification;
			notification.m_type = COLOR_CHANGED_NOTIFICATION;
			notification.m_newColor = match_lobby_message->m_newColor;
			NotifyClients(playersMatch, &notification);

			break;
		}
		case CHANGE_MAP_REQUEST:
		{
			if( matchID == 0 )
			{
				SendError(connectFD, PROTOCOL_ERROR);
				delete match_lobby_message;
				return IN_MAIN_LOBBY;
			}

			if( playersMatch == NULL)
			{
				//Error, there is no such Match
				SendError(connectFD, MATCH_DOESNT_EXIST);
				delete match_lobby_message;
				return IN_MAIN_LOBBY;
			}
			//Is this this not the leader?
			if( playersMatch->GetLeaderID() != playerID)
			{
				//Error, not allowed
				SendError(connectFD, NOT_ALLOWED_TO_CHANGE_THAT);
				delete match_lobby_message;
				return IN_MATCH_LOBBY;
			}

			playersMatch->SetMap(match_lobby_message->m_mapDescription);

			//*******************************
			// Send CHANGE MAP REPLY
			//*******************************
			MatchLobbyMessage change_map_reply;
			change_map_reply.m_type = CHANGE_MAP_REPLY;
			change_map_reply.m_changeAccepted = true;
			if(  Message::WriteMessage(&change_map_reply, connectFD) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
			}

			//*******************************
			// Send Client Notifications
			//*******************************
			MatchLobbyMessage notification;
			notification.m_type = MAP_CHANGED_NOTIFICATION;
			notification.m_mapDescription = match_lobby_message->m_mapDescription;
			NotifyClients(playersMatch, &notification);

			break;
		}
		case CHANGE_VICTORY_COND_REQUEST:
		{
			if( matchID == 0 )
			{
				SendError(connectFD, PROTOCOL_ERROR);
				delete match_lobby_message;
				return IN_MAIN_LOBBY;
			}

			if( playersMatch == NULL)
			{
				//Error, there is no such Match
				SendError(connectFD, MATCH_DOESNT_EXIST);
				delete match_lobby_message;
				return IN_MAIN_LOBBY;
			}
			//Is this this not the leader?
			if( playersMatch->GetLeaderID() != playerID)
			{
				//Error, not allowed
				SendError(connectFD, NOT_ALLOWED_TO_CHANGE_THAT);
				delete match_lobby_message;
				return IN_MATCH_LOBBY;
			}

			playersMatch->SetVictoryCondition(match_lobby_message->m_newVictCond);

			//*************************************
			// Send Change Victory Condition Reply
			//**************************************
			MatchLobbyMessage change_victory_reply;
			change_victory_reply.m_type = CHANGE_VICTORY_COND_REPLY;
			change_victory_reply.m_changeAccepted = true;
			if(  Message::WriteMessage(&change_victory_reply, connectFD) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
			}

			//*******************************
			// Send Client Notifications
			//*******************************
			MatchLobbyMessage notification;
			notification.m_type = VICTORY_COND_CHANGED_NOTIFICATION;
			notification.m_newVictCond = match_lobby_message->m_newVictCond;
			NotifyClients(playersMatch, &notification);

			break;
		}
		case CHANGE_GAME_SPEED_REQUEST:
		{
			if( matchID == 0 )
			{
				SendError(connectFD, PROTOCOL_ERROR);
				delete match_lobby_message;
				return IN_MAIN_LOBBY;
			}

			if( playersMatch == NULL)
			{
				//Error, there is no such Match
				SendError(connectFD, MATCH_DOESNT_EXIST);
				delete match_lobby_message;
				return IN_MAIN_LOBBY;
			}
			//Is this this not the leader?
			if( playersMatch->GetLeaderID() != playerID)
			{
				//Error, not allowed
				SendError(connectFD, NOT_ALLOWED_TO_CHANGE_THAT);
				delete match_lobby_message;
				return IN_MATCH_LOBBY;
			}

			playersMatch->SetGamespeed(match_lobby_message->m_newSpeed);

			//*******************************
			// Send Change Game Speed Reply
			//*******************************
			MatchLobbyMessage change_speed_reply;
			change_speed_reply.m_type = CHANGE_GAME_SPEED_REPLY;
			change_speed_reply.m_changeAccepted = true;
			if(  Message::WriteMessage(&change_speed_reply, connectFD) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
			}

			//*******************************
			// Send Client Notifications
			//*******************************
			MatchLobbyMessage notification;
			notification.m_type = GAME_SPEED_CHANGED_NOTIFICATION;
			notification.m_newSpeed = match_lobby_message->m_newSpeed;
			NotifyClients(playersMatch, &notification);

			break;
		}
		case CHANGE_LEADER_REQUEST:
		{
			if( matchID == 0 )
			{
				SendError(connectFD, PROTOCOL_ERROR);
				delete match_lobby_message;
				return IN_MAIN_LOBBY;
			}

			if( playersMatch == NULL)
			{
				//Error, there is no such Match
				SendError(connectFD, MATCH_DOESNT_EXIST);
				delete match_lobby_message;
				return IN_MAIN_LOBBY;
			}

			bool changed = false;
			//Only allow the current leader to set a new leader
			if( playerID ==  playersMatch->GetLeaderID())
			{
				changed = playersMatch->SetLeader(match_lobby_message->m_playerID);
			}

			//*******************************
			// Send Change Leader Reply
			//*******************************
			MatchLobbyMessage change_leader_reply;
			change_leader_reply.m_type = CHANGE_LEADER_REPLY;
			change_leader_reply.m_changeAccepted = changed;
			if(  Message::WriteMessage(&change_leader_reply, connectFD) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
			}

			if( changed )
			{
				//*******************************
				// Send Client Notifications
				//*******************************
				MatchLobbyMessage notification;
				notification.m_type = CHANGE_LEADER_NOTIFICATION;
				notification.m_playerID = match_lobby_message->m_playerID;
				NotifyClients(playersMatch, &notification);
			}

			return IN_MATCH_LOBBY;
		}
		case KICK_PLAYER_REQUEST:
		{
			if( matchID == 0 )
			{
				SendError(connectFD, PROTOCOL_ERROR);
				delete match_lobby_message;
				return IN_MAIN_LOBBY;
			}

			if( playersMatch == NULL)
			{
				//Error, there is no such Match
				SendError(connectFD, MATCH_DOESNT_EXIST);
				delete match_lobby_message;
				return IN_MAIN_LOBBY;
			}
			//Is this this not the leader?
			if( playersMatch->GetLeaderID() != playerID)
			{
				//Error, not allowed
				SendError(connectFD, NOT_ALLOWED_TO_CHANGE_THAT);
				delete match_lobby_message;
				return IN_MATCH_LOBBY;
			}

			bool removed = playersMatch->RemovePlayer(match_lobby_message->m_playerID);

			//*******************************
			// Send Kick Player Reply
			//*******************************
			MatchLobbyMessage kick_player_reply;
			kick_player_reply.m_type = KICK_PLAYER_REPLY;
			kick_player_reply.m_changeAccepted = removed;
			if(  Message::WriteMessage(&kick_player_reply, connectFD) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
			}

			if( removed )
			{
				//*******************************
				// Send Client Notifications
				//*******************************
				MatchLobbyMessage notification;
				notification.m_type = PLAYER_LEFT_MATCH_NOTIFICATION;
				notification.m_playerID = match_lobby_message->m_playerID;
				notification.m_newLeaderID = playersMatch->GetLeaderID();
				NotifyClients(playersMatch, &notification);
			}

			break;
		}
		default:
		{
			SendError(connectFD, PROTOCOL_ERROR);
			delete match_lobby_message_init;
			match_lobby_message_init = NULL;
			return IN_MATCH_LOBBY;
		}
	}
	delete match_lobby_message;
	return IN_MATCH_LOBBY;
}

//Processes one game command
//	Starts out by listening on the given socket for a GameMessage
//	Executes the game protocol
//	Returns a enum LobbyReturn to describe the end state
enum LobbyReturn RTT::ProcessGameCommand(int connectFD, Player *player)
{
	if(player == NULL)
	{
		cerr << "ERROR: Player in game does not exist\n";
		return EXITING_SERVER;
	}

	uint matchID = player->GetCurrentMatchID();

	pthread_rwlock_rdlock(&matchListLock);
	Match *playersMatch = NULL;
	if( matchList.count(matchID) != 0)
	{
		playersMatch = matchList[matchID];
	}
	pthread_rwlock_unlock(&matchListLock);

	Message *message = Message::ReadMessage(connectFD);
	if(message == NULL)
	{
		//ERROR
		cerr << "WARNING: Game message read failed\n";
		SendError(connectFD, PROTOCOL_ERROR);
		return EXITING_SERVER;
	}
	GameMessage *game_message = (GameMessage*)message;
	switch(game_message->m_type)
	{
		case MOVE_UNIT_DIRECTION_REQUEST:
		{
			//TODO: Check to see if the move is legal
			//TODO: Move the unit around in the gameboard

			GameMessage move_reply;
			move_reply.m_type = MOVE_UNIT_DIRECTION_REPLY;
			move_reply.m_moveResult = MOVE_SUCCESS;
			GameMessage::WriteMessage(&move_reply, connectFD);

			GameMessage move_notice;
			move_notice.m_type = UNIT_MOVED_DIRECTION_NOTICE;
			move_notice.m_unitID = game_message->m_unitID;
			move_notice.m_xOld = game_message->m_xOld;
			move_notice.m_yOld = game_message->m_yOld;
			move_notice.m_direction = game_message->m_direction;

			NotifyClients(playersMatch, &move_notice);

			return IN_GAME;
		}
		default:
		{
			return IN_GAME;
		}
	}

	return IN_GAME;
}

//Send a message of type Error to the client
void  RTT::SendError(int connectFD, enum ErrorType errorType)
{
	ErrorMessage error_msg;
	error_msg.m_type = MESSAGE_ERROR;
	error_msg.m_errorType = errorType;
	if(  Message::WriteMessage(&error_msg, connectFD) == false)
	{
		cerr << "ERROR: Error message send returned failure.\n";
	}
}

bool RTT::NotifyClients(Match *match, Message *message)
{
	bool fullSuccess = true;
	if( match == NULL)
	{
		return false;
	}
	for(uint i = 0; i < MAX_TEAMS; i++)
	{
		vector<Player*> players = match->m_teams[i]->GetPlayers();
		vector<Player*>::iterator it = players.begin();
		for( ; it != players.end(); it++ )
		{
			int recvSocket = (*it)->GetCallbackSocket();
			if( recvSocket >= 0 )
			{
				if(  Message::WriteMessage(message, recvSocket) == false )
				{
					cerr << "ERROR: Message send returned failure.\n";
				}
				Message *message_ack = Message::ReadMessage(recvSocket);
				if( message_ack == NULL )
				{
					fullSuccess = false;
					continue;
				}
				//TODO: Not strictly correct. We only want to allow ACKs
				if( message_ack->m_type < CHANGE_TEAM_REQUEST ||
						message_ack->m_type > MATCH_START_ACK)
				{
					//Got a bad return message. Should have been an ack
					fullSuccess = false;
				}
				delete message_ack;
			}
		}
	}
	return fullSuccess;
}


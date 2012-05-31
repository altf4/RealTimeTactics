//============================================================================
// Name        : ServerProtocolHandler.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Code for handling socket IO on the server side
//============================================================================

#include "RTT_Server.h"
#include "ServerProtocolHandler.h"
#include "messaging/MessageManager.h"
#include "Lock.h"

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

extern pthread_rwlock_t matchListLock;
extern pthread_rwlock_t playerListLock;

extern pthread_rwlock_t playerIDLock;
extern uint lastPlayerID;

//Negotiates the hello messages and authentication to a new client
//	Returns a new Player object, NULL on error
Player *RTT::GetNewClient(int socketFD)
{
	if(!MessageManager::Instance().RegisterCallback(socketFD))
	{
		return NULL;
	}

	//***************************
	// Read client Hello
	//***************************
	Message *client_hello_init = Message::ReadMessage(socketFD, DIRECTION_TO_SERVER);
	if( client_hello_init->m_messageType != MESSAGE_AUTH)
	{
		SendError(socketFD, PROTOCOL_ERROR, DIRECTION_TO_SERVER);
		delete client_hello_init;
		return NULL;
	}

	AuthMessage *client_hello = (AuthMessage*)client_hello_init;
	if(client_hello->m_authType != CLIENT_HELLO)
	{
		cerr << "ERROR: Expected CLIENT_HELLO message, received: "
				<< client_hello->m_authType << "\n";
		SendError(socketFD, PROTOCOL_ERROR, DIRECTION_TO_SERVER);
		delete client_hello;
		return NULL;
	}

	//Check version compatibility
	if ((client_hello->m_softwareVersion.m_major != SERVER_VERSION_MAJOR) ||
		(client_hello->m_softwareVersion.m_minor != SERVER_VERSION_MINOR) ||
		(client_hello->m_softwareVersion.m_rev != SERVER_VERSION_REV) )
	{
		//If versions are not the same, send an error message to the client
		delete client_hello;
		cout << "Client Connected with bad software version.\n";
		SendError(socketFD, INCOMPATIBLE_SOFTWARE_VERSION, DIRECTION_TO_SERVER);
		return NULL;
	}
	delete client_hello;


	//***************************
	// Send Server Hello
	//***************************
	AuthMessage server_hello(SERVER_HELLO, DIRECTION_TO_SERVER);
	server_hello.m_softwareVersion.m_major = SERVER_VERSION_MAJOR;
	server_hello.m_softwareVersion.m_minor = SERVER_VERSION_MINOR;
	server_hello.m_softwareVersion.m_rev = SERVER_VERSION_REV;
	server_hello.m_authMechanism = HASHED_SALTED_PASS;

	if(  Message::WriteMessage(&server_hello, socketFD) == false)
	{
		//Error in write
		return NULL;
	}

	//***************************
	// Receive Client Auth
	//***************************
	Message *client_auth_init = Message::ReadMessage(socketFD, DIRECTION_TO_SERVER);
	if( client_auth_init->m_messageType != MESSAGE_AUTH)
	{
		//Error
		SendError(socketFD, PROTOCOL_ERROR, DIRECTION_TO_SERVER);
		delete client_auth_init;
		return NULL;
	}
	AuthMessage *client_auth = (AuthMessage*)client_auth_init;
	if(client_auth->m_authType != CLIENT_AUTH)
	{
		//Error
		SendError(socketFD, PROTOCOL_ERROR, DIRECTION_TO_SERVER);
		delete client_auth;
		return NULL;
	}

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
	AuthMessage server_auth_reply(SERVER_AUTH_REPLY, DIRECTION_TO_SERVER);
	server_auth_reply.m_authSuccess = authresult;
	if( player != NULL )
	{
		server_auth_reply.m_playerDescription = player->GetDescription();
	}
	if( Message::WriteMessage(&server_auth_reply, socketFD) == false)
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
enum LobbyReturn RTT::ProcessLobbyCommand(int socketFD, Player *player)
{
	if( player == NULL )
	{
		return EXITING_SERVER;
	}

	//uint playerMatchID = player->GetCurrentMatchID();

	if(!MessageManager::Instance().RegisterCallback(socketFD))
	{
		return EXITING_SERVER;
	}

	//********************************
	// Receive Initial Lobby Message
	//********************************
	Message *lobby_message_init = Message::ReadMessage(socketFD, DIRECTION_TO_SERVER);
	if( lobby_message_init->m_messageType != MESSAGE_LOBBY )
	{
		//ERROR
		SendError(socketFD, PROTOCOL_ERROR, DIRECTION_TO_SERVER);
		return EXITING_SERVER;
	}

	LobbyMessage *lobby_message = (LobbyMessage*)lobby_message_init;
	switch (lobby_message->m_lobbyType)
	{
		case MATCH_LIST_REQUEST:
		{
			struct MatchDescription matches[MATCHES_PER_PAGE];
			uint matchCount = GetMatchDescriptions(lobby_message->m_requestedPage, matches);

			//***************************
			// Send Query Reply
			//***************************
			LobbyMessage query_reply(MATCH_LIST_REPLY, DIRECTION_TO_SERVER);
			query_reply.m_returnedMatchesCount = matchCount;
			query_reply.m_matchDescriptions = (MatchDescription*)
					malloc(sizeof(struct MatchDescription) * matchCount);
			for(uint i = 0; i < matchCount; i++ )
			{
				query_reply.m_matchDescriptions[i] = matches[i];
			}
			if( Message::WriteMessage(&query_reply, socketFD) == false)
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
			LobbyMessage options_available(MATCH_CREATE_OPTIONS_AVAILABLE, DIRECTION_TO_SERVER);
			options_available.m_options.m_maxPlayers = MAX_PLAYERS_IN_MATCH;
			if(  Message::WriteMessage(&options_available, socketFD) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
				delete lobby_message;
				return IN_MAIN_LOBBY;
			}

			//********************************
			// Receive Options Chosen
			//********************************
			Message *options_chosen_init = Message::ReadMessage(socketFD, DIRECTION_TO_SERVER);
			if( options_chosen_init->m_messageType !=  MESSAGE_LOBBY)
			{
				//Error
				cerr << "ERROR: Client gave us the wrong message type.\n";
				SendError(socketFD, PROTOCOL_ERROR, DIRECTION_TO_SERVER);
				delete options_chosen_init;
				delete lobby_message;
				return IN_MAIN_LOBBY;
			}

			LobbyMessage *options_chosen = (LobbyMessage*)options_chosen_init;
			if(options_chosen->m_lobbyType != MATCH_CREATE_OPTIONS_CHOSEN)
			{
				//Error
				cerr << "ERROR: Client gave us the wrong message type.\n";
				SendError(socketFD, PROTOCOL_ERROR, DIRECTION_TO_SERVER);
				delete options_chosen;
				delete lobby_message;
				return IN_MAIN_LOBBY;
			}

			if((options_chosen->m_options.m_maxPlayers > MAX_PLAYERS_IN_MATCH) ||
					(options_chosen->m_options.m_maxPlayers < 2))
			{
				cerr << "ERROR: Client asked an invalid number of max playersb.\n";
				SendError(socketFD, INVALID_MAX_PLAYERS, DIRECTION_TO_SERVER);
				delete lobby_message;
				return IN_MAIN_LOBBY;
			}

			uint matchID = RegisterNewMatch(player, options_chosen->m_options);

			if( matchID == 0 )
			{
				//TODO: Find a better error message hereb
				SendError(socketFD, TOO_BUSY, DIRECTION_TO_SERVER);
				delete lobby_message;
				return IN_MAIN_LOBBY;
			}
			delete options_chosen;

			//***************************
			// Send Match Create Reply
			//***************************
			LobbyMessage create_reply(MATCH_CREATE_REPLY, DIRECTION_TO_SERVER);
			pthread_rwlock_rdlock(&matchListLock);
			Match *joinedMatch = matchList[matchID];
			create_reply.m_matchDescription = joinedMatch->GetDescription();
			pthread_rwlock_unlock(&matchListLock);
			if( Message::WriteMessage(&create_reply, socketFD) == false)
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
					LobbyMessage match_join(MATCH_JOIN_REPLY, DIRECTION_TO_SERVER);

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



					if( Message::WriteMessage(&match_join, socketFD) == false)
					{
						//Error in write, do something?
						cerr << "ERROR: Message send returned failure.\n";
					}

					//*******************************
					// Send Client Notifications
					//*******************************
					MatchLobbyMessage notification(PLAYER_JOINED_MATCH_NOTIFICATION, DIRECTION_TO_CLIENT);
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
				SendError(socketFD, errorType, DIRECTION_TO_SERVER);
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
			LobbyMessage stats_reply(SERVER_STATS_REPLY, DIRECTION_TO_SERVER);

			pthread_rwlock_rdlock(&matchListLock);
			stats_reply.m_serverStats.m_numMatches = matchList.size();
			pthread_rwlock_unlock(&matchListLock);

			pthread_rwlock_rdlock(&playerListLock);
			stats_reply.m_serverStats.m_numPlayers = playerList.size();
			pthread_rwlock_unlock(&playerListLock);

			if( Message::WriteMessage(&stats_reply, socketFD) == false )
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
			LobbyMessage exit_server(MATCH_EXIT_SERVER_ACKNOWLEDGE, DIRECTION_TO_SERVER);
			if( Message::WriteMessage(&exit_server, socketFD) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
			}
			delete lobby_message;
			return EXITING_SERVER;
		}
		default:
		{
			SendError(socketFD, PROTOCOL_ERROR, DIRECTION_TO_SERVER);
			delete lobby_message_init;
			lobby_message_init = NULL;
			return IN_MAIN_LOBBY;
		}
	}
	delete lobby_message;
	cerr << "ERROR: Should probably not get here... Exiting client handler" << endl;
	return EXITING_SERVER;
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
enum LobbyReturn RTT::ProcessMatchLobbyCommand(int socketFD, Player *player)
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

	if(!MessageManager::Instance().RegisterCallback(socketFD))
	{
		return EXITING_SERVER;
	}

	//********************************
	// Receive Initial MatchLobby Message
	//********************************
	Message *match_lobby_message_init = Message::ReadMessage(socketFD, DIRECTION_TO_SERVER);
	if( match_lobby_message_init->m_messageType != MESSAGE_MATCH_LOBBY )
	{
		//ERROR
		cerr << "ERROR: Lobby message came back NULL\n";
		SendError(socketFD, PROTOCOL_ERROR, DIRECTION_TO_SERVER);
		return EXITING_SERVER;
	}

	MatchLobbyMessage *match_lobby_message = (MatchLobbyMessage*)match_lobby_message_init;
	switch (match_lobby_message->m_matchLobbyType)
	{
		case MATCH_LEAVE_NOTIFICATION:
		{
			if( matchID == 0 )
			{
				SendError(socketFD, NOT_IN_THAT_MATCH, DIRECTION_TO_SERVER);
				delete match_lobby_message;
				return IN_MAIN_LOBBY;
			}
			if( LeaveMatch(player) )
			{
				//*******************************
				// Send Match Leave Acknowledge
				//*******************************
				MatchLobbyMessage leave_ack(MATCH_LEAVE_ACKNOWLEDGE, DIRECTION_TO_SERVER);
				if(  Message::WriteMessage(&leave_ack, socketFD) == false)
				{
					//Error in write, do something?
					cerr << "ERROR: Message send returned failure.\n";
				}
				delete match_lobby_message;
				return IN_MAIN_LOBBY;
			}
			else
			{
				SendError(socketFD, NOT_IN_THAT_MATCH, DIRECTION_TO_SERVER);
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
				SendError(socketFD, PROTOCOL_ERROR, DIRECTION_TO_SERVER);
				delete match_lobby_message;
				return IN_MAIN_LOBBY;
			}

			if( playersMatch == NULL)
			{
				//Error, there is no such Match
				SendError(socketFD, MATCH_DOESNT_EXIST, DIRECTION_TO_SERVER);
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
					SendError(socketFD, NOT_ALLOWED_TO_CHANGE_THAT, DIRECTION_TO_SERVER);
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
			MatchLobbyMessage change_team_reply(CHANGE_TEAM_REPLY, DIRECTION_TO_SERVER);
			change_team_reply.m_changeAccepted = changed;
			if(  Message::WriteMessage(&change_team_reply, socketFD) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
			}

			if( changed )
			{
				//*******************************
				// Send Client Notifications
				//*******************************
				MatchLobbyMessage notification(TEAM_CHANGED_NOTIFICATION, DIRECTION_TO_CLIENT);
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
				SendError(socketFD, PROTOCOL_ERROR, DIRECTION_TO_SERVER);
				delete match_lobby_message;
				return IN_MAIN_LOBBY;
			}

			if(playersMatch == NULL)
			{
				//Error, there is no such Match
				SendError(socketFD, MATCH_DOESNT_EXIST, DIRECTION_TO_SERVER);
				delete match_lobby_message;
				return IN_MAIN_LOBBY;
			}
			//Is this this not the leader?
			if( playersMatch->GetLeaderID() != playerID)
			{
				//Error, there is no such Match
				SendError(socketFD, NOT_ALLOWED_TO_CHANGE_THAT, DIRECTION_TO_SERVER);
				delete match_lobby_message;
				return IN_MATCH_LOBBY;
			}

			bool started = playersMatch->StartMatch();

			//*******************************
			// Send START MATCH REPLY
			//*******************************
			MatchLobbyMessage start_match_reply(START_MATCH_REPLY, DIRECTION_TO_SERVER);
			start_match_reply.m_changeAccepted = started;
			if(  Message::WriteMessage(&start_match_reply, socketFD) == false)
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
				MatchLobbyMessage notification(MATCH_START_NOTIFICATION, DIRECTION_TO_CLIENT);
				NotifyClients(playersMatch, &notification);
			}

			break;
		}
		case CHANGE_COLOR_REQUEST:
		{
			if( matchID == 0 )
			{
				SendError(socketFD, PROTOCOL_ERROR, DIRECTION_TO_SERVER);
				delete match_lobby_message;
				return IN_MAIN_LOBBY;
			}

			if( playersMatch == NULL)
			{
				//Error, there is no such Match
				SendError(socketFD, MATCH_DOESNT_EXIST, DIRECTION_TO_SERVER);
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
					SendError(socketFD, NOT_ALLOWED_TO_CHANGE_THAT, DIRECTION_TO_SERVER);
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
				SendError(socketFD, NO_SUCH_PLAYER, DIRECTION_TO_SERVER);
				delete match_lobby_message;
				return IN_MATCH_LOBBY;
			}
			pthread_rwlock_unlock(&playerListLock);

			//*******************************
			// Send CHANGE COLOR REPLY
			//*******************************
			MatchLobbyMessage change_color_reply(CHANGE_COLOR_REPLY, DIRECTION_TO_SERVER);
			change_color_reply.m_changeAccepted = true;
			if(  Message::WriteMessage(&change_color_reply, socketFD) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
			}

			//*******************************
			// Send Client Notifications
			//*******************************
			MatchLobbyMessage notification(COLOR_CHANGED_NOTIFICATION, DIRECTION_TO_CLIENT);
			notification.m_newColor = match_lobby_message->m_newColor;
			NotifyClients(playersMatch, &notification);

			break;
		}
		case CHANGE_MAP_REQUEST:
		{
			if( matchID == 0 )
			{
				SendError(socketFD, PROTOCOL_ERROR, DIRECTION_TO_SERVER);
				delete match_lobby_message;
				return IN_MAIN_LOBBY;
			}

			if( playersMatch == NULL)
			{
				//Error, there is no such Match
				SendError(socketFD, MATCH_DOESNT_EXIST, DIRECTION_TO_SERVER);
				delete match_lobby_message;
				return IN_MAIN_LOBBY;
			}
			//Is this this not the leader?
			if( playersMatch->GetLeaderID() != playerID)
			{
				//Error, not allowed
				SendError(socketFD, NOT_ALLOWED_TO_CHANGE_THAT, DIRECTION_TO_SERVER);
				delete match_lobby_message;
				return IN_MATCH_LOBBY;
			}

			playersMatch->SetMap(match_lobby_message->m_mapDescription);

			//*******************************
			// Send CHANGE MAP REPLY
			//*******************************
			MatchLobbyMessage change_map_reply(CHANGE_MAP_REPLY, DIRECTION_TO_SERVER);
			change_map_reply.m_changeAccepted = true;
			if(  Message::WriteMessage(&change_map_reply, socketFD) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
			}

			//*******************************
			// Send Client Notifications
			//*******************************
			MatchLobbyMessage notification(MAP_CHANGED_NOTIFICATION, DIRECTION_TO_CLIENT);
			notification.m_mapDescription = match_lobby_message->m_mapDescription;
			NotifyClients(playersMatch, &notification);
			break;
		}
		case CHANGE_VICTORY_COND_REQUEST:
		{
			if( matchID == 0 )
			{
				SendError(socketFD, PROTOCOL_ERROR, DIRECTION_TO_SERVER);
				delete match_lobby_message;
				return IN_MAIN_LOBBY;
			}

			if( playersMatch == NULL)
			{
				//Error, there is no such Match
				SendError(socketFD, MATCH_DOESNT_EXIST, DIRECTION_TO_SERVER);
				delete match_lobby_message;
				return IN_MAIN_LOBBY;
			}
			//Is this this not the leader?
			if( playersMatch->GetLeaderID() != playerID)
			{
				//Error, not allowed
				SendError(socketFD, NOT_ALLOWED_TO_CHANGE_THAT, DIRECTION_TO_SERVER);
				delete match_lobby_message;
				return IN_MATCH_LOBBY;
			}

			playersMatch->SetVictoryCondition(match_lobby_message->m_newVictCond);

			//*************************************
			// Send Change Victory Condition Reply
			//**************************************
			MatchLobbyMessage change_victory_reply(CHANGE_VICTORY_COND_REPLY, DIRECTION_TO_SERVER);
			change_victory_reply.m_changeAccepted = true;
			if( Message::WriteMessage(&change_victory_reply, socketFD) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
			}

			//*******************************
			// Send Client Notifications
			//*******************************
			MatchLobbyMessage notification(VICTORY_COND_CHANGED_NOTIFICATION, DIRECTION_TO_CLIENT);
			notification.m_newVictCond = match_lobby_message->m_newVictCond;
			NotifyClients(playersMatch, &notification);

			break;
		}
		case CHANGE_GAME_SPEED_REQUEST:
		{
			if( matchID == 0 )
			{
				SendError(socketFD, PROTOCOL_ERROR, DIRECTION_TO_SERVER);
				delete match_lobby_message;
				return IN_MAIN_LOBBY;
			}

			if( playersMatch == NULL)
			{
				//Error, there is no such Match
				SendError(socketFD, MATCH_DOESNT_EXIST, DIRECTION_TO_SERVER);
				delete match_lobby_message;
				return IN_MAIN_LOBBY;
			}
			//Is this this not the leader?
			if( playersMatch->GetLeaderID() != playerID)
			{
				//Error, not allowed
				SendError(socketFD, NOT_ALLOWED_TO_CHANGE_THAT, DIRECTION_TO_SERVER);
				delete match_lobby_message;
				return IN_MATCH_LOBBY;
			}

			playersMatch->SetGamespeed(match_lobby_message->m_newSpeed);

			//*******************************
			// Send Change Game Speed Reply
			//*******************************
			MatchLobbyMessage change_speed_reply(CHANGE_GAME_SPEED_REPLY, DIRECTION_TO_SERVER);
			change_speed_reply.m_changeAccepted = true;
			if(  Message::WriteMessage(&change_speed_reply, socketFD) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
			}

			//*******************************
			// Send Client Notifications
			//*******************************
			MatchLobbyMessage notification(GAME_SPEED_CHANGED_NOTIFICATION, DIRECTION_TO_CLIENT);
			notification.m_newSpeed = match_lobby_message->m_newSpeed;
			NotifyClients(playersMatch, &notification);
			break;
		}
		case CHANGE_LEADER_REQUEST:
		{
			if( matchID == 0 )
			{
				SendError(socketFD, PROTOCOL_ERROR, DIRECTION_TO_SERVER);
				delete match_lobby_message;
				return IN_MAIN_LOBBY;
			}

			if( playersMatch == NULL)
			{
				//Error, there is no such Match
				SendError(socketFD, MATCH_DOESNT_EXIST, DIRECTION_TO_SERVER);
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
			MatchLobbyMessage change_leader_reply(CHANGE_LEADER_REPLY, DIRECTION_TO_SERVER);
			change_leader_reply.m_changeAccepted = changed;
			if(  Message::WriteMessage(&change_leader_reply, socketFD) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
			}

			if( changed )
			{
				//*******************************
				// Send Client Notifications
				//*******************************
				MatchLobbyMessage notification(CHANGE_LEADER_NOTIFICATION, DIRECTION_TO_CLIENT);
				notification.m_playerID = match_lobby_message->m_playerID;
				NotifyClients(playersMatch, &notification);
			}

			return IN_MATCH_LOBBY;
		}
		case KICK_PLAYER_REQUEST:
		{
			if( matchID == 0 )
			{
				SendError(socketFD, PROTOCOL_ERROR, DIRECTION_TO_SERVER);
				delete match_lobby_message;
				return IN_MAIN_LOBBY;
			}

			if( playersMatch == NULL)
			{
				//Error, there is no such Match
				SendError(socketFD, MATCH_DOESNT_EXIST, DIRECTION_TO_SERVER);
				delete match_lobby_message;
				return IN_MAIN_LOBBY;
			}
			//Is this this not the leader?
			if( playersMatch->GetLeaderID() != playerID)
			{
				//Error, not allowed
				SendError(socketFD, NOT_ALLOWED_TO_CHANGE_THAT, DIRECTION_TO_SERVER);
				delete match_lobby_message;
				return IN_MATCH_LOBBY;
			}

			bool removed = playersMatch->RemovePlayer(match_lobby_message->m_playerID);

			//*******************************
			// Send Kick Player Reply
			//*******************************
			MatchLobbyMessage kick_player_reply(KICK_PLAYER_REPLY, DIRECTION_TO_SERVER);
			kick_player_reply.m_changeAccepted = removed;
			if(  Message::WriteMessage(&kick_player_reply, socketFD) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
			}

			if( removed )
			{
				//*******************************
				// Send Client Notifications
				//*******************************
				MatchLobbyMessage notification(PLAYER_LEFT_MATCH_NOTIFICATION, DIRECTION_TO_CLIENT);
				notification.m_playerID = match_lobby_message->m_playerID;
				notification.m_newLeaderID = playersMatch->GetLeaderID();
				NotifyClients(playersMatch, &notification);
			}

			break;
		}
		default:
		{
			SendError(socketFD, PROTOCOL_ERROR, DIRECTION_TO_SERVER);
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
enum LobbyReturn RTT::ProcessGameCommand(int socketFD, Player *player)
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

	if(!MessageManager::Instance().RegisterCallback(socketFD))
	{
		return EXITING_SERVER;
	}

	Message *message = Message::ReadMessage(socketFD, DIRECTION_TO_SERVER);
	if(message == NULL)
	{
		//ERROR
		cerr << "WARNING: Game message read failed\n";
		SendError(socketFD, PROTOCOL_ERROR, DIRECTION_TO_SERVER);
		return EXITING_SERVER;
	}
	GameMessage *game_message = (GameMessage*)message;
	switch(game_message->m_gameMessageType)
	{
		case MOVE_UNIT_DIRECTION_REQUEST:
		{
			//TODO: Check to see if the move is legal
			//TODO: Move the unit around in the gameboard

			GameMessage move_reply(MOVE_UNIT_DIRECTION_REPLY, DIRECTION_TO_SERVER);
			move_reply.m_moveResult = MOVE_SUCCESS;
			GameMessage::WriteMessage(&move_reply, socketFD);

			GameMessage move_notice(UNIT_MOVED_DIRECTION_NOTICE, DIRECTION_TO_CLIENT);
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
//	NOTE: Does not synchronize. You must have the lock from UseSocket() before calling this
void  RTT::SendError(int connectFD, enum ErrorType errorType, enum ProtocolDirection direction)
{
	ErrorMessage error_msg(errorType, direction);
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
			int recvSocket = (*it)->GetSocket();
			if( recvSocket >= 0 )
			{
				Lock lock = MessageManager::Instance().UseSocket(recvSocket);

				if( Message::WriteMessage(message, recvSocket) == false )
				{
					cerr << "ERROR: Message send returned failure.\n";
				}
				Message *message_ack = Message::ReadMessage(recvSocket, DIRECTION_TO_CLIENT);
				//TODO: Not strictly correct. We only want to allow ACKs
				if( message_ack->m_messageType != MESSAGE_MATCH_LOBBY)
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


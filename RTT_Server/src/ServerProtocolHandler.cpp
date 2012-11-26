//============================================================================
// Name        : ServerProtocolHandler.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Code for handling socket IO on the server side
//============================================================================

#include "RTT_Server.h"
#include "MatchLoop.h"
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
Player *RTT::GetNewClient(Ticket &ticket)
{
	//***************************
	// Read client Hello
	//***************************
	Message *client_hello_init = MessageManager::Instance().ReadMessage(ticket);
	if( client_hello_init->m_messageType != MESSAGE_AUTH)
	{
		SendError(ticket, PROTOCOL_ERROR);
		delete client_hello_init;
		return NULL;
	}

	AuthMessage *client_hello = (AuthMessage*)client_hello_init;
	if(client_hello->m_authType != CLIENT_HELLO)
	{
		cerr << "ERROR: Expected CLIENT_HELLO message, received: "
				<< client_hello->m_authType << "\n";
		SendError(ticket, PROTOCOL_ERROR);
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
		SendError(ticket, INCOMPATIBLE_SOFTWARE_VERSION);
		return NULL;
	}
	delete client_hello;


	//***************************
	// Send Server Hello
	//***************************
	AuthMessage server_hello(SERVER_HELLO);
	server_hello.m_softwareVersion.m_major = SERVER_VERSION_MAJOR;
	server_hello.m_softwareVersion.m_minor = SERVER_VERSION_MINOR;
	server_hello.m_softwareVersion.m_rev = SERVER_VERSION_REV;
	server_hello.m_authMechanism = HASHED_SALTED_PASS;

	if(MessageManager::Instance().WriteMessage(ticket, &server_hello) == false)
	{
		//Error in write
		return NULL;
	}

	//***************************
	// Receive Client Auth
	//***************************
	Message *client_auth_init = MessageManager::Instance().ReadMessage(ticket);
	if( client_auth_init->m_messageType != MESSAGE_AUTH)
	{
		//Error
		SendError(ticket, PROTOCOL_ERROR);
		delete client_auth_init;
		return NULL;
	}
	AuthMessage *client_auth = (AuthMessage*)client_auth_init;
	if(client_auth->m_authType != CLIENT_AUTH)
	{
		//Error
		SendError(ticket, PROTOCOL_ERROR);
		delete client_auth;
		return NULL;
	}

	enum AuthResult authresult =
			AuthenticateClient(client_auth->m_username, client_auth->m_hashedPassword);

	Player *player = NULL;
	if( authresult == AUTH_SUCCESS)
	{
		uint ID;
		{
			Lock lock(&playerIDLock, WRITE_LOCK);
			ID = ++lastPlayerID;
		}
		player = new Player(client_auth->m_username, ID);
		{
			Lock lock(&playerListLock, WRITE_LOCK);
			playerList[ID] = player;
		}
	}

	delete client_auth;


	//***************************
	// Send Server Auth Reply
	//***************************
	AuthMessage server_auth_reply(SERVER_AUTH_REPLY);
	server_auth_reply.m_authSuccess = authresult;
	if( player != NULL )
	{
		server_auth_reply.m_playerDescription = player->GetDescription();
	}
	else
	{
		return NULL;
	}

	if(MessageManager::Instance().WriteMessage(ticket, &server_auth_reply) == false)
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
enum LobbyReturn RTT::ProcessLobbyCommand(Ticket &ticket, Player *player)
{
	if( player == NULL )
	{
		return EXITING_SERVER;
	}

	//********************************
	// Receive Initial Lobby Message
	//********************************
	Message *lobby_message_init = MessageManager::Instance().ReadMessage(ticket);
	if( lobby_message_init->m_messageType != MESSAGE_LOBBY )
	{
		//ERROR
		SendError(ticket, PROTOCOL_ERROR);
		return IN_MAIN_LOBBY;
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
			LobbyMessage query_reply(MATCH_LIST_REPLY);
			query_reply.m_returnedMatchesCount = matchCount;
			query_reply.m_matchDescriptions = (MatchDescription*)
					malloc(sizeof(struct MatchDescription) * matchCount);
			for(uint i = 0; i < matchCount; i++ )
			{
				query_reply.m_matchDescriptions[i] = matches[i];
			}
			if( MessageManager::Instance().WriteMessage(ticket, &query_reply) == false)
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
			LobbyMessage options_available(MATCH_CREATE_OPTIONS_AVAILABLE);
			options_available.m_options.m_maxPlayers = MAX_PLAYERS_IN_MATCH;
			if(MessageManager::Instance().WriteMessage(ticket, &options_available) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
				delete lobby_message;
				return IN_MAIN_LOBBY;
			}

			//********************************
			// Receive Options Chosen
			//********************************
			Message *options_chosen_init = MessageManager::Instance().ReadMessage(ticket);
			if( options_chosen_init->m_messageType !=  MESSAGE_LOBBY)
			{
				//Error
				cerr << "ERROR: Client gave us the wrong message type.\n";
				SendError(ticket, PROTOCOL_ERROR);
				delete options_chosen_init;
				delete lobby_message;
				return IN_MAIN_LOBBY;
			}

			LobbyMessage *options_chosen = (LobbyMessage*)options_chosen_init;
			if(options_chosen->m_lobbyType != MATCH_CREATE_OPTIONS_CHOSEN)
			{
				//Error
				cerr << "ERROR: Client gave us the wrong message type.\n";
				SendError(ticket, PROTOCOL_ERROR);
				delete options_chosen;
				delete lobby_message;
				return IN_MAIN_LOBBY;
			}

			if((options_chosen->m_options.m_maxPlayers > MAX_PLAYERS_IN_MATCH) ||
					(options_chosen->m_options.m_maxPlayers < 2))
			{
				cerr << "ERROR: Client asked an invalid number of max playersb.\n";
				SendError(ticket, INVALID_MAX_PLAYERS);
				delete lobby_message;
				return IN_MAIN_LOBBY;
			}

			uint matchID = RegisterNewMatch(player, options_chosen->m_options);

			if( matchID == 0 )
			{
				//TODO: Find a better error message
				SendError(ticket, TOO_BUSY);
				delete lobby_message;
				return IN_MAIN_LOBBY;
			}
			delete options_chosen;

			//***************************
			// Send Match Create Reply
			//***************************
			LobbyMessage create_reply(MATCH_CREATE_REPLY);
			Match *joinedMatch;
			{
				Lock lock(&matchListLock, READ_LOCK);
				joinedMatch = matchList[matchID];
				create_reply.m_matchDescription = joinedMatch->GetDescription();
			}
			if( MessageManager::Instance().WriteMessage(ticket, &create_reply) == false)
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
					LobbyMessage match_join(MATCH_JOIN_REPLY);
					Match *joinedMatch;
					{
						Lock lock(&matchListLock, READ_LOCK);
						joinedMatch = matchList[lobby_message->m_ID];
					}
					match_join.m_matchDescription = joinedMatch->GetDescription();

					//Put in the player descriptions of current members
					match_join.m_playerDescriptions = (struct PlayerDescription*)
						malloc(sizeof(struct PlayerDescription) * MAX_PLAYERS_IN_MATCH);

					uint count = GetPlayerDescriptions(joinedMatch->GetID(),
							match_join.m_playerDescriptions);
					match_join.m_returnedPlayersCount = count;



					if( MessageManager::Instance().WriteMessage(ticket, &match_join) == false)
					{
						//Error in write, do something?
						cerr << "ERROR: Message send returned failure.\n";
					}

					//*******************************
					// Send Client Notifications
					//*******************************
					MatchLobbyMessage notification(PLAYER_JOINED_MATCH_NOTIFICATION);
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
				SendError(ticket, errorType);
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
			LobbyMessage stats_reply(SERVER_STATS_REPLY);

			{
				Lock lock(&matchListLock, READ_LOCK);
				stats_reply.m_serverStats.m_numMatches = matchList.size();
			}
			{
				Lock lock(&playerListLock, READ_LOCK);
				stats_reply.m_serverStats.m_numPlayers = playerList.size();
			}
			if( MessageManager::Instance().WriteMessage(ticket, &stats_reply) == false )
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
			LobbyMessage exit_server(MATCH_EXIT_SERVER_ACKNOWLEDGE);
			if( MessageManager::Instance().WriteMessage(ticket, &exit_server) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
			}
			delete lobby_message;
			return EXITING_SERVER;
		}
		default:
		{
			SendError(ticket, PROTOCOL_ERROR);
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
	Lock lock(&playerListLock, READ_LOCK);
	//Check if the username exists in the active list
	PlayerList::iterator it = playerList.begin();
	for(; it != playerList.end(); ++it)
	{
		if( it->second->GetName().compare(username) == 0)
		{
			return USERNAME_ALREADY_EXISTS;
		}
	}

	//TODO: Check if the username exists in the non-active list (from file on disk)
	return AUTH_SUCCESS;
}

//Processes one MatchLobby command
//	Starts out by listening on the given socket for a MatchLobbyMessage
//	Executes the MatchLobby protocol
//	Returns a enum LobbyReturn to describe the end state
enum LobbyReturn RTT::ProcessMatchLobbyCommand(Ticket &ticket, Player *player)
{
	if( player == NULL )
	{
		cerr << "ERROR: Tried to process MatchLobby command for NULL player\n";
		return EXITING_SERVER;
	}

	uint matchID = player->GetCurrentMatchID();
	uint playerID = player->GetID();

	Match *playersMatch = NULL;
	{
		Lock lock(&matchListLock, READ_LOCK);
		if( matchList.count(matchID) != 0)
		{
			playersMatch = matchList[matchID];
		}
	}

	//********************************
	// Receive Initial MatchLobby Message
	//********************************
	Message *match_lobby_message_init = MessageManager::Instance().ReadMessage(ticket);
	if(match_lobby_message_init->m_messageType != MESSAGE_MATCH_LOBBY )
	{
		//ERROR
		if(match_lobby_message_init->m_messageType == MESSAGE_ERROR)
		{
			ErrorMessage *error = (ErrorMessage*)match_lobby_message_init;
			if(error->m_errorType == ERROR_SOCKET_CLOSED)
			{
				return EXITING_SERVER;
			}
		}
		cerr << "ERROR: Lobby message came back NULL\n";
		SendError(ticket, PROTOCOL_ERROR);
		return IN_MATCH_LOBBY;
	}

	enum LobbyReturn ret = IN_MAIN_LOBBY;

	MatchLobbyMessage *match_lobby_message = (MatchLobbyMessage*)match_lobby_message_init;
	switch (match_lobby_message->m_matchLobbyType)
	{
		case MATCH_LEAVE_NOTIFICATION:
		{
			if( matchID == 0 )
			{
				SendError(ticket, NOT_IN_THAT_MATCH);
				ret = IN_MAIN_LOBBY;
				break;
			}
			if(LeaveMatch(player))
			{
				//*******************************
				// Send Match Leave Acknowledge
				//*******************************
				MatchLobbyMessage leave_ack(MATCH_LEAVE_ACKNOWLEDGE);
				if(MessageManager::Instance().WriteMessage(ticket, &leave_ack) == false)
				{
					//Error in write, do something?
					cerr << "ERROR: Message send returned failure.\n";
				}
				ret = IN_MAIN_LOBBY;
				break;
			}
			else
			{
				SendError(ticket, NOT_IN_THAT_MATCH);
				delete match_lobby_message;
				if( matchID == 0)
				{
					ret = IN_MAIN_LOBBY;
					break;
				}
				ret = IN_MATCH_LOBBY;
				break;
			}
		}
		case CHANGE_TEAM_REQUEST:
		{
			if( matchID == 0 )
			{
				SendError(ticket, PROTOCOL_ERROR);
				ret = IN_MAIN_LOBBY;
				break;
			}

			if( playersMatch == NULL)
			{
				//Error, there is no such Match
				SendError(ticket, MATCH_DOESNT_EXIST);
				ret = IN_MAIN_LOBBY;
				break;
			}

			//Is this not a self change?
			if( playerID != match_lobby_message->m_playerID )
			{
				//Is this this not the leader?
				if( playersMatch->GetLeaderID() != playerID)
				{
					//Error, there is no such Match
					SendError(ticket, NOT_ALLOWED_TO_CHANGE_THAT);
					ret = IN_MATCH_LOBBY;
					break;
				}
			}

			Player *changingPlayer = NULL;
			bool changed = false;

			{
				Lock lock(&playerListLock, WRITE_LOCK);
				if( playerList.count(match_lobby_message->m_playerID) > 0 )
				{
					changingPlayer = playerList[match_lobby_message->m_playerID];
				}
			}
			if(changingPlayer != NULL)
			{
				changed = playersMatch->ChangeTeam(changingPlayer, match_lobby_message->m_newTeam);
			}

			//*******************************
			// Send CHANGE TEAM REPLY
			//*******************************
			MatchLobbyMessage change_team_reply(CHANGE_TEAM_REPLY);
			change_team_reply.m_changeAccepted = changed;
			if(MessageManager::Instance().WriteMessage(ticket, &change_team_reply) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
			}

			if( changed )
			{
				//*******************************
				// Send Client Notifications
				//*******************************
				MatchLobbyMessage notification(TEAM_CHANGED_NOTIFICATION);
				notification.m_newTeam = match_lobby_message->m_newTeam;
				notification.m_playerID = match_lobby_message->m_playerID;
				NotifyClients(playersMatch, &notification);
			}

			ret = IN_MATCH_LOBBY;
			break;
		}
		case START_MATCH_REQUEST:
		{
			if( matchID == 0 )
			{
				SendError(ticket, PROTOCOL_ERROR);
				ret = IN_MAIN_LOBBY;
				break;
			}

			if(playersMatch == NULL)
			{
				//Error, there is no such Match
				SendError(ticket, MATCH_DOESNT_EXIST);
				ret = IN_MAIN_LOBBY;
				break;
			}
			//Is this this not the leader?
			if( playersMatch->GetLeaderID() != playerID)
			{
				//Error, there is no such Match
				SendError(ticket, NOT_ALLOWED_TO_CHANGE_THAT);
				ret = IN_MATCH_LOBBY;
				break;
			}

			bool started = playersMatch->StartMatch();

			//*******************************
			// Send START MATCH REPLY
			//*******************************
			MatchLobbyMessage start_match_reply(START_MATCH_REPLY);
			start_match_reply.m_changeAccepted = started;
			if(  MessageManager::Instance().WriteMessage(ticket, &start_match_reply) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
			}

			if(started)
			{
				//*******************************
				// Send Client Notifications
				//*******************************
				//TODO: Make sure each client is ready. IE: Listed for replies
				MatchLobbyMessage notification(MATCH_START_NOTIFICATION);
				NotifyClients(playersMatch, &notification);

				MatchLoop(playersMatch);
			}
			else
			{

			}

			ret = IN_GAME;
			break;
			}
		case CHANGE_COLOR_REQUEST:
		{
			if( matchID == 0 )
			{
				SendError(ticket, PROTOCOL_ERROR);
				ret = IN_MAIN_LOBBY;
				break;
			}

			if( playersMatch == NULL)
			{
				//Error, there is no such Match
				SendError(ticket, MATCH_DOESNT_EXIST);
				ret = IN_MAIN_LOBBY;
				break;
			}
			//Is this not a self change?
			if( playerID != match_lobby_message->m_playerID )
			{
				//Is this this not the leader?
				if(playersMatch->GetLeaderID() != playerID)
				{
					//Error, not allowed
					SendError(ticket, NOT_ALLOWED_TO_CHANGE_THAT);
					ret = IN_MATCH_LOBBY;
					break;
				}
			}

			{
				Lock lock(&playerListLock, READ_LOCK);
				if( playerList.count(match_lobby_message->m_playerID) != 0 )
				{
					playerList[match_lobby_message->m_playerID]->SetColor(
							match_lobby_message->m_newColor);
				}
				else
				{
					//Error, there is no such player
					SendError(ticket, NO_SUCH_PLAYER);
					ret = IN_MATCH_LOBBY;
					break;
				}
			}

			//*******************************
			// Send CHANGE COLOR REPLY
			//*******************************
			MatchLobbyMessage change_color_reply(CHANGE_COLOR_REPLY);
			change_color_reply.m_changeAccepted = true;
			if(MessageManager::Instance().WriteMessage(ticket, &change_color_reply) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
			}

			//*******************************
			// Send Client Notifications
			//*******************************
			MatchLobbyMessage notification(COLOR_CHANGED_NOTIFICATION);
			notification.m_newColor = match_lobby_message->m_newColor;
			NotifyClients(playersMatch, &notification);

			ret = IN_MATCH_LOBBY;
			break;
		}
		case CHANGE_MAP_REQUEST:
		{
			if( matchID == 0 )
			{
				SendError(ticket, PROTOCOL_ERROR);
				ret = IN_MAIN_LOBBY;
				break;
			}

			if( playersMatch == NULL)
			{
				//Error, there is no such Match
				SendError(ticket, MATCH_DOESNT_EXIST);
				ret = IN_MAIN_LOBBY;
				break;
			}
			//Is this this not the leader?
			if( playersMatch->GetLeaderID() != playerID)
			{
				//Error, not allowed
				SendError(ticket, NOT_ALLOWED_TO_CHANGE_THAT);
				ret = IN_MAIN_LOBBY;
				break;
			}

			playersMatch->SetMap(match_lobby_message->m_mapDescription);

			//*******************************
			// Send CHANGE MAP REPLY
			//*******************************
			MatchLobbyMessage change_map_reply(CHANGE_MAP_REPLY);
			change_map_reply.m_changeAccepted = true;
			if(MessageManager::Instance().WriteMessage(ticket, &change_map_reply) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
			}

			//*******************************
			// Send Client Notifications
			//*******************************
			MatchLobbyMessage notification(MAP_CHANGED_NOTIFICATION);
			notification.m_mapDescription = match_lobby_message->m_mapDescription;
			NotifyClients(playersMatch, &notification);

			ret = IN_MATCH_LOBBY;
			break;
		}
		case CHANGE_VICTORY_COND_REQUEST:
		{
			if( matchID == 0 )
			{
				SendError(ticket, PROTOCOL_ERROR);
				ret = IN_MAIN_LOBBY;
				break;
			}

			if( playersMatch == NULL)
			{
				//Error, there is no such Match
				SendError(ticket, MATCH_DOESNT_EXIST);
				ret = IN_MAIN_LOBBY;
				break;
			}
			//Is this this not the leader?
			if( playersMatch->GetLeaderID() != playerID)
			{
				//Error, not allowed
				SendError(ticket, NOT_ALLOWED_TO_CHANGE_THAT);
				ret = IN_MATCH_LOBBY;
				break;
			}

			playersMatch->SetVictoryCondition(match_lobby_message->m_newVictCond);

			//*************************************
			// Send Change Victory Condition Reply
			//**************************************
			MatchLobbyMessage change_victory_reply(CHANGE_VICTORY_COND_REPLY);
			change_victory_reply.m_changeAccepted = true;
			if( MessageManager::Instance().WriteMessage(ticket, &change_victory_reply) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
			}

			//*******************************
			// Send Client Notifications
			//*******************************
			MatchLobbyMessage notification(VICTORY_COND_CHANGED_NOTIFICATION);
			notification.m_newVictCond = match_lobby_message->m_newVictCond;
			NotifyClients(playersMatch, &notification);

			ret = IN_MATCH_LOBBY;
			break;
		}
		case CHANGE_GAME_SPEED_REQUEST:
		{
			if( matchID == 0 )
			{
				SendError(ticket, PROTOCOL_ERROR);
				ret = IN_MAIN_LOBBY;
				break;
			}

			if( playersMatch == NULL)
			{
				//Error, there is no such Match
				SendError(ticket, MATCH_DOESNT_EXIST);
				ret = IN_MAIN_LOBBY;
				break;
			}
			//Is this this not the leader?
			if( playersMatch->GetLeaderID() != playerID)
			{
				//Error, not allowed
				SendError(ticket, NOT_ALLOWED_TO_CHANGE_THAT);
				ret = IN_MATCH_LOBBY;
				break;
			}

			playersMatch->SetGamespeed(match_lobby_message->m_newSpeed);

			//*******************************
			// Send Change Game Speed Reply
			//*******************************
			MatchLobbyMessage change_speed_reply(CHANGE_GAME_SPEED_REPLY);
			change_speed_reply.m_changeAccepted = true;
			if(MessageManager::Instance().WriteMessage(ticket, &change_speed_reply) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
			}

			//*******************************
			// Send Client Notifications
			//*******************************
			MatchLobbyMessage notification(GAME_SPEED_CHANGED_NOTIFICATION);
			notification.m_newSpeed = match_lobby_message->m_newSpeed;
			NotifyClients(playersMatch, &notification);

			ret = IN_MATCH_LOBBY;
			break;
		}
		case CHANGE_LEADER_REQUEST:
		{
			if( matchID == 0 )
			{
				SendError(ticket, PROTOCOL_ERROR);
				ret = IN_MAIN_LOBBY;
				break;
			}

			if( playersMatch == NULL)
			{
				//Error, there is no such Match
				SendError(ticket, MATCH_DOESNT_EXIST);
				ret = IN_MAIN_LOBBY;
				break;
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
			MatchLobbyMessage change_leader_reply(CHANGE_LEADER_REPLY);
			change_leader_reply.m_changeAccepted = changed;
			if(  MessageManager::Instance().WriteMessage(ticket, &change_leader_reply) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
			}

			if( changed )
			{
				//*******************************
				// Send Client Notifications
				//*******************************
				MatchLobbyMessage notification(CHANGE_LEADER_NOTIFICATION);
				notification.m_playerID = match_lobby_message->m_playerID;
				NotifyClients(playersMatch, &notification);
			}

			ret = IN_MATCH_LOBBY;
			break;
		}
		case KICK_PLAYER_REQUEST:
		{
			if( matchID == 0 )
			{
				SendError(ticket, PROTOCOL_ERROR);
				ret = IN_MAIN_LOBBY;
				break;
			}

			if( playersMatch == NULL)
			{
				//Error, there is no such Match
				SendError(ticket, MATCH_DOESNT_EXIST);
				ret = IN_MAIN_LOBBY;
				break;
			}
			//Is this this not the leader?
			if( playersMatch->GetLeaderID() != playerID)
			{
				//Error, not allowed
				SendError(ticket, NOT_ALLOWED_TO_CHANGE_THAT);
				ret = IN_MATCH_LOBBY;
				break;
			}

			bool removed = playersMatch->RemovePlayer(match_lobby_message->m_playerID);

			//*******************************
			// Send Kick Player Reply
			//*******************************
			MatchLobbyMessage kick_player_reply(KICK_PLAYER_REPLY);
			kick_player_reply.m_changeAccepted = removed;
			if(MessageManager::Instance().WriteMessage(ticket, &kick_player_reply) == false)
			{
				//Error in write, do something?
				cerr << "ERROR: Message send returned failure.\n";
			}

			if( removed )
			{
				//*******************************
				// Send Client Notifications
				//*******************************
				MatchLobbyMessage notification(PLAYER_LEFT_MATCH_NOTIFICATION);
				notification.m_playerID = match_lobby_message->m_playerID;
				notification.m_newLeaderID = playersMatch->GetLeaderID();
				NotifyClients(playersMatch, &notification);
			}

			ret = IN_MATCH_LOBBY;
			break;
		}
		default:
		{
			SendError(ticket, PROTOCOL_ERROR);
			ret = IN_MATCH_LOBBY;
			break;
		}
	}
	delete match_lobby_message;
	return ret;
}

//Processes one game command
//	Starts out by listening on the given socket for a GameMessage
//	Executes the game protocol
//	Returns a enum LobbyReturn to describe the end state
enum LobbyReturn RTT::ProcessGameCommand(Ticket &ticket, Player *player)
{
	if(player == NULL)
	{
		cerr << "ERROR: Player in game does not exist\n";
		return EXITING_SERVER;
	}

	uint matchID = player->GetCurrentMatchID();

	Match *playersMatch = NULL;
	{
		Lock lock(&matchListLock, READ_LOCK);
		if( matchList.count(matchID) != 0)
		{
			playersMatch = matchList[matchID];
		}
	}

	Message *message = MessageManager::Instance().ReadMessage(ticket);
	if(message->m_messageType != MESSAGE_GAME)
	{
		//ERROR
		cerr << "WARNING: Game message read failed\n";
		SendError(ticket, PROTOCOL_ERROR);
		return EXITING_SERVER;
	}

	enum LobbyReturn ret = IN_MATCH_LOBBY;

	GameMessage *game_message = (GameMessage*)message;
	switch(game_message->m_gameMessageType)
	{
		case MOVE_UNIT_DIRECTION_REQUEST:
		{
			//TODO: Check to see if the move is legal
			//TODO: Move the unit around in the gameboard

			GameMessage move_reply(MOVE_UNIT_DIRECTION_REPLY);
			move_reply.m_moveResult = MOVE_SUCCESS;
			MessageManager::Instance().WriteMessage(ticket, &move_reply);

			GameMessage move_notice(UNIT_MOVED_DIRECTION_NOTICE);
			move_notice.m_unitID = game_message->m_unitID;
			move_notice.m_xOld = game_message->m_xOld;
			move_notice.m_yOld = game_message->m_yOld;

			NotifyClients(playersMatch, &move_notice);

			ret = IN_GAME;
			break;
		}
		default:
		{
			ret = IN_GAME;
			break;
		}
	}

	return ret;
}

//Send a message of type Error to the client
//	NOTE: Does not synchronize. You must have the lock from UseSocket() before calling this
void RTT::SendError(const Ticket &ticket, enum ErrorType errorType)
{
	ErrorMessage error_msg(errorType);
	if(MessageManager::Instance().WriteMessage(ticket, &error_msg) == false)
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
				Ticket ticket = MessageManager::Instance().StartConversation(recvSocket);

				if(MessageManager::Instance().WriteMessage(ticket, message) == false)
				{
					cerr << "ERROR: Message send returned failure.\n";
				}
				Message *message_ack = MessageManager::Instance().ReadMessage(ticket);
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


//============================================================================
// Name        : LoF_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Code for handling socket IO on the server side
//============================================================================

#include "LoF_Server.h"
#include "ServerProtocolHandler.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>

using namespace std;
using namespace LoF;

extern PlayerList playerList;
extern MatchList matchList;

extern uint lastMatchID;

//Negotiates the hello messages and authentication to a new client
//	Returns a new Player object, NULL on error
Player *LoF::GetNewClient(int ConnectFD)
{
	//***************************
	// Read client Hello
	//***************************

	AuthMessage *client_hello =(AuthMessage*) Message::ReadMessage(ConnectFD);

	if( client_hello == NULL )
	{
		return NULL;
	}

	if( client_hello->type != CLIENT_HELLO )
	{
		cerr << "ERROR: Expected CLIENT_HELLO message, received: "
				<< client_hello->type << "\n";
		delete client_hello;
		return NULL;
	}

	//Check version compatibility
	if ((client_hello->softwareVersion.major != SERVER_VERSION_MAJOR) ||
		(client_hello->softwareVersion.minor != SERVER_VERSION_MINOR) ||
		(client_hello->softwareVersion.rev != SERVER_VERSION_REV) )
	{
		//If versions are not the same, send an error message to the client
		delete client_hello;
		cout << "Client Connected with bad software version.\n";

		//*********************************
		// Send Server Auth Reply (Error)
		//*********************************
		AuthMessage *server_auth_reply = new AuthMessage();
		server_auth_reply->type = SERVER_AUTH_REPLY;
		server_auth_reply->authSuccess = INCOMPATIBLE_SOFTWARE_VERSIONS;

		if(  Message::WriteMessage(server_auth_reply, ConnectFD) == false)
		{
			//Error in write
			cerr << "Wasn't able to send AUTH_REPLY to client to tell it about "
					"the bad incompatible software versions.\n";
		}
		delete server_auth_reply;
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

	AuthMessage *client_auth = (AuthMessage*)Message::ReadMessage(ConnectFD);
	if( client_auth == NULL )
	{
		//ERROR
		return NULL;
	}
	if( client_auth->type !=  CLIENT_AUTH)
	{
		//Error
		delete client_auth;
		return NULL;
	}

	enum AuthResult authresult =
			AuthenticateClient(client_auth->username, client_auth->hashedPassword);

	Player *player = NULL;
	if( authresult == AUTH_SUCCESS)
	{
		player = new Player(client_auth->username);
		playerList[client_auth->username] = player;
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
enum LobbyReturn LoF::ProcessLobbyCommand(int ConnectFD, Player *player)
{
	//********************************
	// Receive Initial Lobby Message
	//********************************
	LobbyMessage *lobby_message = (LobbyMessage*)Message::ReadMessage(ConnectFD);
	if( lobby_message == NULL )
	{
		//ERROR
		cerr << "ERROR: Lobby message came back NULL\n";
		return STILL_IN_LOBBY;
	}

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

			return STILL_IN_LOBBY;
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
				return STILL_IN_LOBBY;
			}
			delete options_available;

			//********************************
			// Receive Options Chosen
			//********************************
			LobbyMessage *options_chosen = (LobbyMessage*)Message::ReadMessage(ConnectFD);
			if( options_chosen == NULL )
			{
				//Error
				cerr << "ERROR: Reading from client failed.\n";
				return STILL_IN_LOBBY;
			}
			if( options_chosen->type !=  MATCH_CREATE_OPTIONS_CHOSEN)
			{
				//Error
				cerr << "ERROR: Client gave us the wrong message type.\n";
				delete options_chosen;
				return STILL_IN_LOBBY;
			}

			if(options_chosen->options.maxPlayers > MAX_PLAYERS_IN_MATCH)
			{
				cerr << "ERROR: Client asked for more players in a match than allowed.\n";
				SendError(ConnectFD, INVALID_MAX_PLAYERS);
				return STILL_IN_LOBBY;
			}

			uint matchID = RegisterNewMatch(player);

			if( matchID == 0 )
			{
				//TODO: Find a better error message hereb
				SendError(ConnectFD, TOO_BUSY);
				return STILL_IN_LOBBY;
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
				return STILL_IN_LOBBY;
			}
			delete create_reply;

			return STARTING_MATCH;
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
					match_join->matchDescription = matchList[match_join->ID]->description;
					if(  Message::WriteMessage(match_join, ConnectFD) == false)
					{
						//Error in write, do something?
						cerr << "ERROR: Message send returned failure.\n";
					}
					delete match_join;
					return STARTING_MATCH;
				}
				case LOBBY_MATCH_IS_FULL:
				{
					errorType = MATCH_IS_FULL;
				}
				case LOBBY_MATCH_DOESNT_EXIST:
				{
					errorType = MATCH_DOESNT_EXIST;
				}
				case LOBBY_NOT_ALLOWED_IN:
				{
					errorType = NOT_ALLOWED_IN;
				}
				case LOBBY_ALREADY_IN_MATCH:
				{
					errorType = ALREADY_IN_MATCH;
				}
				default:
				{
					errorType = PROTOCOL_ERROR;
				}
				SendError(ConnectFD, errorType);
				return STILL_IN_LOBBY;
			}
		}
		case MATCH_LEAVE_NOTIFICATION:
		{
			if( LeaveMatch(player, lobby_message->ID) )
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
				return STILL_IN_LOBBY;
			}
			else
			{
				SendError(ConnectFD, NOT_IN_THAT_MATCH);
				return STILL_IN_LOBBY;
			}
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
		}
	}
	delete lobby_message;
}

//Authenticates the given username/password with the server
//Checks that:
//	a) The username exists in the system
//	b) The given password hash is correct for the specified username
//	c) The username is unique on the server
enum AuthResult LoF::AuthenticateClient(char *username, unsigned char *hashedPassword)
{
	//TODO: Authenticate!

	//Check if the username exists in the active list
	if( playerList[username] != NULL)
	{
		return USERNAME_ALREADY_EXISTS;
	}

	//TODO: Check if the username exists in the non-active list (from file on disk)

	return AUTH_SUCCESS;
}

//Send a message of type Error to the client
void  LoF::SendError(int connectFD, enum ErrorType errorType)
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

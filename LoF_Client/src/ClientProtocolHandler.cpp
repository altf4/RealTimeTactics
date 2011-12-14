//============================================================================
// Name        : LoF_Client.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Code for handling socket IO on the server side
//============================================================================

#include "LoF_Client.h"
#include "ClientProtocolHandler.h"
#include "messages/AuthMessage.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <string.h>

using namespace std;
using namespace LoF;

bool LoF::AuthToServer(int connectFD, string username, unsigned char *hashedPassword)
{
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
		return false;
	}
	delete client_hello;

	//***************************
	// Receive Server Hello
	//***************************

	AuthMessage *server_hello = (AuthMessage*)Message::ReadMessage(connectFD);
	if( server_hello == NULL)
	{
		return false;
	}
	if( server_hello->type == SERVER_AUTH_REPLY)
	{
		if(server_hello->authSuccess != AUTH_SUCCESS)
		{
			if( server_hello->authSuccess == INCOMPATIBLE_SOFTWARE_VERSIONS )
			{
				//Rejected by server
				cout << "Rejected by server, incompatible software versions.\n";
			}
			else
			{
				cout << "Rejected by server.\n";
			}
		}
		else
		{
			//ERROR: Shouldn't get here. So assume some kind of error happened
			cerr << "ERROR: Protocol error. Server replied with wrong message...\n";
		}
		delete server_hello;
		return false;
	}
	if( server_hello->type != SERVER_HELLO)
	{
		delete server_hello;
		return false;
	}

	//Check version compatibility
	if ((server_hello->softwareVersion.major != CLIENT_VERSION_MAJOR) ||
		(server_hello->softwareVersion.minor != CLIENT_VERSION_MINOR) ||
		(server_hello->softwareVersion.rev != CLIENT_VERSION_REV) )
	{
		//Incompatible software versions.
		//The server should have caught this, though.

		delete server_hello;
		return false;
	}
	delete server_hello;


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
		return false;
	}
	delete client_auth;

	//***************************
	// Receive Server Auth Reply
	//***************************

	AuthMessage *server_auth_reply = (AuthMessage*)Message::ReadMessage(connectFD);
	if( server_auth_reply == NULL)
	{
		return false;
	}
	if( server_auth_reply->type != SERVER_AUTH_REPLY)
	{
		delete server_auth_reply;
		return false;
	}

	return true;
}

//Informs the server that we want to exit
//	connectFD: Socket File descriptor of the server
//	Returns true if we get a successful acknowledgment back
bool LoF::ExitServer(int connectFD)
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
	LobbyMessage *exit_server_ack = (LobbyMessage*)Message::ReadMessage(connectFD);
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
uint LoF::ListMatches(int connectFD, uint page, MatchDescription *matchArray)
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
	LobbyMessage *list_reply = (LobbyMessage*)Message::ReadMessage(connectFD);
	if( list_reply == NULL)
	{
		return 0;
	}
	if( list_reply->type != MATCH_LIST_REPLY)
	{
		delete list_reply;
		return 0;
	}
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
bool LoF::CreateMatch(int connectFD, struct MatchOptions options)
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
	LobbyMessage *ops_available = (LobbyMessage*)Message::ReadMessage(connectFD);
	if( ops_available == NULL)
	{
		return false;
	}
	if( ops_available->type != MATCH_CREATE_OPTIONS_AVAILABLE)
	{
		delete ops_available;
		return false;
	}

	if( ops_available->options.maxPlayers != options.maxPlayers )
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
	LobbyMessage *create_reply = (LobbyMessage*)Message::ReadMessage(connectFD);
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
bool LoF::JoinMatch(int connectFD, uint matchID)
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
	LobbyMessage *join_reply = (LobbyMessage*)Message::ReadMessage(connectFD);
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
bool LoF::LeaveMatch(int connectFD, uint matchID)
{
	//********************************
	// Send Match Leave Notification
	//********************************
	LobbyMessage *leave_note = new LobbyMessage();
	leave_note->type = MATCH_LEAVE_NOTIFICATION;
	leave_note->ID = matchID;
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
	LobbyMessage *leave_ack = (LobbyMessage*)Message::ReadMessage(connectFD);
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


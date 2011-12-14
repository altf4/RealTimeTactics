//============================================================================
// Name        : LoF_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Code for handling socket IO on the server side
//============================================================================

#ifndef PROTOCOLHANDLER_H_
#define PROTOCOLHANDLER_H_

#include <vector>
#include "messages/AuthMessage.h"

using namespace std;

namespace LoF
{

enum LobbyReturn
{
	STARTING_MATCH,
	STILL_IN_LOBBY,
	EXITING_SERVER,
};

//Negotiates the hello messages and authentication to a new client
//	Returns a new Player object, NULL on error
Player *GetNewClient(int connectFD);

//Authenticates the given username/password with the server
//Checks that:
//	a) The username exists in the system
//	b) The given password hash is correct for the specified username
//	c) The username is unique on the server
enum AuthResult AuthenticateClient(char *username, unsigned char *hashedPassword);

//Processes one Lobby command
//	Starts out by listening on the given socket for a LobbyMessage
//	Executes the Lobby protocol
//	Returns a enum LobbyReturn to describe the end state
enum LobbyReturn ProcessLobbyCommand(int connectFD, Player *player);

}
#endif /* PROTOCOLHANDLER_H_ */

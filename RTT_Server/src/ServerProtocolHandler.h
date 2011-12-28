//============================================================================
// Name        : ServerProtocolHandler.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Code for handling socket IO on the server side
//============================================================================

#ifndef PROTOCOLHANDLER_H_
#define PROTOCOLHANDLER_H_

#include <vector>
#include "messages/AuthMessage.h"
#include "messages/ErrorMessage.h"
#include "messages/MatchLobbyMessage.h"

#define CALLBACK_WAIT_TIME 10

using namespace std;

namespace RTT
{

enum LobbyReturn
{
	IN_MATCH_LOBBY,
	IN_MAIN_LOBBY,
	EXITING_SERVER,
	IN_GAME,
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

//Processes one MatchLobby command
//	Starts out by listening on the given socket for a MatchLobbyMessage
//	Executes the MatchLobby protocol
//	Returns a enum LobbyReturn to describe the end state
enum LobbyReturn ProcessMatchLobbyCommand(int connectFD, Player *player);

//Establishes the player's receive socket
//	Sets the player's receiveSocket
//	connectFD: The old socket, used to tell the client we're ready
//	returns the created socket
int MatchLobbyConnectBack(int connectFD, uint portNum, Player *player);

//Sends out the given MatchLobbyMessage to all clients in the given match
bool NotifyClients(Match *match, MatchLobbyMessage *message);

//Send a message of type Error to the client
void SendError(int connectFD, enum ErrorType errorType);

}
#endif /* PROTOCOLHANDLER_H_ */

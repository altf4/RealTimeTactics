//============================================================================
// Name        : Unit.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Message class which is passed to/from client/server
//============================================================================

#ifndef MESSAGE_H_
#define MESSAGE_H_

#define MESSAGE_MIN_SIZE	sizeof(enum MessageType)
#define USERNAME_MAX_LENGTH	20

#include <stdlib.h>
#include <vector>
#include <unistd.h>

using namespace std;

namespace RTT
{

enum MessageType: char
{
	//****************
	//	AuthMessage
	//****************
	//Messages for first contacting the server and authenticating
	CLIENT_HELLO = 0,
	SERVER_HELLO,
	CLIENT_AUTH,
	SERVER_AUTH_REPLY,

	//****************
	//	LobbyMessage
	//****************
	//Querying matches
	MATCH_LIST_REQUEST,
	MATCH_LIST_REPLY,
	//Creating a new match
	MATCH_CREATE_REQUEST,
	MATCH_CREATE_OPTIONS_AVAILABLE,
	MATCH_CREATE_OPTIONS_CHOSEN,
	MATCH_CREATE_REPLY,
	//Joining a match already created
	MATCH_JOIN_REQUEST,
	MATCH_JOIN_REPLY,
	//Deleting a match you're in
	MATCH_LEAVE_NOTIFICATION,
	MATCH_LEAVE_ACKNOWLEDGE,
	//Exit the server
	MATCH_EXIT_SERVER_NOTIFICATION,
	MATCH_EXIT_SERVER_ACKNOWLEDGE,

	//*******************
	//   Error Message
	//*******************
	MESSAGE_ERROR,

	//TODO: Fill this out as they become needed / invented

};

struct VersionNumber
{
	uint major;
	uint minor;
	uint rev;
};

class Message
{
public:

	//Not ever sent. Just used to return errors
	bool serializeError;

	enum MessageType type;

	//Plain old constructor
	Message();

	virtual char *Serialize(uint *length);
	static Message *Deserialize(char *buffer, uint length);

	static Message *ReadMessage(int connectFD);
	static bool WriteMessage(Message *message, int connectFD);

};

}


#endif /* MESSAGE_H_ */

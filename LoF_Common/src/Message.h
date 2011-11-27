//============================================================================
// Name        : Unit.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Message class which is passed to/from client/server
//============================================================================

#ifndef MESSAGE_H_
#define MESSAGE_H_

#define MESSAGE_MIN_SIZE	sizeof(enum MessageType)

#include <stdlib.h>

using namespace std;

namespace LoF
{

enum MessageType
{
	//Messages for first contacting the server and authenticating
	CLIENT_HELLO = 0,
	SERVER_HELLO,
	CLIENT_AUTH,
	SERVER_AUTH_REPLY,

	//Messages for creating a new match
	MATCH_CREATE_REQUEST,
	MATCH_CREATE_OPTIONS_AVAILABLE,
	MATCH_CREATE_OPTIONS_CHOSEN,
	MATCH_CREATE_ERROR,
	MATCH_CREATE_REPLY,

	//Joining a match already created
	MATCH_JOIN_REQUEST,
	MATCH_JOIN_REPLY,

	//TODO: Fill this out as they become needed / invented

};

class Message
{
public:

	enum MessageType type;

	//Plain old constructor
	Message();
	//Deserialize the buffer into this message
	Message(char *buffer, uint length);

	//Serializes the Message into newly allocated memory
	//	**buffer: The address of a pointer to a buffer.
	//
	//	Serialize will allocate memory on the heap for this object,
	//	then change buffer to point to that
	uint Serialize(char **buffer);

};

}


#endif /* MESSAGE_H_ */

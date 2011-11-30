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

enum AuthMechanism
{
	HASHED_SALTED_PASS = 0,
	SSH_KEY,
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

	enum MessageType type;

	char username[USERNAME_MAX_LENGTH];

	struct VersionNumber serverVersion;

	enum AuthMechanism authMechanism;

	bool authSuccess;

	//Plain old constructor
	Message();

	//Read a message from the given socket
	Message(int ConnectFD);

	//Serializes the Message into newly allocated memory
	//	**buffer: The address of a pointer to a buffer.
	//
	//	Serialize will allocate memory on the heap for this object,
	//	then change buffer to point to that
	//
	//	Returns: The length of the serialized buffer
	uint Serialize(char **buffer);

};

}


#endif /* MESSAGE_H_ */

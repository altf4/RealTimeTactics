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

enum MessageType: char
{
	//Messages for first contacting the server and authenticating
	CLIENT_HELLO = 0,
	SERVER_HELLO,
	CLIENT_AUTH,
	SERVER_AUTH_REPLY,

	//Messages for Querying matches
	MATCH_LIST_REQUEST,
	MATCH_LIST_REPLY,

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

enum AuthResult: char
{
	//Success!
	AUTH_SUCCESS = 0,

	//Failure... :(
	USERNAME_ALREADY_EXISTS,	//For creating a new account
	INCORRECT_PASSWORD,
	USERNAME_NOT_FOUND,			//For logging into an existing account
	INVALID_USERNAME,
	INCOMPATIBLE_SOFTWARE_VERSIONS,

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

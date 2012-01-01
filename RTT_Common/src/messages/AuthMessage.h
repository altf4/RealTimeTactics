//============================================================================
// Name        : Unit.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Message class pertaining to client authentication
//============================================================================

#ifndef AUTHMESSAGE_H_
#define AUTHMESSAGE_H_

#include "Message.h"
#include "../Player.h"

using namespace std;

namespace RTT
{

enum AuthMechanism: uint32_t
{
	HASHED_SALTED_PASS = 0,
	SSH_KEY,
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

class AuthMessage: public Message
{

public:
	enum AuthMechanism authMechanism;
	struct VersionNumber softwareVersion;
	char username[USERNAME_MAX_LENGTH];
	//TODO: I didn't want to import libssl just to use SHA256_DIGEST_LENGTH
	unsigned char hashedPassword[32];
	enum AuthResult authSuccess;
	struct PlayerDescription playerDescription;

	AuthMessage();
	AuthMessage(char *buffer, uint length);
	char *Serialize(uint *length);

};

}



#endif /* AUTHMESSAGE_H_ */

//============================================================================
// Name        : AuthMesssage.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Message class pertaining to client authentication
//============================================================================

#ifndef AUTHMESSAGE_H_
#define AUTHMESSAGE_H_

#include "Message.h"
#include "../../Player.h"

namespace RTT
{

enum AuthMechanism: char
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

enum AuthType: char
{
	CLIENT_HELLO = 0,
	SERVER_HELLO,
	CLIENT_AUTH,
	SERVER_AUTH_REPLY,
};

class AuthMessage: public Message
{

public:
	enum AuthMechanism m_authMechanism;
	struct VersionNumber m_softwareVersion;
	char m_username[USERNAME_MAX_LENGTH];

	//I didn't want to import libssl just to use SHA256_DIGEST_LENGTH
	unsigned char m_hashedPassword[32];

	enum AuthResult m_authSuccess;
	struct PlayerDescription m_playerDescription;

	enum AuthType m_authType;

	AuthMessage(enum AuthType type, enum ProtocolDirection direction);
	AuthMessage(char *buffer, uint length);
	char *Serialize(uint *length);

};

}

#endif /* AUTHMESSAGE_H_ */

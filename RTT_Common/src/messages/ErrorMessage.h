//============================================================================
// Name        : Unit.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Message class pertaining to sending and receiving error conditions
//============================================================================

#ifndef ERRORMESSAGE_H_
#define ERRORMESSAGE_H_


#include "Message.h"

namespace RTT
{

enum ErrorType: char
{
	PROTOCOL_ERROR = 0,
	SERVER_FAULT,

	//Authentication Errors
	AUTHENTICATION_ERROR,
	INCOMPATIBLE_SOFTWARE_VERSION,

	//Failure, Invalid options
	INVALID_MAX_PLAYERS,

	//Failure joining
	MATCH_IS_FULL,
	MATCH_DOESNT_EXIST,
	NOT_ALLOWED_IN,
	ALREADY_IN_MATCH,

	//Failure leaving
	NOT_IN_THAT_MATCH,

	//Failure changing attributes
	NOT_ALLOWED_TO_CHANGE_THAT,
	NO_SUCH_PLAYER,

	//Failure, General server stuff
	TOO_BUSY,
	SHUTTING_DOWN_SOON,

};

class ErrorMessage: public Message
{
public:

	enum ErrorType m_errorType;

	ErrorMessage();
	ErrorMessage(char *buffer, uint length);
	char *Serialize(uint *length);

};

}

#endif /* ERRORMESSAGE_H_ */

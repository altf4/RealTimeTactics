//============================================================================
// Name        : ErrorMessage.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Message class pertaining to sending and receiving error conditions
//============================================================================

#ifndef ERRORMESSAGE_H_
#define ERRORMESSAGE_H_


#include "Message.h"

#define ERROR_MSG_MIN_SIZE 2

namespace RTT
{

enum ErrorType: char
{
	//Errors which are never sent remotely
	ERROR_SOCKET_CLOSED = 0,
	ERROR_TIMEOUT,
	ERROR_MALFORMED_MESSAGE,
	ERROR_UNKNOWN_MESSAGE_TYPE,

	PROTOCOL_ERROR,
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

class ErrorMessage : public Message
{

public:

	ErrorMessage(enum ErrorType errorType);
	ErrorMessage(enum ErrorType errorType, uint32_t ourSerial);

	//Deserialization constructor
	//	buffer - pointer to array in memory where serialized ControlMessage resides
	//	length - the length of this array
	//	On error, sets m_serializeError to true, on success sets it to false
	ErrorMessage(char *buffer, uint32_t length);

	enum ErrorType m_errorType;


	//Serializes the ErrorMessage object into a char array
	//	*length - Return parameter, specifies the length of the serialized array returned
	// Returns - A pointer to the serialized array
	//	NOTE: The caller must manually free() the returned buffer after use
	char *Serialize(uint32_t *length);
};

}

#endif /* ERRORMESSAGE_H_ */

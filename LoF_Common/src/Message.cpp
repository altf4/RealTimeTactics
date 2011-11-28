//============================================================================
// Name        : Unit.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Message class which is passed to/from client/server
//============================================================================

#include "Message.h"
#include "string.h"

using namespace std;
using namespace LoF;

//Deserialize the buffer into this message
Message::Message(char *buffer, uint length)
{
	//Copy the message type
	memcpy(&type, buffer, MESSAGE_MIN_SIZE);
	buffer += MESSAGE_MIN_SIZE;

	switch(type)
	{
		case CLIENT_HELLO:
		{
			//Nothing to do here. This message is only the type
			break;
		}
		case SERVER_HELLO:
		{
			//Uses: 1) Version Number
			//		2) AuthMechanism

			memcpy(&serverVersion, buffer, sizeof(serverVersion));
			buffer += sizeof(serverVersion);

			memcpy(&authMechanism, buffer, sizeof(authMechanism));

			break;
		}
		case CLIENT_AUTH:
		{

			break;
		}
		case SERVER_AUTH_REPLY:
		{

			break;
		}
		case MATCH_CREATE_REQUEST:
		{

			break;
		}
		case MATCH_CREATE_OPTIONS_AVAILABLE:
		{

			break;
		}
		case MATCH_CREATE_OPTIONS_CHOSEN:
		{

			break;
		}
		case MATCH_CREATE_ERROR:
		{

			break;
		}
		case MATCH_CREATE_REPLY:
		{

			break;
		}
		case MATCH_JOIN_REQUEST:
		{

			break;
		}
		case MATCH_JOIN_REPLY:
		{

			break;
		}

		default:
		{
			//Error case
		}
	}

}

//Serializes the Message into newly allocated memory
//	**buffer: The address of a pointer to a buffer.
//
//	Serialize will allocate memory on the heap for this object,
//	then change buffer to point to that
//
//	Returns: The length of the serialized buffer
uint Message::Serialize(char **buffer)
{

	switch(type)
	{
		case CLIENT_HELLO:
		{
			//Allocate the memory and assign it to *buffer
			*buffer = (char*)malloc(MESSAGE_MIN_SIZE);

			//Put the type in
			memcpy(*buffer, &type, MESSAGE_MIN_SIZE);
			return MESSAGE_MIN_SIZE;
		}
		case SERVER_HELLO:
		{
			//Uses: 1) Version Number
			//		2) AuthMechanism

			uint messageSize = MESSAGE_MIN_SIZE + sizeof(authMechanism)
							+ sizeof(serverVersion);
			//Allocate the memory and assign it to *buffer
			*buffer = (char*)malloc(messageSize);

			memcpy(*buffer, &serverVersion, sizeof(serverVersion));
			memcpy(*buffer + sizeof(serverVersion), &authMechanism, sizeof(authMechanism));

			return messageSize;
		}
		case CLIENT_AUTH:
		{

			break;
		}
		case SERVER_AUTH_REPLY:
		{

			break;
		}
		case MATCH_CREATE_REQUEST:
		{

			break;
		}
		case MATCH_CREATE_OPTIONS_AVAILABLE:
		{

			break;
		}
		case MATCH_CREATE_OPTIONS_CHOSEN:
		{

			break;
		}
		case MATCH_CREATE_ERROR:
		{

			break;
		}
		case MATCH_CREATE_REPLY:
		{

			break;
		}
		case MATCH_JOIN_REQUEST:
		{

			break;
		}
		case MATCH_JOIN_REPLY:
		{

			break;
		}

		default:
		{
			//Error case
			return -1;
		}
	}
	return -1;
}

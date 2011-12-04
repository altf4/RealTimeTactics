//============================================================================
// Name        : Unit.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Message class which is passed to/from client/server
//============================================================================

#include "AuthMessage.h"
#include "string.h"

using namespace std;
using namespace LoF;

AuthMessage::AuthMessage()
{

}

AuthMessage::AuthMessage(char *buffer, uint length)
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
			//TODO: Fill in authentication
			break;
		}
		case SERVER_AUTH_REPLY:
		{
			//Uses: 1) authSuccess

			memcpy(&authSuccess, buffer, sizeof(bool));

			break;
		}
		default:
		{
			//error
		}
	}
}

char *AuthMessage::Serialize(uint *length)
{
	char *buffer, *originalBuffer;
	switch(type)
	{
		case CLIENT_HELLO:
		{
			//Allocate the memory and assign it to *buffer
			buffer = (char*)malloc(MESSAGE_MIN_SIZE);
			originalBuffer = buffer;

			//Put the type in
			memcpy(buffer, &type, MESSAGE_MIN_SIZE);
			*length = MESSAGE_MIN_SIZE;
			return originalBuffer;
		}
		case SERVER_HELLO:
		{
			//Uses: 1) Version Number
			//		2) AuthMechanism

			uint messageSize = MESSAGE_MIN_SIZE + sizeof(authMechanism)
							+ sizeof(serverVersion);
			//Allocate the memory and assign it to *buffer
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			//Put the type in
			memcpy(buffer, &type, MESSAGE_MIN_SIZE);
			buffer += MESSAGE_MIN_SIZE;
			memcpy(buffer, &serverVersion, sizeof(serverVersion));
			buffer += sizeof(serverVersion);
			memcpy(buffer, &authMechanism, sizeof(authMechanism));
			*length = messageSize;

			return originalBuffer;
		}
		case CLIENT_AUTH:
		{
			//Allocate the memory and assign it to *buffer
			buffer = (char*)malloc(MESSAGE_MIN_SIZE);
			originalBuffer = buffer;

			//Put the type in
			memcpy(buffer, &type, MESSAGE_MIN_SIZE);
			*length = MESSAGE_MIN_SIZE;
			return originalBuffer;
		}
		case SERVER_AUTH_REPLY:
		{
			uint messageSize = MESSAGE_MIN_SIZE + sizeof(bool);
			//Allocate the memory and assign it to *buffer
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			//Put the type in
			memcpy(buffer, &type, MESSAGE_MIN_SIZE);
			buffer += MESSAGE_MIN_SIZE;

			memcpy(buffer, &authSuccess, sizeof(bool));
			*length = messageSize;
			return originalBuffer;
		}
		default:
		{
			//Error
			return NULL;
		}
	}
}

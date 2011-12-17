//============================================================================
// Name        : Unit.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Message class pertaining to sending and receiving error conditions
//============================================================================

#include "ErrorMessage.h"
#include "string.h"

using namespace std;
using namespace LoF;

ErrorMessage::ErrorMessage()
{

}

ErrorMessage::ErrorMessage(char *buffer, uint length)
{
	uint expectedSize = MESSAGE_MIN_SIZE + sizeof(errorType);
	if( length != expectedSize )
	{
		serializeError = true;
		return;
	}

	serializeError = false;

	memcpy(&type, buffer, MESSAGE_MIN_SIZE);
	buffer += MESSAGE_MIN_SIZE;

	memcpy(&errorType, buffer, sizeof(errorType));
	buffer += sizeof(errorType);

}

char *ErrorMessage::Serialize(uint *length)
{
	char *buffer, *originalBuffer;
	uint messageSize;

	messageSize = MESSAGE_MIN_SIZE +sizeof(errorType);
	buffer = (char*)malloc(messageSize);
	originalBuffer = buffer;

	//Message type
	memcpy(buffer, &type, MESSAGE_MIN_SIZE);
	buffer += MESSAGE_MIN_SIZE;
	//Error type
	memcpy(buffer, &errorType, sizeof(errorType));
	buffer += sizeof(errorType);

	*length = messageSize;

	return originalBuffer;
}

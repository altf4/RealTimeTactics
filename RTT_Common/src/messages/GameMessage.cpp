//============================================================================
// Name        : GameMessage.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Message class for in-game actions
//============================================================================

//============================================================================
// Name        : ErrorMessage.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Message class pertaining to sending and receiving error conditions
//============================================================================

#include "GameMessage.h"
#include "string.h"

using namespace std;
using namespace RTT;

GameMessage::GameMessage()
{

}

GameMessage::GameMessage(char *buffer, uint32_t length)
{
	if( length < MESSAGE_MIN_SIZE )
	{
		return;
	}

	serializeError = false;

	//Copy the message type
	memcpy(&type, buffer, MESSAGE_MIN_SIZE);
	buffer += MESSAGE_MIN_SIZE;

	switch(type)
	{
		case MATCH_LIST_REQUEST: //TODO make a new message type
		{
			break;
		}
		default:
		{
			break;
		}
	}
}

char *GameMessage::Serialize(uint32_t *length)
{
	char *buffer, *originalBuffer;
	uint32_t messageSize;
	switch(type)
	{
		case MATCH_LIST_REQUEST: //TODO make a new message type
		{
			break;
		}
		default:
		{
			break;
		}
	}

	*length = messageSize;
	return originalBuffer;
}




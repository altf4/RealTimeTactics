//============================================================================
// Name        : Unit.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Message class sent while player is in the server lobby
//============================================================================

#include "LobbyMessage.h"

using namespace std;
using namespace LoF;

LobbyMessage::LobbyMessage()
{

}

LobbyMessage::LobbyMessage(char *buffer, uint length)
{

}

char *LobbyMessage::Serialize(uint *length)
{
	char *buffer, *originalBuffer;
	switch(type)
	{
		case MATCH_LIST_REQUEST:
		{
			//Uses: 1) Message Type
			//		2) Page count

			uint messageSize = MESSAGE_MIN_SIZE;

			*length = messageSize;
			return originalBuffer;
		}
		case MATCH_LIST_REPLY:
		{
			uint messageSize = MESSAGE_MIN_SIZE;

			*length = messageSize;
			return originalBuffer;
		}
		//Messages for creating a new match
		case MATCH_CREATE_REQUEST:
		{
			return originalBuffer;
		}
		case MATCH_CREATE_OPTIONS_AVAILABLE:
		{
			uint messageSize = MESSAGE_MIN_SIZE;

			*length = messageSize;
			return originalBuffer;
		}
		case MATCH_CREATE_OPTIONS_CHOSEN:
		{
			uint messageSize = MESSAGE_MIN_SIZE;

			*length = messageSize;
			return originalBuffer;
		}
		case MATCH_CREATE_ERROR:
		{
			uint messageSize = MESSAGE_MIN_SIZE;

			*length = messageSize;
			return originalBuffer;
		}
		case MATCH_CREATE_REPLY:
		{
			uint messageSize = MESSAGE_MIN_SIZE;

			*length = messageSize;
			return originalBuffer;
		}
		//Joining a match already created
		case MATCH_JOIN_REQUEST:
		{
			uint messageSize = MESSAGE_MIN_SIZE;

			*length = messageSize;
			return originalBuffer;
		}
		case MATCH_JOIN_REPLY:
		{
			uint messageSize = MESSAGE_MIN_SIZE;

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

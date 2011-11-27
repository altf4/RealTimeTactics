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
	memcpy(&type, buffer, sizeof(enum MessageType));

	switch(type)
	{
		case CLIENT_HELLO:
		{

			break;
		}
		case SERVER_HELLO:
		{

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

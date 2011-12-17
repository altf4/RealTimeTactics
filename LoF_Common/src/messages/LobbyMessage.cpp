//============================================================================
// Name        : Unit.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Message class sent while player is in the server lobby
//============================================================================

#include "LobbyMessage.h"
#include "string.h"

using namespace std;
using namespace LoF;

LobbyMessage::LobbyMessage()
{

}

LobbyMessage::~LobbyMessage()
{
	if(matchDescriptions != NULL)
	{
		free(matchDescriptions);
	}
}

LobbyMessage::LobbyMessage(char *buffer, uint length)
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
		case MATCH_LIST_REQUEST:
		{
			//Uses: 1) Message Type
			//		2) Page number
			uint expectedSize = MESSAGE_MIN_SIZE + sizeof(requestedPage);
			if( length != expectedSize)
			{
				serializeError = true;
				return;
			}

			//Requested page
			memcpy(&requestedPage, buffer, sizeof(requestedPage));
			buffer += sizeof(requestedPage);

			break;
		}
		case MATCH_LIST_REPLY:
		{
			//Uses: 1) Message Type
			//		2) Returned Matches Count
			//		3) Match Descriptions

			//Returned Matches Count
			memcpy(&returnedMatchesCount, buffer, sizeof(returnedMatchesCount));
			buffer += sizeof(returnedMatchesCount);

			uint expectedSize = MESSAGE_MIN_SIZE + sizeof(returnedMatchesCount)
					+ (returnedMatchesCount * sizeof(struct MatchDescription));
			if( length != expectedSize)
			{
				serializeError = true;
				return;
			}

			//All the returned matches
			matchDescriptions = (struct MatchDescription *)malloc(returnedMatchesCount
					* sizeof(struct MatchDescription));
			memcpy(matchDescriptions, buffer, returnedMatchesCount
					* sizeof(struct MatchDescription));

			break;
		}
		//Messages for creating a new match
		case MATCH_CREATE_REQUEST:
		{
			//Uses: 1) Message Type
			uint expectedSize = MESSAGE_MIN_SIZE;
			if( length != expectedSize)
			{
				serializeError = true;
				return;
			}

			break;
		}
		case MATCH_CREATE_OPTIONS_AVAILABLE:
		{
			//Uses: 1) Message Type
			//		2) Match Options offered by server
			uint expectedSize = MESSAGE_MIN_SIZE + sizeof(options);
			if( length != expectedSize)
			{
				serializeError = true;
				return;
			}

			//MatchOptions
			memcpy(&options, buffer, sizeof(options));
			buffer += sizeof(options);

			break;

		}
		case MATCH_CREATE_OPTIONS_CHOSEN:
		{
			//Uses: 1) Message Type
			//		2) Match Options Set by client
			uint expectedSize = MESSAGE_MIN_SIZE + sizeof(options);
			if( length != expectedSize)
			{
				serializeError = true;
				return;
			}

			//MatchOptions
			memcpy(&options, buffer, sizeof(options));
			buffer += sizeof(options);

			break;

		}
		case MATCH_CREATE_REPLY:
		{
			//Uses: 1) Message Type
			//		2) Description of newly created match
			uint expectedSize = MESSAGE_MIN_SIZE + sizeof(matchDescription);
			if( length != expectedSize)
			{
				serializeError = true;
				return;
			}

			//Match description
			memcpy(&matchDescription, buffer, sizeof(matchDescription));
			buffer += sizeof(matchDescription);

			break;

		}
		//Joining a match already created
		case MATCH_JOIN_REQUEST:
		{
			//Uses: 1) Message Type
			//		2) ID of the match to join
			uint expectedSize = MESSAGE_MIN_SIZE + sizeof(ID);
			if( length != expectedSize)
			{
				serializeError = true;
				return;
			}

			//Match description
			memcpy(&ID, buffer, sizeof(ID));
			buffer += sizeof(ID);

			break;
		}
		case MATCH_JOIN_REPLY:
		{
			//Uses: 1) Message Type
			//		2) Description of newly created match
			uint expectedSize = MESSAGE_MIN_SIZE + sizeof(matchDescription);
			if( length != expectedSize)
			{
				serializeError = true;
				return;
			}

			//Match description
			memcpy(&matchDescription, buffer, sizeof(matchDescription));
			buffer += sizeof(matchDescription);

			break;
		}
		//Leaving a match you're in
		case MATCH_LEAVE_NOTIFICATION:
		{
			//Uses: 1) Message Type
			//		2) ID of the match to leave
			uint expectedSize = MESSAGE_MIN_SIZE + sizeof(ID);
			if( length != expectedSize)
			{
				serializeError = true;
				return;
			}

			//Match description
			memcpy(&ID, buffer, sizeof(ID));
			buffer += sizeof(ID);

			break;
		}
		case MATCH_LEAVE_ACKNOWLEDGE:
		{
			//Uses: 1) Message Type
			uint expectedSize = MESSAGE_MIN_SIZE;
			if( length != expectedSize)
			{
				serializeError = true;
				return;
			}

			break;
		}
		case MATCH_EXIT_SERVER_NOTIFICATION:
		{
			//Uses: 1) Message Type
			uint expectedSize = MESSAGE_MIN_SIZE;
			if( length != expectedSize)
			{
				serializeError = true;
				return;
			}

			break;
		}
		case MATCH_EXIT_SERVER_ACKNOWLEDGE:
		{
			//Uses: 1) Message Type
			uint expectedSize = MESSAGE_MIN_SIZE;
			if( length != expectedSize)
			{
				serializeError = true;
				return;
			}

			break;
		}
		default:
		{
			//Error
			return;
		}
	}

}

char *LobbyMessage::Serialize(uint *length)
{
	char *buffer, *originalBuffer;
	uint messageSize;
	switch(type)
	{
		case MATCH_LIST_REQUEST:
		{
			//Uses: 1) Message Type
			//		2) Page number

			messageSize = MESSAGE_MIN_SIZE + sizeof(requestedPage);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			//Put the type in
			memcpy(buffer, &type, MESSAGE_MIN_SIZE);
			buffer += MESSAGE_MIN_SIZE;
			//Page Count
			memcpy(buffer, &requestedPage, sizeof(requestedPage));
			buffer += sizeof(requestedPage);

			break;
		}
		case MATCH_LIST_REPLY:
		{
			//Uses: 1) Message Type
			//		2) Returned Matches Count
			//		3) Match Descriptions
			messageSize = MESSAGE_MIN_SIZE + sizeof(returnedMatchesCount)
					+ (returnedMatchesCount * sizeof(struct MatchDescription));
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			//Put the type in
			memcpy(buffer, &type, MESSAGE_MIN_SIZE);
			buffer += MESSAGE_MIN_SIZE;

			//Put the type in
			memcpy(buffer, &returnedMatchesCount, sizeof(returnedMatchesCount));
			buffer += sizeof(returnedMatchesCount);

			//Put the match descriptions in
			for(uint i = 0; i < returnedMatchesCount; i++)
			{
				memcpy(buffer, &matchDescriptions[i], sizeof(struct MatchDescription));
				buffer += sizeof(struct MatchDescription);
			}
			break;
		}
		//Messages for creating a new match
		case MATCH_CREATE_REQUEST:
		{
			//Uses: 1) Message Type
			messageSize = MESSAGE_MIN_SIZE;
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			//Put the type in
			memcpy(buffer, &type, MESSAGE_MIN_SIZE);
			buffer += MESSAGE_MIN_SIZE;

			break;
		}
		case MATCH_CREATE_OPTIONS_AVAILABLE:
		{
			//Uses: 1) Message Type
			//		2) Match Options offered by server
			messageSize = MESSAGE_MIN_SIZE + sizeof(options);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			//TODO: Fill in with match options allowed by server

			//Put the type in
			memcpy(buffer, &type, MESSAGE_MIN_SIZE);
			buffer += MESSAGE_MIN_SIZE;

			//Max players
			memcpy(buffer, &options, sizeof(options));
			buffer += sizeof(options);

			break;
		}
		case MATCH_CREATE_OPTIONS_CHOSEN:
		{
			//Uses: 1) Message Type
			//		2) Match Options chosen by client
			messageSize = MESSAGE_MIN_SIZE + sizeof(options);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			//Put the type in
			memcpy(buffer, &type, MESSAGE_MIN_SIZE);
			buffer += MESSAGE_MIN_SIZE;

			//Max players
			memcpy(buffer, &options, sizeof(options));
			buffer += sizeof(options);

			break;
		}
		case MATCH_CREATE_REPLY:
		{
			//Uses: 1) Message Type
			//		2) Description of newly created match
			messageSize = MESSAGE_MIN_SIZE + sizeof(matchDescription);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			//Put the type in
			memcpy(buffer, &type, MESSAGE_MIN_SIZE);
			buffer += MESSAGE_MIN_SIZE;

			//New match description
			memcpy(buffer, &matchDescription, sizeof(matchDescription));
			buffer += sizeof(matchDescription);

			break;
		}
		//Joining a match already created
		case MATCH_JOIN_REQUEST:
		{
			//Uses: 1) Message Type
			//		2) ID of the match to join
			messageSize = MESSAGE_MIN_SIZE + sizeof(ID);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			//Put the type in
			memcpy(buffer, &type, MESSAGE_MIN_SIZE);
			buffer += MESSAGE_MIN_SIZE;

			//ID of the match to join
			memcpy(buffer, &ID, sizeof(ID));
			buffer += sizeof(ID);

			break;
		}
		case MATCH_JOIN_REPLY:
		{
			//Uses: 1) Message Type
			//		2) Match Description
			messageSize = MESSAGE_MIN_SIZE + sizeof(matchDescription);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			//Put the type in
			memcpy(buffer, &type, MESSAGE_MIN_SIZE);
			buffer += MESSAGE_MIN_SIZE;

			//Match Description
			memcpy(buffer, &matchDescription, sizeof(matchDescription));
			buffer += sizeof(matchDescription);

			break;
		}
		//Leaving a match you're in
		case MATCH_LEAVE_NOTIFICATION:
		{
			//Uses: 1) Message Type
			//		2) ID of the match to leave
			messageSize = MESSAGE_MIN_SIZE + sizeof(ID);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			memcpy(buffer, &type, MESSAGE_MIN_SIZE);
			buffer += MESSAGE_MIN_SIZE;

			memcpy(buffer, &ID, sizeof(ID));
			buffer += sizeof(ID);

			break;
		}
		case MATCH_LEAVE_ACKNOWLEDGE:
		{
			//Uses: 1) Message Type
			messageSize = MESSAGE_MIN_SIZE;
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			//Put the type in
			memcpy(buffer, &type, MESSAGE_MIN_SIZE);
			buffer += MESSAGE_MIN_SIZE;

			break;
		}
		//Leaving a match you're in
		case MATCH_EXIT_SERVER_NOTIFICATION:
		{
			//Uses: 1) Message Type
			messageSize = MESSAGE_MIN_SIZE;
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			memcpy(buffer, &type, MESSAGE_MIN_SIZE);
			buffer += MESSAGE_MIN_SIZE;

			break;
		}
		case MATCH_EXIT_SERVER_ACKNOWLEDGE:
		{
			//Uses: 1) Message Type
			messageSize = MESSAGE_MIN_SIZE;
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			//Put the type in
			memcpy(buffer, &type, MESSAGE_MIN_SIZE);
			buffer += MESSAGE_MIN_SIZE;

			break;
		}
		default:
		{
			//Error
			return NULL;
		}
	}

	*length = messageSize;
	return originalBuffer;
}

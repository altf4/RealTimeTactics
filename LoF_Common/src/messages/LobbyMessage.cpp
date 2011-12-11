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
	if(matchDescription != NULL)
	{
		free(matchDescription);
	}
}

LobbyMessage::LobbyMessage(char *buffer, uint length)
{
	if( length < MESSAGE_MIN_SIZE )
	{
		return;
	}
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
			uint expectedSize = MESSAGE_MIN_SIZE + sizeof(returnedMatchesCount)
					+ (returnedMatchesCount * sizeof(struct MatchDescription));
			if( length != expectedSize)
			{
				serializeError = true;
				return;
			}

			//Returned Matches Count
			memcpy(&returnedMatchesCount, buffer, sizeof(returnedMatchesCount));
			buffer += sizeof(returnedMatchesCount);

			//All the returned matches
			matchDescription = (struct MatchDescription *)malloc(returnedMatchesCount
					* sizeof(struct MatchDescription));
			memcpy(matchDescription, buffer, returnedMatchesCount
					* sizeof(struct MatchDescription));

			break;
		}
		//Messages for creating a new match
		case MATCH_CREATE_REQUEST:
		{
		}
		case MATCH_CREATE_OPTIONS_AVAILABLE:
		{

		}
		case MATCH_CREATE_OPTIONS_CHOSEN:
		{

		}
		case MATCH_CREATE_ERROR:
		{

		}
		case MATCH_CREATE_REPLY:
		{

		}
		//Joining a match already created
		case MATCH_JOIN_REQUEST:
		{

		}
		case MATCH_JOIN_REPLY:
		{

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
	switch(type)
	{
		case MATCH_LIST_REQUEST:
		{
			//Uses: 1) Message Type
			//		2) Page number

			uint messageSize = MESSAGE_MIN_SIZE + sizeof(requestedPage);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			//Put the type in
			memcpy(buffer, &type, MESSAGE_MIN_SIZE);
			buffer += MESSAGE_MIN_SIZE;
			//Page Count
			memcpy(buffer, &requestedPage, sizeof(requestedPage));
			buffer += sizeof(requestedPage);

			*length = messageSize;
			return originalBuffer;
		}
		case MATCH_LIST_REPLY:
		{
			//Uses: 1) Message Type
			//		2) Returned Matches Count
			//		3) Match Descriptions
			uint messageSize = MESSAGE_MIN_SIZE + sizeof(returnedMatchesCount)
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
				memcpy(buffer, &matchDescription[i], sizeof(struct MatchDescription));
				buffer += sizeof(struct MatchDescription);
			}

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

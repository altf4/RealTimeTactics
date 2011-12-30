//============================================================================
// Name        : Unit.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Message class sent while player is in the server lobby
//============================================================================

#include "LobbyMessage.h"
#include "string.h"

using namespace std;
using namespace RTT;

LobbyMessage::LobbyMessage()
{
	matchDescriptions = NULL;
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
	matchDescriptions = NULL;
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

			//Get the count of returned matches
			memcpy(&returnedMatchesCount, buffer, sizeof(returnedMatchesCount));
			buffer += sizeof(returnedMatchesCount);

			uint expectedSize = MESSAGE_MIN_SIZE + sizeof(returnedMatchesCount)
					+ (returnedMatchesCount * (MATCH_DESCR_SIZE));
			if( length != expectedSize)
			{
				serializeError = true;
				return;
			}

			//Allocate new space for the match descriptions
			matchDescriptions = (struct MatchDescription *)malloc(returnedMatchesCount
					* (MATCH_DESCR_SIZE));

			//Copy over the memory for the match descriptions
			for(uint i = 0; i < returnedMatchesCount; i++)
			{
				memcpy(&matchDescriptions[i].status, buffer, sizeof(enum Status));
				buffer += sizeof(enum Status);
				memcpy(&matchDescriptions[i].ID, buffer, sizeof(uint));
				buffer += sizeof(uint);
				memcpy(&matchDescriptions[i].currentPlayerCount, buffer, sizeof(uint));
				buffer += sizeof(uint);
				memcpy(&matchDescriptions[i].maxPlayers, buffer, sizeof(uint));
				buffer += sizeof(uint);
				memcpy(&matchDescriptions[i].name, buffer, MAX_MATCHNAME_LEN);
				buffer += MAX_MATCHNAME_LEN;
				memcpy(&matchDescriptions[i].timeCreated, buffer, sizeof(time_t));
				buffer += sizeof(time_t);
			}

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
			uint expectedSize = MESSAGE_MIN_SIZE + MATCH_OPTIONS_SIZE;
			if( length != expectedSize)
			{
				serializeError = true;
				return;
			}

			//MatchOptions
			memcpy(&options.maxPlayers, buffer, sizeof(uint));
			buffer += sizeof(uint);
			memcpy(&options.name, buffer, MAX_MATCHNAME_LEN);
			buffer += MAX_MATCHNAME_LEN;

			break;

		}
		case MATCH_CREATE_OPTIONS_CHOSEN:
		{
			//Uses: 1) Message Type
			//		2) Match Options Set by client
			uint expectedSize = MESSAGE_MIN_SIZE + MATCH_OPTIONS_SIZE;
			if( length != expectedSize)
			{
				serializeError = true;
				return;
			}

			//MatchOptions
			memcpy(&options.maxPlayers, buffer, sizeof(uint));
			buffer += sizeof(uint);
			memcpy(&options.name, buffer, MAX_MATCHNAME_LEN);
			buffer += MAX_MATCHNAME_LEN;

			break;

		}
		case MATCH_CREATE_REPLY:
		{
			//Uses: 1) Message Type
			//		2) Description of newly created match
			uint expectedSize = MESSAGE_MIN_SIZE + MATCH_DESCR_SIZE;
			if( length != expectedSize)
			{
				serializeError = true;
				return;
			}

			//Match description
			memcpy(&matchDescription.status, buffer, sizeof(enum Status));
			buffer += sizeof(enum Status);
			memcpy(&matchDescription.ID, buffer, sizeof(uint));
			buffer += sizeof(uint);
			memcpy(&matchDescription.currentPlayerCount, buffer, sizeof(uint));
			buffer += sizeof(uint);
			memcpy(&matchDescription.maxPlayers, buffer, sizeof(uint));
			buffer += sizeof(uint);
			memcpy(&matchDescription.name, buffer, MAX_MATCHNAME_LEN);
			buffer += MAX_MATCHNAME_LEN;
			memcpy(&matchDescription.timeCreated, buffer, sizeof(time_t));
			buffer += sizeof(time_t);

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
			uint expectedSize = MESSAGE_MIN_SIZE + MATCH_DESCR_SIZE;
			if( length != expectedSize)
			{
				serializeError = true;
				return;
			}

			//Match description
			memcpy(&matchDescription.status, buffer, sizeof(enum Status));
			buffer += sizeof(enum Status);
			memcpy(&matchDescription.ID, buffer, sizeof(uint));
			buffer += sizeof(uint);
			memcpy(&matchDescription.currentPlayerCount, buffer, sizeof(uint));
			buffer += sizeof(uint);
			memcpy(&matchDescription.maxPlayers, buffer, sizeof(uint));
			buffer += sizeof(uint);
			memcpy(&matchDescription.name, buffer, MAX_MATCHNAME_LEN);
			buffer += MAX_MATCHNAME_LEN;
			memcpy(&matchDescription.timeCreated, buffer, sizeof(time_t));
			buffer += sizeof(time_t);

			break;
		}
		case SERVER_STATS_REQUEST:
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
		case SERVER_STATS_REPLY:
		{
			//Uses: 1) Message Type
			uint expectedSize = MESSAGE_MIN_SIZE + SERVER_STATS_SIZE;
			if( length != expectedSize)
			{
				serializeError = true;
				return;
			}

			//Match description
			memcpy(&serverStats.numMatches, buffer, sizeof(uint));
			buffer += sizeof(uint);
			memcpy(&serverStats.numPlayers, buffer, sizeof(uint));
			buffer += sizeof(uint);

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
					+ (returnedMatchesCount * (MATCH_DESCR_SIZE));
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			//Put the type in
			memcpy(buffer, &type, MESSAGE_MIN_SIZE);
			buffer += MESSAGE_MIN_SIZE;

			//Put match count in
			memcpy(buffer, &returnedMatchesCount, sizeof(returnedMatchesCount));
			buffer += sizeof(returnedMatchesCount);

			//Put the match descriptions in
			for(uint i = 0; i < returnedMatchesCount; i++)
			{
				//New match description
				memcpy(buffer, &matchDescriptions[i].status, sizeof(enum Status));
				buffer += sizeof(enum Status);
				memcpy(buffer, &matchDescriptions[i].ID, sizeof(uint));
				buffer += sizeof(uint);
				memcpy(buffer, &matchDescriptions[i].currentPlayerCount, sizeof(uint));
				buffer += sizeof(uint);
				memcpy(buffer, &matchDescriptions[i].maxPlayers, sizeof(uint));
				buffer += sizeof(uint);
				memcpy(buffer, &matchDescriptions[i].name, MAX_MATCHNAME_LEN);
				buffer += MAX_MATCHNAME_LEN;
				memcpy(buffer, &matchDescriptions[i].timeCreated, sizeof(time_t));
				buffer += sizeof(time_t);
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
			messageSize = MESSAGE_MIN_SIZE + MATCH_OPTIONS_SIZE;
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			//Put the type in
			memcpy(buffer, &type, MESSAGE_MIN_SIZE);
			buffer += MESSAGE_MIN_SIZE;

			//Options
			memcpy(buffer, &options.maxPlayers, sizeof(uint));
			buffer += sizeof(uint);
			memcpy(buffer, &options.name, MAX_MATCHNAME_LEN);
			buffer += MAX_MATCHNAME_LEN;

			break;
		}
		case MATCH_CREATE_OPTIONS_CHOSEN:
		{
			//Uses: 1) Message Type
			//		2) Match Options chosen by client
			messageSize = MESSAGE_MIN_SIZE + MATCH_OPTIONS_SIZE;
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			//Put the type in
			memcpy(buffer, &type, MESSAGE_MIN_SIZE);
			buffer += MESSAGE_MIN_SIZE;

			//options
			memcpy(buffer, &options.maxPlayers, sizeof(uint));
			buffer += sizeof(uint);
			memcpy(buffer, &options.name, MAX_MATCHNAME_LEN);
			buffer += MAX_MATCHNAME_LEN;

			break;
		}
		case MATCH_CREATE_REPLY:
		{
			//Uses: 1) Message Type
			//		2) Description of newly created match
			messageSize = MESSAGE_MIN_SIZE + MATCH_DESCR_SIZE;
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			//Put the type in
			memcpy(buffer, &type, MESSAGE_MIN_SIZE);
			buffer += MESSAGE_MIN_SIZE;

			//New match description
			memcpy(buffer, &matchDescription.status, sizeof(enum Status));
			buffer += sizeof(enum Status);
			memcpy(buffer, &matchDescription.ID, sizeof(uint));
			buffer += sizeof(uint);
			memcpy(buffer, &matchDescription.currentPlayerCount, sizeof(uint));
			buffer += sizeof(uint);
			memcpy(buffer, &matchDescription.maxPlayers, sizeof(uint));
			buffer += sizeof(uint);
			memcpy(buffer, &matchDescription.name, MAX_MATCHNAME_LEN);
			buffer += MAX_MATCHNAME_LEN;
			memcpy(buffer, &matchDescription.timeCreated, sizeof(time_t));
			buffer += sizeof(time_t);
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
			messageSize = MESSAGE_MIN_SIZE + MATCH_DESCR_SIZE;
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			//Put the type in
			memcpy(buffer, &type, MESSAGE_MIN_SIZE);
			buffer += MESSAGE_MIN_SIZE;

			//Match Description
			memcpy(buffer, &matchDescription.status, sizeof(enum Status));
			buffer += sizeof(enum Status);
			memcpy(buffer, &matchDescription.ID, sizeof(uint));
			buffer += sizeof(uint);
			memcpy(buffer, &matchDescription.currentPlayerCount, sizeof(uint));
			buffer += sizeof(uint);
			memcpy(buffer, &matchDescription.maxPlayers, sizeof(uint));
			buffer += sizeof(uint);
			memcpy(buffer, &matchDescription.name, MAX_MATCHNAME_LEN);
			buffer += MAX_MATCHNAME_LEN;
			memcpy(buffer, &matchDescription.timeCreated, sizeof(time_t));
			buffer += sizeof(time_t);

			break;
		}
		case SERVER_STATS_REQUEST:
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
		case SERVER_STATS_REPLY:
		{
			//Uses: 1) Message Type
			//		2) Server stats
			messageSize = MESSAGE_MIN_SIZE + SERVER_STATS_SIZE;
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			//Put the type in
			memcpy(buffer, &type, MESSAGE_MIN_SIZE);
			buffer += MESSAGE_MIN_SIZE;

			//Put the stats struct in
			memcpy(buffer, &serverStats.numMatches, sizeof(uint));
			buffer += sizeof(uint);
			memcpy(buffer, &serverStats.numPlayers, sizeof(uint));
			buffer += sizeof(uint);

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

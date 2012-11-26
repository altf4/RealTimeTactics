//============================================================================
// Name        : LobbyMessage.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Message class sent while player is in the server lobby
//============================================================================

#include "LobbyMessage.h"
#include "string.h"

using namespace std;
using namespace RTT;

LobbyMessage::LobbyMessage(enum LobbyType type)
{
	m_matchDescriptions = NULL;
	m_playerDescriptions = NULL;
	m_messageType = MESSAGE_LOBBY;
	m_lobbyType = type;
}

LobbyMessage::~LobbyMessage()
{
	if(m_matchDescriptions != NULL)
	{
		free(m_matchDescriptions);
	}
	if(m_playerDescriptions != NULL)
	{
		free(m_playerDescriptions);
	}
}

LobbyMessage::LobbyMessage(char *buffer, uint32_t length)
{
	m_matchDescriptions = NULL;
	m_playerDescriptions = NULL;
	if( length < MESSAGE_HDR_SIZE )
	{
		return;
	}

	m_serializeError = false;

	DeserializeHeader(&buffer);

	memcpy(&m_lobbyType, buffer, sizeof(m_lobbyType));
	buffer += sizeof(m_lobbyType);

	switch(m_lobbyType)
	{
		case MATCH_LIST_REQUEST:
		{
			//Uses: 1) Message Type
			//		2) Page number
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_lobbyType) + sizeof(m_requestedPage);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			//Requested page
			memcpy(&m_requestedPage, buffer, sizeof(m_requestedPage));
			buffer += sizeof(m_requestedPage);

			break;
		}
		case MATCH_LIST_REPLY:
		{
			//Uses: 1) Message Type
			//		2) Returned Matches Count
			//		3) Match Descriptions

			//Get the count of returned matches
			memcpy(&m_returnedMatchesCount, buffer, sizeof(m_returnedMatchesCount));
			buffer += sizeof(m_returnedMatchesCount);

			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_lobbyType) + sizeof(m_returnedMatchesCount)
					+ (m_returnedMatchesCount * (MATCH_DESCR_SIZE));
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			//Allocate new space for the match descriptions
			m_matchDescriptions = (struct MatchDescription *)malloc(m_returnedMatchesCount
					* sizeof(struct MatchDescription));

			//Copy over the memory for the match descriptions
			for(uint32_t i = 0; i < m_returnedMatchesCount; i++)
			{
				memcpy(&m_matchDescriptions[i].m_status, buffer, sizeof(enum Status));
				buffer += sizeof(enum Status);
				memcpy(&m_matchDescriptions[i].m_ID, buffer, sizeof(uint32_t));
				buffer += sizeof(uint32_t);
				memcpy(&m_matchDescriptions[i].m_currentPlayerCount, buffer, sizeof(uint32_t));
				buffer += sizeof(uint32_t);
				memcpy(&m_matchDescriptions[i].m_maxPlayers, buffer, sizeof(uint32_t));
				buffer += sizeof(uint32_t);
				memcpy(&m_matchDescriptions[i].m_name, buffer, MAX_MATCHNAME_LEN);
				buffer += MAX_MATCHNAME_LEN;
				memcpy(&m_matchDescriptions[i].m_timeCreated, buffer, sizeof(int64_t));
				buffer += sizeof(int64_t);
				memcpy(&m_matchDescriptions[i].m_leaderID, buffer, sizeof(uint32_t));
				buffer += sizeof(uint32_t);
			}

			break;
		}
		//Messages for creating a new match
		case MATCH_CREATE_REQUEST:
		{
			//Uses: 1) Message Type
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_lobbyType);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			break;
		}
		case MATCH_CREATE_OPTIONS_AVAILABLE:
		{
			//Uses: 1) Message Type
			//		2) Match Options offered by server
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_lobbyType) + MATCH_OPTIONS_SIZE;
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			//MatchOptions
			memcpy(&m_options.m_maxPlayers, buffer, sizeof(uint32_t));
			buffer += sizeof(uint32_t);
			memcpy(&m_options.m_name, buffer, MAX_MATCHNAME_LEN);
			buffer += MAX_MATCHNAME_LEN;

			break;

		}
		case MATCH_CREATE_OPTIONS_CHOSEN:
		{
			//Uses: 1) Message Type
			//		2) Match Options Set by client
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_lobbyType) + MATCH_OPTIONS_SIZE;
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			//MatchOptions
			memcpy(&m_options.m_maxPlayers, buffer, sizeof(uint32_t));
			buffer += sizeof(uint32_t);
			memcpy(&m_options.m_name, buffer, MAX_MATCHNAME_LEN);
			buffer += MAX_MATCHNAME_LEN;

			break;

		}
		case MATCH_CREATE_REPLY:
		{
			//Uses: 1) Message Type
			//		2) Description of newly created match
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_lobbyType) + MATCH_DESCR_SIZE;
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			//Match description
			memcpy(&m_matchDescription.m_status, buffer, sizeof(enum Status));
			buffer += sizeof(enum Status);
			memcpy(&m_matchDescription.m_ID, buffer, sizeof(uint32_t));
			buffer += sizeof(uint32_t);
			memcpy(&m_matchDescription.m_currentPlayerCount, buffer, sizeof(uint32_t));
			buffer += sizeof(uint32_t);
			memcpy(&m_matchDescription.m_maxPlayers, buffer, sizeof(uint32_t));
			buffer += sizeof(uint32_t);
			memcpy(&m_matchDescription.m_name, buffer, MAX_MATCHNAME_LEN);
			buffer += MAX_MATCHNAME_LEN;
			memcpy(&m_matchDescription.m_timeCreated, buffer, sizeof(int64_t));
			buffer += sizeof(int64_t);
			memcpy(&m_matchDescription.m_leaderID, buffer, sizeof(uint32_t));
			buffer += sizeof(uint32_t);

			break;

		}
		//Joining a match already created
		case MATCH_JOIN_REQUEST:
		{
			//Uses: 1) Message Type
			//		2) ID of the match to join
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_lobbyType) + sizeof(m_ID);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			//Match description
			memcpy(&m_ID, buffer, sizeof(m_ID));
			buffer += sizeof(m_ID);

			break;
		}
		case MATCH_JOIN_REPLY:
		{
			//Get the count of returned matches
			memcpy(&m_returnedPlayersCount, buffer, sizeof(m_returnedPlayersCount));
			buffer += sizeof(m_returnedPlayersCount);

			//Uses: 1) Message Type
			//		2) Count of players in match
			//		3) Players in match
			//		4) Description of newly created match
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_lobbyType) + sizeof(m_returnedPlayersCount) +
					(m_returnedPlayersCount * (PLAYER_DESCR_SIZE)) + MATCH_DESCR_SIZE;
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			//Allocate new space for the player descriptions
			m_playerDescriptions = (struct PlayerDescription *)malloc( m_returnedPlayersCount
					* (sizeof(struct PlayerDescription)));

			//Copy over the memory for the player descriptions
			for(uint32_t i = 0; i < m_returnedPlayersCount; i++)
			{
				memcpy(&(m_playerDescriptions[i].m_ID), buffer, sizeof(uint32_t));
				buffer += sizeof(uint32_t);
				memcpy(&(m_playerDescriptions[i].m_name), buffer, PLAYER_NAME_SIZE);
				buffer += PLAYER_NAME_SIZE;
				memcpy(&(m_playerDescriptions[i].m_team), buffer, sizeof(enum TeamNumber));
				buffer += sizeof(enum TeamNumber);
				memcpy(&(m_playerDescriptions[i].m_color), buffer, sizeof(enum TeamColor));
				buffer += sizeof(enum TeamColor);
			}

			//Match description
			memcpy(&m_matchDescription.m_status, buffer, sizeof(enum Status));
			buffer += sizeof(enum Status);
			memcpy(&m_matchDescription.m_ID, buffer, sizeof(uint32_t));
			buffer += sizeof(uint32_t);
			memcpy(&m_matchDescription.m_currentPlayerCount, buffer, sizeof(uint32_t));
			buffer += sizeof(uint32_t);
			memcpy(&m_matchDescription.m_maxPlayers, buffer, sizeof(uint32_t));
			buffer += sizeof(uint32_t);
			memcpy(&m_matchDescription.m_name, buffer, MAX_MATCHNAME_LEN);
			buffer += MAX_MATCHNAME_LEN;
			memcpy(&m_matchDescription.m_timeCreated, buffer, sizeof(int64_t));
			buffer += sizeof(int64_t);
			memcpy(&m_matchDescription.m_leaderID, buffer, sizeof(uint32_t));
			buffer += sizeof(uint32_t);

			break;
		}
		case SERVER_STATS_REQUEST:
		{
			//Uses: 1) Message Type
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_lobbyType);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			break;
		}
		case SERVER_STATS_REPLY:
		{
			//Uses: 1) Message Type
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_lobbyType) + SERVER_STATS_SIZE;
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			//Match description
			memcpy(&m_serverStats.m_numMatches, buffer, sizeof(uint32_t));
			buffer += sizeof(uint32_t);
			memcpy(&m_serverStats.m_numPlayers, buffer, sizeof(uint32_t));
			buffer += sizeof(uint32_t);

			break;
		}
		case MATCH_EXIT_SERVER_NOTIFICATION:
		{
			//Uses: 1) Message Type
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_lobbyType);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			break;
		}
		case MATCH_EXIT_SERVER_ACKNOWLEDGE:
		{
			//Uses: 1) Message Type
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_lobbyType);
			if( length != expectedSize)
			{
				m_serializeError = true;
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

char *LobbyMessage::Serialize(uint32_t *length)
{
	char *buffer, *originalBuffer;
	uint32_t messageSize;
	switch(m_lobbyType)
	{
		case MATCH_LIST_REQUEST:
		{
			//Uses: 1) Message Type
			//		2) Page number

			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize)  + sizeof(m_lobbyType)+ sizeof(m_requestedPage);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_lobbyType, sizeof(m_lobbyType));
			buffer += sizeof(m_lobbyType);
			//Page Count
			memcpy(buffer, &m_requestedPage, sizeof(m_requestedPage));
			buffer += sizeof(m_requestedPage);

			break;
		}
		case MATCH_LIST_REPLY:
		{
			//Uses: 1) Message Type
			//		2) Returned Matches Count
			//		3) Match Descriptions
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_lobbyType) + sizeof(m_returnedMatchesCount)
					+ (m_returnedMatchesCount * (MATCH_DESCR_SIZE));
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_lobbyType, sizeof(m_lobbyType));
			buffer += sizeof(m_lobbyType);

			//Put match count in
			memcpy(buffer, &m_returnedMatchesCount, sizeof(m_returnedMatchesCount));
			buffer += sizeof(m_returnedMatchesCount);

			//Put the match descriptions in
			for(uint32_t i = 0; i < m_returnedMatchesCount; i++)
			{
				//New match description
				memcpy(buffer, &m_matchDescriptions[i].m_status, sizeof(enum Status));
				buffer += sizeof(enum Status);
				memcpy(buffer, &m_matchDescriptions[i].m_ID, sizeof(uint32_t));
				buffer += sizeof(uint32_t);
				memcpy(buffer, &m_matchDescriptions[i].m_currentPlayerCount, sizeof(uint32_t));
				buffer += sizeof(uint32_t);
				memcpy(buffer, &m_matchDescriptions[i].m_maxPlayers, sizeof(uint32_t));
				buffer += sizeof(uint32_t);
				memcpy(buffer, &m_matchDescriptions[i].m_name, MAX_MATCHNAME_LEN);
				buffer += MAX_MATCHNAME_LEN;
				memcpy(buffer, &m_matchDescriptions[i].m_timeCreated, sizeof(int64_t));
				buffer += sizeof(int64_t);
				memcpy(buffer, &m_matchDescriptions[i].m_leaderID, sizeof(uint32_t));
				buffer += sizeof(uint32_t);
			}
			break;
		}
		//Messages for creating a new match
		case MATCH_CREATE_REQUEST:
		{
			//Uses: 1) Message Type
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_lobbyType);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_lobbyType, sizeof(m_lobbyType));
			buffer += sizeof(m_lobbyType);

			break;
		}
		case MATCH_CREATE_OPTIONS_AVAILABLE:
		{
			//Uses: 1) Message Type
			//		2) Match Options offered by server
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_lobbyType) + MATCH_OPTIONS_SIZE;
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_lobbyType, sizeof(m_lobbyType));
			buffer += sizeof(m_lobbyType);

			//Options
			memcpy(buffer, &m_options.m_maxPlayers, sizeof(uint32_t));
			buffer += sizeof(uint32_t);
			memcpy(buffer, &m_options.m_name, MAX_MATCHNAME_LEN);
			buffer += MAX_MATCHNAME_LEN;

			break;
		}
		case MATCH_CREATE_OPTIONS_CHOSEN:
		{
			//Uses: 1) Message Type
			//		2) Match Options chosen by client
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_lobbyType) + MATCH_OPTIONS_SIZE;
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_lobbyType, sizeof(m_lobbyType));
			buffer += sizeof(m_lobbyType);

			//options
			memcpy(buffer, &m_options.m_maxPlayers, sizeof(uint32_t));
			buffer += sizeof(uint32_t);
			memcpy(buffer, &m_options.m_name, MAX_MATCHNAME_LEN);
			buffer += MAX_MATCHNAME_LEN;

			break;
		}
		case MATCH_CREATE_REPLY:
		{
			//Uses: 1) Message Type
			//		2) Description of newly created match
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_lobbyType) + MATCH_DESCR_SIZE;
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_lobbyType, sizeof(m_lobbyType));
			buffer += sizeof(m_lobbyType);

			//New match description
			memcpy(buffer, &m_matchDescription.m_status, sizeof(enum Status));
			buffer += sizeof(enum Status);
			memcpy(buffer, &m_matchDescription.m_ID, sizeof(uint32_t));
			buffer += sizeof(uint32_t);
			memcpy(buffer, &m_matchDescription.m_currentPlayerCount, sizeof(uint32_t));
			buffer += sizeof(uint32_t);
			memcpy(buffer, &m_matchDescription.m_maxPlayers, sizeof(uint32_t));
			buffer += sizeof(uint32_t);
			memcpy(buffer, &m_matchDescription.m_name, MAX_MATCHNAME_LEN);
			buffer += MAX_MATCHNAME_LEN;
			memcpy(buffer, &m_matchDescription.m_timeCreated, sizeof(int64_t));
			buffer += sizeof(int64_t);
			memcpy(buffer, &m_matchDescription.m_leaderID, sizeof(uint32_t));
			buffer += sizeof(uint32_t);
			break;
		}
		//Joining a match already created
		case MATCH_JOIN_REQUEST:
		{
			//Uses: 1) Message Type
			//		2) ID of the match to join
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_lobbyType) + sizeof(m_ID);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_lobbyType, sizeof(m_lobbyType));
			buffer += sizeof(m_lobbyType);

			//ID of the match to join
			memcpy(buffer, &m_ID, sizeof(m_ID));
			buffer += sizeof(m_ID);

			break;
		}
		case MATCH_JOIN_REPLY:
		{
			//Uses: 1) Message Type
			//		2) Count of players in match
			//		3) Players in match
			//		4) Description of newly created match
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_lobbyType) + sizeof(m_returnedPlayersCount) +
					(m_returnedPlayersCount * (PLAYER_DESCR_SIZE)) + MATCH_DESCR_SIZE;
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_lobbyType, sizeof(m_lobbyType));
			buffer += sizeof(m_lobbyType);

			//Put the count of returned players
			memcpy(buffer, &m_returnedPlayersCount, sizeof(m_returnedPlayersCount));
			buffer += sizeof(m_returnedPlayersCount);

			//Copy over the memory for the player descriptions
			for(uint32_t i = 0; i < m_returnedPlayersCount; i++)
			{
				memcpy(buffer, &m_playerDescriptions[i].m_ID, sizeof(uint32_t));
				buffer += sizeof(uint32_t);
				memcpy(buffer, &m_playerDescriptions[i].m_name, PLAYER_NAME_SIZE);
				buffer += PLAYER_NAME_SIZE;
				memcpy(buffer, &m_playerDescriptions[i].m_team, sizeof(enum TeamNumber));
				buffer += sizeof(enum TeamNumber);
				memcpy(buffer, &m_playerDescriptions[i].m_color, sizeof(enum TeamColor));
				buffer += sizeof(enum TeamColor);
			}

			//Match Description
			memcpy(buffer, &m_matchDescription.m_status, sizeof(enum Status));
			buffer += sizeof(enum Status);
			memcpy(buffer, &m_matchDescription.m_ID, sizeof(uint32_t));
			buffer += sizeof(uint32_t);
			memcpy(buffer, &m_matchDescription.m_currentPlayerCount, sizeof(uint32_t));
			buffer += sizeof(uint32_t);
			memcpy(buffer, &m_matchDescription.m_maxPlayers, sizeof(uint32_t));
			buffer += sizeof(uint32_t);
			memcpy(buffer, &m_matchDescription.m_name, MAX_MATCHNAME_LEN);
			buffer += MAX_MATCHNAME_LEN;
			memcpy(buffer, &m_matchDescription.m_timeCreated, sizeof(int64_t));
			buffer += sizeof(int64_t);
			memcpy(buffer, &m_matchDescription.m_leaderID, sizeof(uint32_t));
			buffer += sizeof(uint32_t);

			break;
		}
		case SERVER_STATS_REQUEST:
		{
			//Uses: 1) Message Type
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_lobbyType);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_lobbyType, sizeof(m_lobbyType));
			buffer += sizeof(m_lobbyType);

			break;
		}
		case SERVER_STATS_REPLY:
		{
			//Uses: 1) Message Type
			//		2) Server stats
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_lobbyType) + SERVER_STATS_SIZE;
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_lobbyType, sizeof(m_lobbyType));
			buffer += sizeof(m_lobbyType);

			//Put the stats struct in
			memcpy(buffer, &m_serverStats.m_numMatches, sizeof(uint32_t));
			buffer += sizeof(uint32_t);
			memcpy(buffer, &m_serverStats.m_numPlayers, sizeof(uint32_t));
			buffer += sizeof(uint32_t);

			break;
		}
		//Leaving a match you're in
		case MATCH_EXIT_SERVER_NOTIFICATION:
		{
			//Uses: 1) Message Type
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_lobbyType);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			memcpy(buffer, &m_lobbyType, sizeof(m_lobbyType));
			buffer += sizeof(m_lobbyType);

			break;
		}
		case MATCH_EXIT_SERVER_ACKNOWLEDGE:
		{
			//Uses: 1) Message Type
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_lobbyType);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_lobbyType, sizeof(m_lobbyType));
			buffer += sizeof(m_lobbyType);

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

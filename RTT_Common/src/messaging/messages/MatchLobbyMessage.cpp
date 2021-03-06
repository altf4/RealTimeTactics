//============================================================================
// Name        : MatchLobbyMessage.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Message class pertaining to the Match Lobby (not main lobby)
//============================================================================

#include "MatchLobbyMessage.h"
#include "string.h"

using namespace std;
using namespace RTT;

MatchLobbyMessage::~MatchLobbyMessage()
{

}

MatchLobbyMessage::MatchLobbyMessage(enum MatchLobbyType type)
{
	m_messageType = MESSAGE_MATCH_LOBBY;
	m_matchLobbyType = type;
}

MatchLobbyMessage::MatchLobbyMessage(char *buffer, uint32_t length)
{
	if( length < MESSAGE_HDR_SIZE )
	{
		return;
	}

	m_serializeError = false;

	DeserializeHeader(&buffer);

	memcpy(&m_matchLobbyType, buffer, sizeof(m_matchLobbyType));
	buffer += sizeof(m_matchLobbyType);

	switch(m_matchLobbyType)
	{
		//*********************
		//	MatchLobbyMessage
		// (Initiated by client)
		//*********************
		//Leaving a match you're in
		case MATCH_LEAVE_NOTIFICATION:
		{
			//Uses: 1) Message Type
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_matchLobbyType);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			break;
		}
		case MATCH_LEAVE_ACKNOWLEDGE:
		{
			//Uses: 1) Message Type
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_matchLobbyType);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			break;
		}
		case CHANGE_TEAM_REQUEST:
		{
			//Uses: 1) Message Type
			//		2) New team
			//		3) Player ID to change
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_matchLobbyType) + sizeof(m_newTeam) + sizeof(m_playerID);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			//new team
			memcpy(&m_newTeam, buffer, sizeof(m_newTeam));
			buffer += sizeof(m_newTeam);
			//new team
			memcpy(&m_playerID, buffer, sizeof(m_playerID));
			buffer += sizeof(m_playerID);

			break;
		}
		case CHANGE_TEAM_REPLY:
		{
			//Uses: 1) Message Type
			//		2) Change Accepted
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_matchLobbyType) + sizeof(m_changeAccepted);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			//new team
			memcpy(&m_changeAccepted, buffer, sizeof(m_changeAccepted));
			buffer += sizeof(m_changeAccepted);

			break;
		}
		case START_MATCH_REQUEST:
		{
			//Uses: 1) Message Type
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_matchLobbyType);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			break;
		}
		case START_MATCH_REPLY:
		{
			//Uses: 1) Message Type
			//		2) Change Accepted
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_matchLobbyType) + sizeof(m_changeAccepted);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			//new team
			memcpy(&m_changeAccepted, buffer, sizeof(m_changeAccepted));
			buffer += sizeof(m_changeAccepted);

			break;
		}
		case CHANGE_COLOR_REQUEST:
		{
			//Uses: 1) Message Type
			//		2) New Color
			//		3) Player ID
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_matchLobbyType) + sizeof(m_newColor) + sizeof(m_playerID);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			//new team
			memcpy(&m_newColor, buffer, sizeof(m_newColor));
			buffer += sizeof(m_newColor);
			//Player to change
			memcpy(&m_playerID, buffer, sizeof(m_playerID));
			buffer += sizeof(m_playerID);

			break;
		}
		case CHANGE_COLOR_REPLY:
		{
			//Uses: 1) Message Type
			//		2) Change Accepted
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_matchLobbyType) + sizeof(m_changeAccepted);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			//new team
			memcpy(&m_changeAccepted, buffer, sizeof(m_changeAccepted));
			buffer += sizeof(m_changeAccepted);

			break;
		}
		case CHANGE_MAP_REQUEST:
		{
			//Uses: 1) Message Type
			//		2) New map data
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_matchLobbyType) + MAP_DESCR_SIZE;
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			//new map
			memcpy(&m_mapDescription.m_name, buffer, MAP_NAME_LEN);
			buffer += MAP_NAME_LEN;
			memcpy(&m_mapDescription.m_length, buffer, sizeof(uint32_t));
			buffer += sizeof(uint32_t);
			memcpy(&m_mapDescription.m_width, buffer, sizeof(uint32_t));
			buffer += sizeof(uint32_t);
			break;
		}
		case CHANGE_MAP_REPLY:
		{
			//Uses: 1) Message Type
			//		2) Change Accepted
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_matchLobbyType) + sizeof(m_changeAccepted);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			//new team
			memcpy(&m_changeAccepted, buffer, sizeof(m_changeAccepted));
			buffer += sizeof(m_changeAccepted);

			break;
		}
		case CHANGE_VICTORY_COND_REQUEST:
		{
			//Uses: 1) Message Type
			//		2) New map data
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_matchLobbyType) + sizeof(m_newVictCond);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			//new team
			memcpy(&m_newVictCond, buffer, sizeof(m_newVictCond));
			buffer += sizeof(m_newVictCond);

			break;
		}
		case CHANGE_VICTORY_COND_REPLY:
		{
			//Uses: 1) Message Type
			//		2) Change Accepted
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_matchLobbyType) + sizeof(m_changeAccepted);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			//new team
			memcpy(&m_changeAccepted, buffer, sizeof(m_changeAccepted));
			buffer += sizeof(m_changeAccepted);

			break;
		}
		case CHANGE_GAME_SPEED_REQUEST:
		{
			//Uses: 1) Message Type
			//		2) New map data
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_matchLobbyType) + sizeof(m_newSpeed);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			//new team
			memcpy(&m_newSpeed, buffer, sizeof(m_newSpeed));
			buffer += sizeof(m_newSpeed);

			break;
		}
		case CHANGE_GAME_SPEED_REPLY:
		{
			//Uses: 1) Message Type
			//		2) Change Accepted
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_matchLobbyType) + sizeof(m_changeAccepted);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			//new team
			memcpy(&m_changeAccepted, buffer, sizeof(m_changeAccepted));
			buffer += sizeof(m_changeAccepted);

			break;
		}
		case CALLBACK_REGISTER:
		{
			//Uses: 1) Message Type
			//		2) playerID
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_matchLobbyType) + sizeof(m_playerID);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			//PlayerID
			memcpy(&m_playerID, buffer, sizeof(m_playerID));
			buffer += sizeof(m_playerID);

			break;
		}
		case CHANGE_LEADER_REQUEST:
		{
			//Uses: 1) Message Type
			//		2) playerID
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_matchLobbyType) + sizeof(m_playerID);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			//PlayerID
			memcpy(&m_playerID, buffer, sizeof(m_playerID));
			buffer += sizeof(m_playerID);

			break;
		}
		case CHANGE_LEADER_REPLY:
		{
			//Uses: 1) Message Type
			//		2) Success or failure (bool)
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_matchLobbyType) + sizeof(m_changeAccepted);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			//PlayerID
			memcpy(&m_changeAccepted, buffer, sizeof(m_changeAccepted));
			buffer += sizeof(m_changeAccepted);

			break;
		}
		case KICK_PLAYER_REQUEST:
		{
			//Uses: 1) Message Type
			//		2) Player ID
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_matchLobbyType) + sizeof(m_playerID);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			//new team
			memcpy(&m_playerID, buffer, sizeof(m_playerID));
			buffer += sizeof(m_playerID);

			break;
		}
		case KICK_PLAYER_REPLY:
		{
			//Uses: 1) Message Type
			//		2) Change Accepted
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_matchLobbyType) + sizeof(m_changeAccepted);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			//new team
			memcpy(&m_changeAccepted, buffer, sizeof(m_changeAccepted));
			buffer += sizeof(m_changeAccepted);

			break;
		}

		//************************
		//	MatchLobbyMessage
		// (Initiated by server)
		//************************
		case TEAM_CHANGED_NOTIFICATION:
		{
			//Uses: 1) Message Type
			//		2) New team
			//		3) Player ID
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_matchLobbyType) + sizeof(m_newTeam) + sizeof(m_playerID);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			//New team
			memcpy(&m_newTeam, buffer, sizeof(m_newTeam));
			buffer += sizeof(m_newTeam);
			//Player ID that changed
			memcpy(&m_playerID, buffer, sizeof(m_playerID));
			buffer += sizeof(m_playerID);

			break;
		}
		case TEAM_CHANGED_ACK:
		{
			//Uses: 1) Message Type
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_matchLobbyType);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			break;
		}
		case KICKED_FROM_MATCH_NOTIFICATION:
		{
			//Uses: 1) Message Type
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_matchLobbyType);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			break;
		}
		case KICKED_FROM_MATCH_ACK:
		{
			//Uses: 1) Message Type
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_matchLobbyType);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			break;
		}
		case PLAYER_LEFT_MATCH_NOTIFICATION:
		{
			//Uses: 1) Message Type
			//		2) Player ID who left
			//		3) Player ID of new leader
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_matchLobbyType) + sizeof(m_playerID) +
					sizeof(m_newLeaderID);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			//Player ID that left
			memcpy(&m_playerID, buffer, sizeof(m_playerID));
			buffer += sizeof(m_playerID);

			//Player ID of new leader
			memcpy(&m_newLeaderID, buffer, sizeof(m_newLeaderID));
			buffer += sizeof(m_newLeaderID);

			break;
		}
		case PLAYER_LEFT_MATCH_ACK:
		{
			//Uses: 1) Message Type
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_matchLobbyType);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			break;
		}
		case PLAYER_JOINED_MATCH_NOTIFICATION:
		{
			//Uses: 1) Message Type
			//		2) Player Description
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_matchLobbyType) + PLAYER_DESCR_SIZE;
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			//Player ID that joined
			memcpy(&m_playerDescription.m_name, buffer, PLAYER_NAME_SIZE);
			buffer += PLAYER_NAME_SIZE;
			memcpy(&m_playerDescription.m_ID, buffer, sizeof(uint32_t));
			buffer += sizeof(uint32_t);
			memcpy(&m_playerDescription.m_color, buffer, sizeof(enum TeamColor));
			buffer += sizeof(enum TeamColor);
			memcpy(&m_playerDescription.m_team, buffer, sizeof(enum TeamNumber));
			buffer += sizeof(enum TeamNumber);


			break;
		}
		case PLAYER_JOINED_MATCH_ACK:
		{
			//Uses: 1) Message Type
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_matchLobbyType);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			break;
		}
		case COLOR_CHANGED_NOTIFICATION:
		{
			//Uses: 1) Message Type
			//		2) Player ID
			//		3) New color
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_matchLobbyType) + sizeof(m_playerID) + sizeof(m_newColor);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			//Player ID
			memcpy(&m_playerID, buffer, sizeof(m_playerID));
			buffer += sizeof(m_playerID);
			//New Color
			memcpy(&m_newColor, buffer, sizeof(m_newColor));
			buffer += sizeof(m_newColor);

			break;
		}
		case COLOR_CHANGED_ACK:
		{
			//Uses: 1) Message Type
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_matchLobbyType);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			break;
		}
		case MAP_CHANGED_NOTIFICATION:
		{
			//Uses: 1) Message Type
			//		2) Map Description
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_matchLobbyType) + MAP_DESCR_SIZE;
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			//Map description
			memcpy(&m_mapDescription.m_name, buffer, MAP_NAME_LEN);
			buffer += MAP_NAME_LEN;
			memcpy(&m_mapDescription.m_length, buffer, sizeof(uint32_t));
			buffer += sizeof(uint32_t);
			memcpy(&m_mapDescription.m_width, buffer, sizeof(uint32_t));
			buffer += sizeof(uint32_t);

			break;
		}
		case MAP_CHANGED_ACK:
		{
			//Uses: 1) Message Type
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_matchLobbyType);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			break;
		}
		case GAME_SPEED_CHANGED_NOTIFICATION:
		{
			//Uses: 1) Message Type
			//		2) Game speed
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_matchLobbyType) + sizeof(m_newSpeed);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			//Map description
			memcpy(&m_newSpeed, buffer, sizeof(m_newSpeed));
			buffer += sizeof(m_newSpeed);

			break;
		}
		case GAME_SPEED_CHANGED_ACK:
		{
			//Uses: 1) Message Type
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_matchLobbyType);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			break;
		}
		case VICTORY_COND_CHANGED_NOTIFICATION:
		{
			//Uses: 1) Message Type
			//		2) Victory Condition
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_matchLobbyType) + sizeof(m_newVictCond);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			//Victory condition
			memcpy(&m_newVictCond, buffer, sizeof(m_newVictCond));
			buffer += sizeof(m_newVictCond);

			break;
		}
		case VICTORY_COND_CHANGED_ACK:
		{
			//Uses: 1) Message Type
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_matchLobbyType);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			break;
		}
		case CHANGE_LEADER_NOTIFICATION:
		{
			//Uses: 1) Message Type
			//		2) PlayerID of new leader
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_matchLobbyType) + sizeof(m_playerID);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			//Victory condition
			memcpy(&m_playerID, buffer, sizeof(m_playerID));
			buffer += sizeof(m_playerID);

			break;
		}
		case CHANGE_LEADER_ACK:
		{
			//Uses: 1) Message Type
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_matchLobbyType);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			break;
		}
		case MATCH_START_NOTIFICATION:
		{
			//Uses: 1) Message Type
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_matchLobbyType);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			break;
		}
		case MATCH_START_ACK:
		{
			//Uses: 1) Message Type
			//		2) Change Accepted
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_matchLobbyType) + sizeof(m_changeAccepted);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			//Map description
			memcpy(&m_changeAccepted, buffer, sizeof(m_changeAccepted));
			buffer += sizeof(m_changeAccepted);

			break;
		}


		default:
		{
			//Error
			return;
		}
	}
}


char *MatchLobbyMessage::Serialize(uint32_t *length)
{
	char *buffer, *originalBuffer;
	uint32_t messageSize;
	switch(m_matchLobbyType)
	{
		//*********************
		//	MatchLobbyMessage
		// (Initiated by client)
		//*********************
		//Leaving a match you're in
		case MATCH_LEAVE_NOTIFICATION:
		{
			//Uses: 1) Message Type
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_matchLobbyType);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			memcpy(buffer, &m_matchLobbyType, sizeof(m_matchLobbyType));
			buffer += sizeof(m_matchLobbyType);

			break;
		}
		case MATCH_LEAVE_ACKNOWLEDGE:
		{
			//Uses: 1) Message Type
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_matchLobbyType);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_matchLobbyType, sizeof(m_matchLobbyType));
			buffer += sizeof(m_matchLobbyType);

			break;
		}
		case CHANGE_TEAM_REQUEST:
		{
			//Uses: 1) Message Type
			//		2) New team
			//		3) Player ID to change
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_matchLobbyType) + sizeof(m_newTeam) + sizeof(m_playerID);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_matchLobbyType, sizeof(m_matchLobbyType));
			buffer += sizeof(m_matchLobbyType);
			//Page Count
			memcpy(buffer, &m_newTeam, sizeof(m_newTeam));
			buffer += sizeof(m_newTeam);
			//PlayerID
			memcpy(buffer, &m_playerID, sizeof(m_playerID));
			buffer += sizeof(m_playerID);

			break;
		}
		case CHANGE_TEAM_REPLY:
		{
			//Uses: 1) Message Type
			//		2) Change Accepted
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_matchLobbyType) + sizeof(m_changeAccepted);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_matchLobbyType, sizeof(m_matchLobbyType));
			buffer += sizeof(m_matchLobbyType);
			//Change accepted
			memcpy(buffer, &m_changeAccepted, sizeof(m_changeAccepted));
			buffer += sizeof(m_changeAccepted);

			break;
		}
		case START_MATCH_REQUEST:
		{
			//Uses: 1) Message Type
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_matchLobbyType);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_matchLobbyType, sizeof(m_matchLobbyType));
			buffer += sizeof(m_matchLobbyType);

			break;
		}
		case START_MATCH_REPLY:
		{
			//Uses: 1) Message Type
			//		2) Change Accepted
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_matchLobbyType) + sizeof(m_changeAccepted);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_matchLobbyType, sizeof(m_matchLobbyType));
			buffer += sizeof(m_matchLobbyType);
			//Change accepted
			memcpy(buffer, &m_changeAccepted, sizeof(m_changeAccepted));
			buffer += sizeof(m_changeAccepted);

			break;
		}
		case CHANGE_COLOR_REQUEST:
		{
			//Uses: 1) Message Type
			//		2) new Color
			//		3) Player ID
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_matchLobbyType) + sizeof(m_newColor) + sizeof(m_playerID);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_matchLobbyType, sizeof(m_matchLobbyType));
			buffer += sizeof(m_matchLobbyType);
			//New color
			memcpy(buffer, &m_newColor, sizeof(m_newColor));
			buffer += sizeof(m_newColor);
			//Player ID
			memcpy(buffer, &m_playerID, sizeof(m_playerID));
			buffer += sizeof(m_playerID);

			break;
		}
		case CHANGE_COLOR_REPLY:
		{
			//Uses: 1) Message Type
			//		2) Change Accepted
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_matchLobbyType) + sizeof(m_changeAccepted);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_matchLobbyType, sizeof(m_matchLobbyType));
			buffer += sizeof(m_matchLobbyType);
			//Change accepted
			memcpy(buffer, &m_changeAccepted, sizeof(m_changeAccepted));
			buffer += sizeof(m_changeAccepted);

			break;
		}
		case CHANGE_MAP_REQUEST:
		{
			//Uses: 1) Message Type
			//		2) New Map
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_matchLobbyType) + MAP_DESCR_SIZE;
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_matchLobbyType, sizeof(m_matchLobbyType));
			buffer += sizeof(m_matchLobbyType);

			//New color
			memcpy(buffer, &m_mapDescription.m_name, MAP_NAME_LEN);
			buffer += MAP_NAME_LEN;
			memcpy(buffer, &m_mapDescription.m_length, sizeof(uint32_t));
			buffer += sizeof(uint32_t);
			memcpy(buffer, &m_mapDescription.m_width, sizeof(uint32_t));
			buffer += sizeof(uint32_t);

			break;
		}
		case CHANGE_MAP_REPLY:
		{
			//Uses: 1) Message Type
			//		2) Change Accepted
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_matchLobbyType) + sizeof(m_changeAccepted);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_matchLobbyType, sizeof(m_matchLobbyType));
			buffer += sizeof(m_matchLobbyType);
			//Change accepted
			memcpy(buffer, &m_changeAccepted, sizeof(m_changeAccepted));
			buffer += sizeof(m_changeAccepted);

			break;
		}
		case CHANGE_VICTORY_COND_REQUEST:
		{
			//Uses: 1) Message Type
			//		2) New victory condition
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_matchLobbyType) + sizeof(m_newVictCond);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_matchLobbyType, sizeof(m_matchLobbyType));
			buffer += sizeof(m_matchLobbyType);
			//New color
			memcpy(buffer, &m_newVictCond, sizeof(m_newVictCond));
			buffer += sizeof(m_newVictCond);

			break;
		}
		case CHANGE_VICTORY_COND_REPLY:
		{
			//Uses: 1) Message Type
			//		2) Change Accepted
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_matchLobbyType) + sizeof(m_changeAccepted);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_matchLobbyType, sizeof(m_matchLobbyType));
			buffer += sizeof(m_matchLobbyType);
			//Change accepted
			memcpy(buffer, &m_changeAccepted, sizeof(m_changeAccepted));
			buffer += sizeof(m_changeAccepted);

			break;
		}
		case CHANGE_LEADER_REQUEST:
		{
			//Uses: 1) Message Type
			//		2) playerID of new leader
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_matchLobbyType) + sizeof(m_playerID);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_matchLobbyType, sizeof(m_matchLobbyType));
			buffer += sizeof(m_matchLobbyType);
			//New color
			memcpy(buffer, &m_playerID, sizeof(m_playerID));
			buffer += sizeof(m_playerID);

			break;
		}
		case CHANGE_LEADER_REPLY:
		{
			//Uses: 1) Message Type
			//		2) Change Accepted
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_matchLobbyType) + sizeof(m_changeAccepted);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_matchLobbyType, sizeof(m_matchLobbyType));
			buffer += sizeof(m_matchLobbyType);
			//Change accepted
			memcpy(buffer, &m_changeAccepted, sizeof(m_changeAccepted));
			buffer += sizeof(m_changeAccepted);

			break;
		}
		case CHANGE_GAME_SPEED_REQUEST:
		{
			//Uses: 1) Message Type
			//		2) New Game Speed
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_matchLobbyType) + sizeof(m_newSpeed);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_matchLobbyType, sizeof(m_matchLobbyType));
			buffer += sizeof(m_matchLobbyType);
			//New color
			memcpy(buffer, &m_newSpeed, sizeof(m_newSpeed));
			buffer += sizeof(m_newSpeed);

			break;
		}
		case CHANGE_GAME_SPEED_REPLY:
		{
			//Uses: 1) Message Type
			//		2) Change Accepted
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_matchLobbyType) + sizeof(m_changeAccepted);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_matchLobbyType, sizeof(m_matchLobbyType));
			buffer += sizeof(m_matchLobbyType);
			//Change accepted
			memcpy(buffer, &m_changeAccepted, sizeof(m_changeAccepted));
			buffer += sizeof(m_changeAccepted);

			break;
		}
		case CALLBACK_REGISTER:
		{
			//Uses: 1) Message Type
			//		2) PlayerID
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_matchLobbyType) + sizeof(m_playerID);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_matchLobbyType, sizeof(m_matchLobbyType));
			buffer += sizeof(m_matchLobbyType);
			//New color
			memcpy(buffer, &m_playerID, sizeof(m_playerID));
			buffer += sizeof(m_playerID);

			break;
		}
		case KICK_PLAYER_REQUEST:
		{
			//Uses: 1) Message Type
			//		2) Player ID
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_matchLobbyType) + sizeof(m_playerID);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_matchLobbyType, sizeof(m_matchLobbyType));
			buffer += sizeof(m_matchLobbyType);
			//New color
			memcpy(buffer, &m_playerID, sizeof(m_playerID));
			buffer += sizeof(m_playerID);

			break;
		}
		case KICK_PLAYER_REPLY:
		{
			//Uses: 1) Message Type
			//		2) Change Accepted
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_matchLobbyType) + sizeof(m_changeAccepted);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_matchLobbyType, sizeof(m_matchLobbyType));
			buffer += sizeof(m_matchLobbyType);
			//Change accepted
			memcpy(buffer, &m_changeAccepted, sizeof(m_changeAccepted));
			buffer += sizeof(m_changeAccepted);

			break;
		}

		//************************
		//	MatchLobbyMessage
		// (Initiated by server)
		//************************
		case TEAM_CHANGED_NOTIFICATION:
		{
			//Uses: 1) Message Type
			//		2) New team
			//		3) Player ID
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_matchLobbyType) + sizeof(m_newTeam) + sizeof(m_playerID);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_matchLobbyType, sizeof(m_matchLobbyType));
			buffer += sizeof(m_matchLobbyType);
			//New Team
			memcpy(buffer, &m_newTeam, sizeof(m_newTeam));
			buffer += sizeof(m_newTeam);
			//Player ID
			memcpy(buffer, &m_playerID, sizeof(m_playerID));
			buffer += sizeof(m_playerID);

			break;
		}
		case TEAM_CHANGED_ACK:
		{
			//Uses: 1) Message Type
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_matchLobbyType);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_matchLobbyType, sizeof(m_matchLobbyType));
			buffer += sizeof(m_matchLobbyType);

			break;
		}
		case KICKED_FROM_MATCH_NOTIFICATION:
		{
			//Uses: 1) Message Type
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_matchLobbyType);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_matchLobbyType, sizeof(m_matchLobbyType));
			buffer += sizeof(m_matchLobbyType);

			break;
		}
		case KICKED_FROM_MATCH_ACK:
		{
			//Uses: 1) Message Type
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_matchLobbyType);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_matchLobbyType, sizeof(m_matchLobbyType));
			buffer += sizeof(m_matchLobbyType);

			break;
		}
		case PLAYER_LEFT_MATCH_NOTIFICATION:
		{
			//Uses: 1) Message Type
			//		2) Player ID who left
			//		3) Player ID of new leader
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_matchLobbyType) + sizeof(m_playerID) + sizeof(m_newLeaderID);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_matchLobbyType, sizeof(m_matchLobbyType));
			buffer += sizeof(m_matchLobbyType);
			//Player ID who left
			memcpy(buffer, &m_playerID, sizeof(m_playerID));
			buffer += sizeof(m_playerID);
			//Player ID of new leader
			memcpy(buffer, &m_newLeaderID, sizeof(m_newLeaderID));
			buffer += sizeof(m_newLeaderID);

			break;
		}
		case PLAYER_LEFT_MATCH_ACK:
		{
			//Uses: 1) Message Type
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_matchLobbyType);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_matchLobbyType, sizeof(m_matchLobbyType));
			buffer += sizeof(m_matchLobbyType);

			break;
		}
		case PLAYER_JOINED_MATCH_NOTIFICATION:
		{
			//Uses: 1) Message Type
			//		2) Player Description
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_matchLobbyType) + PLAYER_DESCR_SIZE;
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_matchLobbyType, sizeof(m_matchLobbyType));
			buffer += sizeof(m_matchLobbyType);

			//Put the player description in
			memcpy(buffer, &m_playerDescription.m_name, PLAYER_NAME_SIZE);
			buffer += PLAYER_NAME_SIZE;
			memcpy(buffer, &m_playerDescription.m_ID, sizeof(uint32_t));
			buffer += sizeof(uint32_t);
			memcpy(buffer, &m_playerDescription.m_color, sizeof(enum TeamColor));
			buffer += sizeof(enum TeamColor);
			memcpy(buffer, &m_playerDescription.m_team, sizeof(enum TeamNumber));
			buffer += sizeof(enum TeamNumber);

			break;
		}
		case PLAYER_JOINED_MATCH_ACK:
		{
			//Uses: 1) Message Type
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_matchLobbyType);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_matchLobbyType, sizeof(m_matchLobbyType));
			buffer += sizeof(m_matchLobbyType);

			break;
		}
		case COLOR_CHANGED_NOTIFICATION:
		{
			//Uses: 1) Message Type
			//		2) Player ID
			//		3) New Color
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_matchLobbyType) + sizeof(m_playerID) + sizeof(m_newColor);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_matchLobbyType, sizeof(m_matchLobbyType));
			buffer += sizeof(m_matchLobbyType);
			//Player ID
			memcpy(buffer, &m_playerID, sizeof(m_playerID));
			buffer += sizeof(m_playerID);
			//Player ID
			memcpy(buffer, &m_newColor, sizeof(m_newColor));
			buffer += sizeof(m_newColor);

			break;
		}
		case COLOR_CHANGED_ACK:
		{
			//Uses: 1) Message Type
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_matchLobbyType);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_matchLobbyType, sizeof(m_matchLobbyType));
			buffer += sizeof(m_matchLobbyType);

			break;
		}
		case MAP_CHANGED_NOTIFICATION:
		{
			//Uses: 1) Message Type
			//		2) Map Description
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_matchLobbyType) + MAP_DESCR_SIZE;
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_matchLobbyType, sizeof(m_matchLobbyType));
			buffer += sizeof(m_matchLobbyType);

			//Put the map description in
			memcpy(buffer, &m_mapDescription.m_name, MAP_NAME_LEN);
			buffer += MAP_NAME_LEN;
			memcpy(buffer, &m_mapDescription.m_length, sizeof(uint32_t));
			buffer += sizeof(uint32_t);
			memcpy(buffer, &m_mapDescription.m_width, sizeof(uint32_t));
			buffer += sizeof(uint32_t);
			break;
		}
		case MAP_CHANGED_ACK:
		{
			//Uses: 1) Message Type
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_matchLobbyType);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_matchLobbyType, sizeof(m_matchLobbyType));
			buffer += sizeof(m_matchLobbyType);

			break;
		}
		case GAME_SPEED_CHANGED_NOTIFICATION:
		{
			//Uses: 1) Message Type
			//		2) Game Speed
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_matchLobbyType) + sizeof(m_newSpeed);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_matchLobbyType, sizeof(m_matchLobbyType));
			buffer += sizeof(m_matchLobbyType);
			//Put the type in
			memcpy(buffer, &m_newSpeed, sizeof(m_newSpeed));
			buffer += sizeof(m_newSpeed);

			break;
		}
		case GAME_SPEED_CHANGED_ACK:
		{
			//Uses: 1) Message Type
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_matchLobbyType);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_matchLobbyType, sizeof(m_matchLobbyType));
			buffer += sizeof(m_matchLobbyType);

			break;
		}
		case VICTORY_COND_CHANGED_NOTIFICATION:
		{
			//Uses: 1) Message Type
			//		2) Victory Condition
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_matchLobbyType) + sizeof(m_newVictCond);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_matchLobbyType, sizeof(m_matchLobbyType));
			buffer += sizeof(m_matchLobbyType);
			//PVictory Condition
			memcpy(buffer, &m_newVictCond, sizeof(m_newVictCond));
			buffer += sizeof(m_newVictCond);

			break;
		}
		case VICTORY_COND_CHANGED_ACK:
		{
			//Uses: 1) Message Type
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_matchLobbyType);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_matchLobbyType, sizeof(m_matchLobbyType));
			buffer += sizeof(m_matchLobbyType);

			break;
		}
		case CHANGE_LEADER_NOTIFICATION:
		{
			//Uses: 1) Message Type
			//		2) PlayerID of new leader
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_matchLobbyType) + sizeof(m_playerID);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_matchLobbyType, sizeof(m_matchLobbyType));
			buffer += sizeof(m_matchLobbyType);
			//PVictory Condition
			memcpy(buffer, &m_playerID, sizeof(m_playerID));
			buffer += sizeof(m_playerID);

			break;
		}
		case CHANGE_LEADER_ACK:
		{
			//Uses: 1) Message Type
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_matchLobbyType);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_matchLobbyType, sizeof(m_matchLobbyType));
			buffer += sizeof(m_matchLobbyType);

			break;
		}
		case MATCH_START_NOTIFICATION:
		{
			//Uses: 1) Message Type
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_matchLobbyType);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_matchLobbyType, sizeof(m_matchLobbyType));
			buffer += sizeof(m_matchLobbyType);

			break;
		}
		case MATCH_START_ACK:
		{
			//Uses: 1) Message Type
			//		2) Change Accepted
			messageSize = MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_matchLobbyType) + sizeof(m_changeAccepted);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//Put the type in
			memcpy(buffer, &m_matchLobbyType, sizeof(m_matchLobbyType));
			buffer += sizeof(m_matchLobbyType);
			//Accept Changes
			memcpy(buffer, &m_changeAccepted, sizeof(m_changeAccepted));
			buffer += sizeof(m_changeAccepted);

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

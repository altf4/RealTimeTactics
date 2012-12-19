//============================================================================
// Name        : GameMessage.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Message class for in-game actions
//============================================================================

#include "GameMessage.h"
#include "string.h"

using namespace std;
using namespace RTT;

GameMessage::GameMessage(enum GameMessageType type)
{
	m_messageType = MESSAGE_GAME;
	m_gameMessageType = type;
}

GameMessage::GameMessage(char *buffer, uint32_t length)
{
	if( length < MESSAGE_HDR_SIZE )
	{
		return;
	}

	m_serializeError = false;

	//Deserialize the UI_Message header
	if(!DeserializeHeader(&buffer))
	{
		m_serializeError = true;
		return;
	}

	//Copy the message type
	memcpy(&m_gameMessageType, buffer, sizeof(m_gameMessageType));
	buffer += sizeof(m_gameMessageType);

	switch(m_gameMessageType)
	{
		case MOVE_UNIT_DIRECTION_REQUEST:
		{
			//Uses: 1) Message Type
			//		2) Unit ID to move
			//		3) X coordinate, starting location
			//		4) Y coordinate, starting location
			//		5) Direction to move unit (one hop)
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_gameMessageType) + sizeof(m_unitID) +
					sizeof(m_xOld) + sizeof(m_yOld) + sizeof(m_unitDirection);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			//Unit ID
			memcpy(&m_unitID, buffer, sizeof(m_unitID));
			buffer += sizeof(m_unitID);
			//Starting X position
			memcpy(&m_xOld, buffer, sizeof(m_xOld));
			buffer += sizeof(m_xOld);
			//Starting Y position
			memcpy(&m_yOld, buffer, sizeof(m_yOld));
			buffer += sizeof(m_yOld);
			//Direction (enum) to move
			memcpy(&m_unitDirection, buffer, sizeof(m_unitDirection));
			buffer += sizeof(m_unitDirection);

			break;
		}
		case MOVE_UNIT_DIRECTION_REPLY:
		{
			//Uses: 1) Message Type
			//		2) MoveResult enum describing success or failure
			//		3) X coordinate, ending location
			//		4) Y coordinate, ending location
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_gameMessageType) + sizeof(m_moveResult)
					+ sizeof(m_xOld) + sizeof(m_yOld);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			//Unit ID
			memcpy(&m_unitID, buffer, sizeof(m_unitID));
			buffer += sizeof(m_unitID);
			//Movement Result enumeration
			memcpy(&m_moveResult, buffer, sizeof(m_moveResult));
			buffer += sizeof(m_moveResult);
			//Starting X position
			memcpy(&m_xOld, buffer, sizeof(m_xOld));
			buffer += sizeof(m_xOld);
			//Starting Y position
			memcpy(&m_yOld, buffer, sizeof(m_yOld));
			buffer += sizeof(m_yOld);
			break;
		}
		case MOVE_UNIT_DISTANT_REQUEST:
		{
			//Uses: 1) Message Type
			//		2) Unit ID that moved
			//		3) X coordinate, starting location
			//		4) Y coordinate, starting location
			//		5) X coordinate, ending location
			//		6) Y coordinate, ending location
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_gameMessageType) + sizeof(m_unitID) + sizeof(m_xOld) + sizeof(m_yOld)
					+ sizeof(m_xNew) + sizeof(m_yNew);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			//Unit ID
			memcpy(&m_unitID, buffer, sizeof(m_unitID));
			buffer += sizeof(m_unitID);
			//Starting X position
			memcpy(&m_xOld, buffer, sizeof(m_xOld));
			buffer += sizeof(m_xOld);
			//Starting Y position
			memcpy(&m_yOld, buffer, sizeof(m_yOld));
			buffer += sizeof(m_yOld);
			//Ending X position
			memcpy(&m_xNew, buffer, sizeof(m_xNew));
			buffer += sizeof(m_xNew);
			//Ending Y position
			memcpy(&m_yNew, buffer, sizeof(m_yNew));
			buffer += sizeof(m_yNew);
			break;
		}
		case MOVE_UNIT_DISTANT_REPLY:
		{
			//Uses: 1) Message Type
			//		2) MoveResult enum describing success or failure
			//		3) Unit ID
			//		4) X coordinate, ending location
			//		5) Y coordinate, ending location
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_gameMessageType) + sizeof(m_unitID) + sizeof(m_moveResult)
					+ sizeof(m_xNew) + sizeof(m_yNew);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			//Unit ID
			memcpy(&m_unitID, buffer, sizeof(m_unitID));
			buffer += sizeof(m_unitID);
			//Movement Result enumeration
			memcpy(&m_moveResult, buffer, sizeof(m_moveResult));
			buffer += sizeof(m_moveResult);
			//Starting X position
			memcpy(&m_xNew, buffer, sizeof(m_xNew));
			buffer += sizeof(m_xNew);
			//Starting Y position
			memcpy(&m_yNew, buffer, sizeof(m_yNew));
			buffer += sizeof(m_yNew);
			break;
		}
		case UNIT_MOVED_DIRECTION_NOTICE:
		{
			//Uses: 1) Message Type
			//		2) Unit ID that moved
			//		3) X coordinate, starting location
			//		4) Y coordinate, starting location
			//		5) Direction unit moved (one hop)
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_gameMessageType) + sizeof(m_unitID)
					+ sizeof(m_xOld) + sizeof(m_yOld) + sizeof(m_unitDirection);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			//Unit ID
			memcpy(&m_unitID, buffer, sizeof(m_unitID));
			buffer += sizeof(m_unitID);
			//Starting X position
			memcpy(&m_xOld, buffer, sizeof(m_xOld));
			buffer += sizeof(m_xOld);
			//Starting Y position
			memcpy(&m_yOld, buffer, sizeof(m_yOld));
			buffer += sizeof(m_yOld);
			//Direction (enum) moved
			memcpy(&m_unitDirection, buffer, sizeof(m_unitDirection));
			buffer += sizeof(m_unitDirection);

			break;
		}
		case UNIT_MOVED_DIRECTION_ACK:
		{
			//Uses: 1) Message Type
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_gameMessageType);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			break;
		}
		case UNIT_MOVED_DISTANT_NOTICE:
		{
			//Uses: 1) Message Type
			//		2) Unit ID that moved
			//		3) X coordinate, starting location
			//		4) Y coordinate, starting location
			//		5) X coordinate, ending location
			//		6) Y coordinate, ending location
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_gameMessageType) + sizeof(m_unitID)
					+ sizeof(m_xOld) + sizeof(m_yOld) + sizeof(m_xNew) + sizeof(m_yNew);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			//Unit ID
			memcpy(&m_unitID, buffer, sizeof(m_unitID));
			buffer += sizeof(m_unitID);
			//Starting X position
			memcpy(&m_xOld, buffer, sizeof(m_xOld));
			buffer += sizeof(m_xOld);
			//Starting Y position
			memcpy(&m_yOld, buffer, sizeof(m_yOld));
			buffer += sizeof(m_yOld);
			//Ending X position
			memcpy(&m_xNew, buffer, sizeof(m_xNew));
			buffer += sizeof(m_xNew);
			//Ending Y position
			memcpy(&m_yNew, buffer, sizeof(m_yNew));
			buffer += sizeof(m_yNew);

			break;
		}
		case UNIT_MOVED_DISTANT_ACK:
		{
			//Uses: 1) Message Type
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_gameMessageType);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			break;
		}
		case SURRENDER_NOTICE:
		{
			//Uses: 1) Message Type
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_gameMessageType);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			break;
		}
		case SURRENDER_ACK:
		{
			//Uses: 1) Message Type
			uint32_t expectedSize = MESSAGE_HDR_SIZE + sizeof(m_gameMessageType);
			if( length != expectedSize)
			{
				m_serializeError = true;
				return;
			}

			break;
		}
		default:
		{
			m_serializeError = true;
			break;
		}
	}
}

char *GameMessage::Serialize(uint32_t *length)
{
	char *buffer, *originalBuffer;
	uint32_t messageSize;
	switch(m_gameMessageType)
	{
		case MOVE_UNIT_DIRECTION_REQUEST:
		{
			//Uses: 1) Message Type
			//		2) Unit ID to move
			//		3) X coordinate, starting location
			//		4) Y coordinate, starting location
			//		5) Direction to move unit (one hop)
			messageSize =  MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_gameMessageType) + sizeof(m_unitID)
					+ sizeof(m_xOld) + sizeof(m_yOld)	+ sizeof(m_unitDirection);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//GameMessage type
			memcpy(buffer, &m_gameMessageType, sizeof(m_gameMessageType));
			buffer += sizeof(m_gameMessageType);
			//Unit ID
			memcpy(buffer, &m_unitID, sizeof(m_unitID));
			buffer += sizeof(m_unitID);
			//X coord
			memcpy(buffer, &m_xOld, sizeof(m_xOld));
			buffer += sizeof(m_xOld);
			//Y coord
			memcpy(buffer, &m_yOld, sizeof(m_yOld));
			buffer += sizeof(m_yOld);
			//Direction enum
			memcpy(buffer, &m_unitDirection, sizeof(m_unitDirection));
			buffer += sizeof(m_unitDirection);

			break;
		}
		case MOVE_UNIT_DIRECTION_REPLY:
		{
			//Uses: 1) Message Type
			//		2) MoveResult enum describing success or failure
			//		3) X coordinate, ending location
			//		4) Y coordinate, ending location
			messageSize =  MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_gameMessageType) + sizeof(m_moveResult)
					+ sizeof(m_xNew) + sizeof(m_yNew);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//GameMessage type
			memcpy(buffer, &m_gameMessageType, sizeof(m_gameMessageType));
			buffer += sizeof(m_gameMessageType);
			//Unit ID
			memcpy(buffer, &m_moveResult, sizeof(m_moveResult));
			buffer += sizeof(m_moveResult);
			//X coord
			memcpy(buffer, &m_xNew, sizeof(m_xNew));
			buffer += sizeof(m_xNew);
			//Y coord
			memcpy(buffer, &m_yNew, sizeof(m_yNew));
			buffer += sizeof(m_yNew);

			break;
		}
		case MOVE_UNIT_DISTANT_REQUEST:
		{
			//Uses: 1) Message Type
			//		2) Unit ID that moved
			//		3) X coordinate, starting location
			//		4) Y coordinate, starting location
			//		5) X coordinate, ending location
			//		6) Y coordinate, ending location
			messageSize =  MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_gameMessageType) + sizeof(m_unitID)
					+ sizeof(m_xOld) + sizeof(m_yOld) + sizeof(m_xNew) + sizeof(m_yNew);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//GameMessage type
			memcpy(buffer, &m_gameMessageType, sizeof(m_gameMessageType));
			buffer += sizeof(m_gameMessageType);
			//Unit ID
			memcpy(buffer, &m_unitID, sizeof(m_unitID));
			buffer += sizeof(m_unitID);
			//X coord starting
			memcpy(buffer, &m_xOld, sizeof(m_xOld));
			buffer += sizeof(m_xOld);
			//Y coord starting
			memcpy(buffer, &m_yOld, sizeof(m_yOld));
			buffer += sizeof(m_yOld);
			//X coord ending
			memcpy(buffer, &m_xNew, sizeof(m_xNew));
			buffer += sizeof(m_xNew);
			//Y coord ending
			memcpy(buffer, &m_yNew, sizeof(m_yNew));
			buffer += sizeof(m_yNew);

			break;
		}
		case MOVE_UNIT_DISTANT_REPLY:
		{
			//Uses: 1) Message Type
			//		2) MoveResult enum describing success or failure
			//		3) Unit ID
			//		4) X coordinate, ending location
			//		5) Y coordinate, ending location
			messageSize =  MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_gameMessageType) + sizeof(m_unitID) + sizeof(m_moveResult)
					+ sizeof(m_xNew) + sizeof(m_yNew);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//GameMessage type
			memcpy(buffer, &m_gameMessageType, sizeof(m_gameMessageType));
			buffer += sizeof(m_gameMessageType);

			//Unit ID
			memcpy(buffer, &m_unitID, sizeof(m_unitID));
			buffer += sizeof(m_unitID);

			//Move result
			memcpy(buffer, &m_moveResult, sizeof(m_moveResult));
			buffer += sizeof(m_moveResult);
			//X coord
			memcpy(buffer, &m_xNew, sizeof(m_xNew));
			buffer += sizeof(m_xNew);
			//Y coord
			memcpy(buffer, &m_yNew, sizeof(m_yNew));
			buffer += sizeof(m_yNew);

			break;
		}
		case UNIT_MOVED_DIRECTION_NOTICE:
		{
			//Uses: 1) Message Type
			//		2) Unit ID moved
			//		3) X coordinate, starting location
			//		4) Y coordinate, starting location
			//		5) Direction moved (one hop)
			messageSize =  MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_gameMessageType) + sizeof(m_unitID)
					+ sizeof(m_xOld) + sizeof(m_yOld)	+ sizeof(m_unitDirection);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//GameMessage type
			memcpy(buffer, &m_gameMessageType, sizeof(m_gameMessageType));
			buffer += sizeof(m_gameMessageType);
			//Unit ID
			memcpy(buffer, &m_unitID, sizeof(m_unitID));
			buffer += sizeof(m_unitID);
			//X coord
			memcpy(buffer, &m_xOld, sizeof(m_xOld));
			buffer += sizeof(m_xOld);
			//Y coord
			memcpy(buffer, &m_yOld, sizeof(m_yOld));
			buffer += sizeof(m_yOld);
			//Direction enum
			memcpy(buffer, &m_unitDirection, sizeof(m_unitDirection));
			buffer += sizeof(m_unitDirection);

			break;
		}
		case UNIT_MOVED_DIRECTION_ACK:
		{
			//Uses: 1) Message Type
			messageSize =  MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_gameMessageType);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//GameMessage type
			memcpy(buffer, &m_gameMessageType, sizeof(m_gameMessageType));
			buffer += sizeof(m_gameMessageType);
			break;
		}
		case UNIT_MOVED_DISTANT_NOTICE:
		{
			messageSize =  MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_gameMessageType) + sizeof(m_unitID)
					+ sizeof(m_xOld) + sizeof(m_yOld)	+ sizeof(m_xNew) + sizeof(m_yNew);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//GameMessage type
			memcpy(buffer, &m_gameMessageType, sizeof(m_gameMessageType));
			buffer += sizeof(m_gameMessageType);
			//Unit ID
			memcpy(buffer, &m_unitID, sizeof(m_unitID));
			buffer += sizeof(m_unitID);
			//X coord starting
			memcpy(buffer, &m_xOld, sizeof(m_xOld));
			buffer += sizeof(m_xOld);
			//Y coord starting
			memcpy(buffer, &m_yOld, sizeof(m_yOld));
			buffer += sizeof(m_yOld);
			//X coord ending
			memcpy(buffer, &m_xNew, sizeof(m_xNew));
			buffer += sizeof(m_xNew);
			//Y coord ending
			memcpy(buffer, &m_yNew, sizeof(m_yNew));
			buffer += sizeof(m_yNew);

			break;
		}
		case UNIT_MOVED_DISTANT_ACK:
		{
			//Uses: 1) Message Type
			messageSize =  MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_gameMessageType);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//GameMessage type
			memcpy(buffer, &m_gameMessageType, sizeof(m_gameMessageType));
			buffer += sizeof(m_gameMessageType);
			break;
		}
		case SURRENDER_NOTICE:
		{
			//Uses: 1) Message Type
			messageSize =  MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_gameMessageType);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//GameMessage type
			memcpy(buffer, &m_gameMessageType, sizeof(m_gameMessageType));
			buffer += sizeof(m_gameMessageType);
			break;
		}
		case SURRENDER_ACK:
		{
			//Uses: 1) Message Type
			messageSize =  MESSAGE_HDR_SIZE + sizeof(messageSize) + sizeof(m_gameMessageType);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			SerializeHeader(&buffer, messageSize);

			//GameMessage type
			memcpy(buffer, &m_gameMessageType, sizeof(m_gameMessageType));
			buffer += sizeof(m_gameMessageType);
			break;
		}
		default:
		{
			return NULL;
		}
	}

	*length = messageSize;
	return originalBuffer;
}


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

GameMessage::GameMessage()
{

}

GameMessage::GameMessage(char *buffer, uint32_t length)
{
	if( length < MESSAGE_MIN_SIZE )
	{
		return;
	}

	m_serializeError = false;

	//Copy the message type
	memcpy(&m_type, buffer, MESSAGE_MIN_SIZE);
	buffer += MESSAGE_MIN_SIZE;

	switch(m_type)
	{
		case MOVE_UNIT_DIRECTION_REQUEST:
		{
			//Uses: 1) Message Type
			//		2) Unit ID to move
			//		3) X coordinate, starting location
			//		4) Y coordinate, starting location
			//		5) Direction to move unit (one hop)
			uint32_t expectedSize = sizeof(m_type) + sizeof(m_unitID) + sizeof(m_xOld) + sizeof(m_yOld)
					+ sizeof(m_direction);
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
			memcpy(&m_direction, buffer, sizeof(m_direction));
			buffer += sizeof(m_direction);

			break;
		}
		case MOVE_UNIT_DIRECTION_REPLY:
		{
			//Uses: 1) Message Type
			//		2) MoveResult enum describing success or failure
			//		3) X coordinate, correct starting location
			//		4) Y coordinate, correct starting location
			uint32_t expectedSize = sizeof(m_type) + sizeof(m_moveResult) + sizeof(m_xOld) + sizeof(m_yOld);
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
		case UNIT_MOVED_DIRECTION_NOTICE:
		{
			//Uses: 1) Message Type
			//		2) Unit ID that moved
			//		3) X coordinate, starting location
			//		4) Y coordinate, starting location
			//		5) Direction unit moved (one hop)
			uint32_t expectedSize = sizeof(m_type) + sizeof(m_unitID) + sizeof(m_xOld) + sizeof(m_yOld)
					+ sizeof(m_direction);
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
			memcpy(&m_direction, buffer, sizeof(m_direction));
			buffer += sizeof(m_direction);

			break;
		}
		case UNIT_MOVED_DIRECTION_ACK:
		{
			//Uses: 1) Message Type
			uint32_t expectedSize = sizeof(m_type);
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
	switch(m_type)
	{
		case MOVE_UNIT_DIRECTION_REQUEST:
		{
			//Uses: 1) Message Type
			//		2) Unit ID to move
			//		3) X coordinate, starting location
			//		4) Y coordinate, starting location
			//		5) Direction to move unit (one hop)
			messageSize =  sizeof(m_type) + sizeof(m_unitID) + sizeof(m_xOld) + sizeof(m_yOld)
					+ sizeof(m_direction);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			//Message type
			memcpy(buffer, &m_type, sizeof(m_type));
			buffer += sizeof(m_type);
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
			memcpy(buffer, &m_direction, sizeof(m_direction));
			buffer += sizeof(m_direction);

			break;
		}
		case MOVE_UNIT_DIRECTION_REPLY:
		{
			//Uses: 1) Message Type
			//		2) MoveResult enum describing success or failure
			//		3) X coordinate, correct starting location
			//		4) Y coordinate, correct starting location
			messageSize =  sizeof(m_type) + sizeof(m_moveResult) + sizeof(m_xOld) + sizeof(m_yOld);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			//Message type
			memcpy(buffer, &m_type, sizeof(m_type));
			buffer += sizeof(m_type);
			//Unit ID
			memcpy(buffer, &m_moveResult, sizeof(m_moveResult));
			buffer += sizeof(m_moveResult);
			//X coord
			memcpy(buffer, &m_xOld, sizeof(m_xOld));
			buffer += sizeof(m_xOld);
			//Y coord
			memcpy(buffer, &m_yOld, sizeof(m_yOld));
			buffer += sizeof(m_yOld);

			break;
		}
		case UNIT_MOVED_DIRECTION_NOTICE:
		{
			//Uses: 1) Message Type
			//		2) Unit ID moved
			//		3) X coordinate, starting location
			//		4) Y coordinate, starting location
			//		5) Direction moved (one hop)
			messageSize =  sizeof(m_type) + sizeof(m_unitID) + sizeof(m_xOld) + sizeof(m_yOld)
					+ sizeof(m_direction);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			//Message type
			memcpy(buffer, &m_type, sizeof(m_type));
			buffer += sizeof(m_type);
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
			memcpy(buffer, &m_direction, sizeof(m_direction));
			buffer += sizeof(m_direction);

			break;
		}
		case UNIT_MOVED_DIRECTION_ACK:
		{
			//Uses: 1) Message Type
			messageSize =  sizeof(m_type);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			//Message type
			memcpy(buffer, &m_type, sizeof(m_type));
			buffer += sizeof(m_type);
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


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

	serializeError = false;

	//Copy the message type
	memcpy(&type, buffer, MESSAGE_MIN_SIZE);
	buffer += MESSAGE_MIN_SIZE;

	switch(type)
	{
		case MOVE_UNIT_DIRECTION_REQUEST:
		{
			//Uses: 1) Message Type
			//		2) Unit ID to move
			//		3) X coordinate, starting location
			//		4) Y coordinate, starting location
			//		5) Direction to move unit (one hop)
			uint32_t expectedSize = sizeof(type) + sizeof(unitID) + sizeof(xOld) + sizeof(yOld)
					+ sizeof(direction);
			if( length != expectedSize)
			{
				serializeError = true;
				return;
			}

			//Unit ID
			memcpy(&unitID, buffer, sizeof(unitID));
			buffer += sizeof(unitID);
			//Starting X position
			memcpy(&xOld, buffer, sizeof(xOld));
			buffer += sizeof(xOld);
			//Starting Y position
			memcpy(&yOld, buffer, sizeof(yOld));
			buffer += sizeof(yOld);
			//Direction (enum) to move
			memcpy(&direction, buffer, sizeof(direction));
			buffer += sizeof(direction);

			break;
		}
		case MOVE_UNIT_DIRECTION_REPLY:
		{
			//Uses: 1) Message Type
			//		2) MoveResult enum describing success or failure
			//		3) X coordinate, correct starting location
			//		4) Y coordinate, correct starting location
			uint32_t expectedSize = sizeof(type) + sizeof(moveResult) + sizeof(xOld) + sizeof(yOld);
			if( length != expectedSize)
			{
				serializeError = true;
				return;
			}

			//Unit ID
			memcpy(&unitID, buffer, sizeof(unitID));
			buffer += sizeof(unitID);
			//Movement Result enumeration
			memcpy(&moveResult, buffer, sizeof(moveResult));
			buffer += sizeof(moveResult);
			//Starting X position
			memcpy(&xOld, buffer, sizeof(xOld));
			buffer += sizeof(xOld);
			//Starting Y position
			memcpy(&yOld, buffer, sizeof(yOld));
			buffer += sizeof(yOld);
			break;
		}
		case UNIT_MOVED_DIRECTION_NOTICE:
		{
			//Uses: 1) Message Type
			//		2) Unit ID that moved
			//		3) X coordinate, starting location
			//		4) Y coordinate, starting location
			//		5) Direction unit moved (one hop)
			uint32_t expectedSize = sizeof(type) + sizeof(unitID) + sizeof(xOld) + sizeof(yOld)
					+ sizeof(direction);
			if( length != expectedSize)
			{
				serializeError = true;
				return;
			}

			//Unit ID
			memcpy(&unitID, buffer, sizeof(unitID));
			buffer += sizeof(unitID);
			//Starting X position
			memcpy(&xOld, buffer, sizeof(xOld));
			buffer += sizeof(xOld);
			//Starting Y position
			memcpy(&yOld, buffer, sizeof(yOld));
			buffer += sizeof(yOld);
			//Direction (enum) moved
			memcpy(&direction, buffer, sizeof(direction));
			buffer += sizeof(direction);

			break;
		}
		case UNIT_MOVED_DIRECTION_ACK:
		{
			//Uses: 1) Message Type
			uint32_t expectedSize = sizeof(type);
			if( length != expectedSize)
			{
				serializeError = true;
				return;
			}

			break;
		}
		default:
		{
			serializeError = true;
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
		case MOVE_UNIT_DIRECTION_REQUEST:
		{
			//Uses: 1) Message Type
			//		2) Unit ID to move
			//		3) X coordinate, starting location
			//		4) Y coordinate, starting location
			//		5) Direction to move unit (one hop)
			messageSize =  sizeof(type) + sizeof(unitID) + sizeof(xOld) + sizeof(yOld)
					+ sizeof(direction);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			//Message type
			memcpy(buffer, &type, sizeof(type));
			buffer += sizeof(type);
			//Unit ID
			memcpy(buffer, &unitID, sizeof(unitID));
			buffer += sizeof(unitID);
			//X coord
			memcpy(buffer, &xOld, sizeof(xOld));
			buffer += sizeof(xOld);
			//Y coord
			memcpy(buffer, &yOld, sizeof(yOld));
			buffer += sizeof(yOld);
			//Direction enum
			memcpy(buffer, &direction, sizeof(direction));
			buffer += sizeof(direction);

			break;
		}
		case MOVE_UNIT_DIRECTION_REPLY:
		{
			//Uses: 1) Message Type
			//		2) MoveResult enum describing success or failure
			//		3) X coordinate, correct starting location
			//		4) Y coordinate, correct starting location
			messageSize =  sizeof(type) + sizeof(moveResult) + sizeof(xOld) + sizeof(yOld);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			//Message type
			memcpy(buffer, &type, sizeof(type));
			buffer += sizeof(type);
			//Unit ID
			memcpy(buffer, &moveResult, sizeof(moveResult));
			buffer += sizeof(moveResult);
			//X coord
			memcpy(buffer, &xOld, sizeof(xOld));
			buffer += sizeof(xOld);
			//Y coord
			memcpy(buffer, &yOld, sizeof(yOld));
			buffer += sizeof(yOld);

			break;
		}
		case UNIT_MOVED_DIRECTION_NOTICE:
		{
			//Uses: 1) Message Type
			//		2) Unit ID moved
			//		3) X coordinate, starting location
			//		4) Y coordinate, starting location
			//		5) Direction moved (one hop)
			messageSize =  sizeof(type) + sizeof(unitID) + sizeof(xOld) + sizeof(yOld)
					+ sizeof(direction);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			//Message type
			memcpy(buffer, &type, sizeof(type));
			buffer += sizeof(type);
			//Unit ID
			memcpy(buffer, &unitID, sizeof(unitID));
			buffer += sizeof(unitID);
			//X coord
			memcpy(buffer, &xOld, sizeof(xOld));
			buffer += sizeof(xOld);
			//Y coord
			memcpy(buffer, &yOld, sizeof(yOld));
			buffer += sizeof(yOld);
			//Direction enum
			memcpy(buffer, &direction, sizeof(direction));
			buffer += sizeof(direction);

			break;
		}
		case UNIT_MOVED_DIRECTION_ACK:
		{
			//Uses: 1) Message Type
			messageSize =  sizeof(type);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			//Message type
			memcpy(buffer, &type, sizeof(type));
			buffer += sizeof(type);
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


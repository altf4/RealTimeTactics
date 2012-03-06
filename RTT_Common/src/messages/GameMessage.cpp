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
			uint32_t expectedSize = sizeof(type) + sizeof(moveResult);
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
		case MOVE_UNIT_DIRECTION_REQUEST: //TODO make a new message type
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
			//Unit ID
			memcpy(buffer, &xOld, sizeof(xOld));
			buffer += sizeof(xOld);
			//Unit ID
			memcpy(buffer, &yOld, sizeof(yOld));
			buffer += sizeof(yOld);
			//Unit ID
			memcpy(buffer, &direction, sizeof(direction));
			buffer += sizeof(direction);

			break;
		}
		case MOVE_UNIT_DIRECTION_REPLY:
		{
			//Uses: 1) Message Type
			//		2) MoveResult enum describing success or failure
			messageSize =  sizeof(type) + sizeof(moveResult);
			buffer = (char*)malloc(messageSize);
			originalBuffer = buffer;

			//Message type
			memcpy(buffer, &type, sizeof(type));
			buffer += sizeof(type);
			//Unit ID
			memcpy(buffer, &moveResult, sizeof(moveResult));
			buffer += sizeof(moveResult);

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


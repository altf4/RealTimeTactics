//============================================================================
// Name        : GameCommands.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Commands sent to the game server by client
//============================================================================

#include "GameCommands.h"
#include "Gameboard.h"
#include "ClientProtocolHandler.h"

using namespace std;
using namespace RTT;

extern int callbackSocket;

//The current gameboard being used this match
Gameboard *gameboard;

UnitList units;

//********************************************
//				Movement Commands
//********************************************

//Move a Unit to an adjacent tile
//	unitID - The ID of the unit to move.
//	direction - The direction to move the unit
//		NOTE: The unit will also by default made to face the direction moved
//	returns - A MovementResult struct describing the success or error of the move
struct MovementResult MoveUnit(uint32_t unitID, uint32_t xOld, uint32_t yOld, enum Direction direction)
{
	struct MovementResult result;

	//First, do a quick legality check. So we don't bother the server if we know
	//	that this is invalid.
	if( units.count(unitID) == 0)
	{
		result.result = MOVE_NO_SUCH_UNIT;
		return result;
	}

	GameMessage moveRequest;
	moveRequest.type = MOVE_UNIT_DIRECTION_REQUEST;
	moveRequest.unitID = unitID;
	moveRequest.xOld = xOld;
	moveRequest.yOld = xOld;
	moveRequest.direction = direction;
	if(!Message::WriteMessage(&moveRequest, callbackSocket))
	{
		result.result = MOVE_MESSAGE_SEND_ERROR;
		return result;
	}

	Message *reply = Message::ReadMessage(callbackSocket);
	if( reply == NULL)
	{
		SendError(callbackSocket, PROTOCOL_ERROR);
		result.result = MOVE_MESSAGE_SEND_ERROR;
		delete reply;
		return result;
	}
	if( reply->type != MOVE_UNIT_DIRECTION_REPLY)
	{
		SendError(callbackSocket, PROTOCOL_ERROR);
		result.result = MOVE_MESSAGE_SEND_ERROR;
		delete reply;
		return result;
	}
	GameMessage *moveReply = (GameMessage*)reply;
	result.result = moveReply->moveResult;
	result.originalX = moveReply->xOld;
	result.originalY = moveReply->yOld;

	delete moveReply;
	return result;
}

//Move a Unit to a distant tile
//	unitID - The ID of the unit to move.
//	destination - The Coordinate of the tile being moved to
//		NOTE: The unit will also by default made to face the direction of the last hop
//	returns - A MovementResult struct describing the success or error of the move
struct MovementResult RTT::MoveUnit(uint32_t unitID, struct Coordinate destination)
{

}

//Make a unit change the direction it is facing
//	unitID - The ID of the unit to move
//	direction - The new direction to move the unit to face
//	returns - simple boolean result of whether the move succeeded
bool RTT::ChangeUnitFacing(uint32_t unitID, enum Direction direction)
{

}

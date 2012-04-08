//============================================================================
// Name        : GameCommands.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Commands sent to the game server by client
//============================================================================

#include "GameCommands.h"
#include "Gameboard.h"

using namespace std;
using namespace RTT;
using google::dense_hash_map;
using tr1::hash;

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
struct MovementResult RTT::MoveUnit(uint32_t unitID, enum Direction direction)
{
	struct MovementResult result;

	//First, do a quick legality check. So we don't bother the server if we know
	//	that this is invalid.
	if( units.count(unitID) == 0)
	{
		result.m_result = UNIT_DOESNT_EXIST;
		return result;
	}

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

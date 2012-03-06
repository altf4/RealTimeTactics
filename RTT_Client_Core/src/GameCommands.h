//============================================================================
// Name        : GameCommands.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Commands sent to the game server by client
//============================================================================
#ifndef GAMECOMMANDS_H_
#define GAMECOMMANDS_H_

#include <stdint.h>
#include <google/dense_hash_map>

#include "Tile.h"
#include "Unit.h"
#include "messages/GameMessage.h"

using google::dense_hash_map;
using tr1::hash;

namespace RTT
{

//********************************************
//				Unit List
//********************************************
struct eqint
{
  bool operator()(uint32_t s1, uint32_t s2) const
  {
    return (s1 == s2);
  }
};

//Define types, so it's easier to refer to later
//Key : Player ID
typedef dense_hash_map<uint32_t, Unit*, hash<uint32_t>, eqint> UnitList;


//********************************************
//				Result Types
//********************************************

struct MovementResult
{
	enum MoveResult result;

	//Used in error conditions to help if the client is unsynced:

	//This is the position that the unit SHOULD be at
	uint32_t originalX, originalY;
};

//********************************************
//				Movement Commands
//********************************************

//Move a Unit to an adjacent tile
//	unitID - The ID of the unit to move.
//	direction - The direction to move the unit
//		NOTE: The unit will also by default made to face the direction moved
//	returns - A MovementResult struct describing the success or error of the move
struct MovementResult MoveUnit(uint32_t unitID, uint32_t oldX, uint32_t oldY, enum Direction direction);

//Move a Unit to a distant tile
//	unitID - The ID of the unit to move.
//	destination - The Coordinate of the tile being moved to
//		NOTE: The unit will also by default made to face the direction of the last hop
//	returns - A MovementResult struct describing the success or error of the move
struct MovementResult MoveUnit(uint32_t unitID, struct Coordinate destination);

//Make a unit change the direction it is facing
//	unitID - The ID of the unit to move
//	direction - The new direction to move the unit to face
//	returns - simple boolean result of whether the move succeeded
bool ChangeUnitFacing(uint32_t unitID, enum Direction direction);

//********************************************
//				Action Commands
//********************************************


//********************************************
//				Match Commands
//********************************************

//Pause the match
bool PauseMatch();

//Unpause Match
bool UnPauseMatch();

//Give up and leave to the MainLobby
bool SurrenderMatch();

}
#endif /* GAMECOMMANDS_H_ */

//============================================================================
// Name        : Unit.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Unit class, represents a single pawn in your army
//============================================================================

#ifndef UNIT_H_
#define UNIT_H_

#include <stdlib.h>

using namespace std;

namespace LoF
{

//The six possible directions to face or move on a hex gameboard
enum direction
{
	NORTHEAST,
	NORTHWEST,
	WEST,
	SOUTHWEST,
	SOUTHEAST,
	EAST
};

class Unit
{

public:

	uint maxHealth;
	uint currentHealth;

	uint maxEnergy;
	uint currentEnergy;

	uint verticalMovement;
	uint horizontalMovement;

	//How many charge points accumulated per round
	//	IE: How quickly this unit gets to have a turn
	//	(Not how far it can walk)
	uint speed;
	uint currentCharge;

	//How many movement points the unit has
	//	Each tile takes up a certain number of movement points
	double movement;
	double currentMovement;

	direction directionFacing;


	//Empty constructor
	Unit();

	//Returns true if the given Unit is the same one as this
	//	IE: The actual same unit. Not another unit with the same properties
	//	Does this by comparing ID's
	bool Equals(Unit *testUnit);

	//TODO: Skills, class, etc...

	//Unique identifier for this unit.
	//	NOT an index into any list
	uint ID;

};


} //end namespace LoF

#endif /* UNIT_H_ */

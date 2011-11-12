//============================================================================
// Name        : Unit.h
// Author      : AltF4
// Copyright   : GNU GPLv3
// Description : Unit class, represents a single pawn in your army
//============================================================================

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

	//TODO: Skills, class, etc...
};


} //end namespace LoF

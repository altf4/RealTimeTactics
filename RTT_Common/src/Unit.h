//============================================================================
// Name        : Unit.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Unit class, represents a single pawn in your army
//============================================================================

#ifndef UNIT_H_
#define UNIT_H_

#include <stdlib.h>
#include <stdint.h>
#include <string>
#include "Job.h"

using namespace std;

namespace RTT
{

//The six possible directions to face or move on a hex gameboard
enum Direction: char
{
	NORTHEAST,
	NORTHWEST,
	WEST,
	SOUTHWEST,
	SOUTHEAST,
	EAST
};

class Unit : public Job
{

public:

	uint currentHealth;
	uint currentEnergy;

	//Charge for this player's turn
	uint currentCharge;

	//How many movement points the unit has
	//	Each tile takes up a certain number of movement points
	double currentMovement;

	//Position on the gameboard
	uint x, y;
	Direction directionFacing;

	//Unique identifier for this unit.
	//	NOT an index into any list
	uint ID;

	//The name of this Unit. Given by the client
	// The client can choose to name his Units unique things to distinguish them
	string unitName;

	Unit( );

	//Returns true if the given Unit is the same one as this
	//	IE: The actual same unit. Not another unit with the same properties
	//	Does this by comparing ID's
	bool Equals(Unit *testUnit);

	//Used for making threadsafe access of the unit when in a list
	pthread_mutex_t m_unitLock;

};


} //end namespace RTT

#endif /* UNIT_H_ */

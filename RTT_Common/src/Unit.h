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

	uint m_currentHealth;
	uint m_currentEnergy;

	//Charge for this player's turn
	uint m_currentCharge;

	//How many movement points the unit has
	//	Each tile takes up a certain number of movement points
	double m_currentMovement;

	//Position on the gameboard
	uint m_x, m_y;
	Direction m_directionFacing;

	//Unique identifier for this unit.
	//	NOT an index into any list
	uint m_ID;

	//The name of this Unit. Given by the client
	// The client can choose to name his Units unique things to distinguish them
	std::string m_unitName;


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

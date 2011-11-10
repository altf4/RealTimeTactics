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


class Unit
{

public:
	uint maxHealth;
	uint currentHealth;

	uint maxEnergy;
	uint currentEnergy;

	uint verticalMovement;
	uint horizontalMovement;

	uint speed;

	uint directionFacing; //Maybe make an enum?

	//Empty constructor
	Unit();

	//TODO: Skills, class, etc...
};


} //end namespace LoF

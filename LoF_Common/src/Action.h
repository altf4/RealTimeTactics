//============================================================================
// Name        : Action.h
// Author      : AltF4
// Copyright   : GNU GPLv3
// Description : Represents any scheduled action that a unit makes, such as
//					attacks, healing, moving, etc...
//============================================================================

#include <stdlib.h>

namespace LoF
{

class Action
{

public:
	//How many charge points gained per round
	uint speed;

	//The current charge level
	uint currentCharge;

	Action();
};

}

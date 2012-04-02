//============================================================================
// Name        : Action.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Represents any scheduled action that a unit makes, such as
//					attacks, healing, moving, and the unit's turn itself
//============================================================================

#ifndef ACTION_H_
#define ACTION_H_

#include <stdlib.h>

namespace RTT
{

class Action
{

public:
	//How many charge points gained per round
	uint m_speed;

	//The current charge level
	uint m_currentCharge;


	Action();

	//Trigger the action
	void Execute();

	//Returns true if action1 < action2
	static bool CompareActions(Action action1, Action action2);

};

}

#endif /* ACTION_H */

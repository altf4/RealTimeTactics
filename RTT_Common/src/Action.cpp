//============================================================================
// Name        : Unit.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Represents any scheduled action that a unit makes, such as
//					attacks, healing, moving, etc...
//============================================================================

#include "Action.h"

using namespace RTT;

Action::Action()
{

}

//Trigger the action
void Action::Execute()
{

}

bool Action::CompareActions(Action action1, Action action2)
{
	//First, compare currentCharge
	if( action1.currentCharge <  action2.currentCharge)
	{
		return true;
	}
	else if( action1.currentCharge >  action2.currentCharge)
	{
		return false;
	}
	//If tied, compare speed
	else
	{
		if( action1.speed <  action2.speed)
		{
			return true;
		}
		else if( action1.speed >  action2.speed)
		{
			return false;
		}
		//If still tied, say the first is lesser
		else
		{
			return true;
		}
	}
}

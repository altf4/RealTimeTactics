//============================================================================
// Name        : Unit.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Unit class, represents a single pawn in your army
//============================================================================

#include "Unit.h"

using namespace RTT;

static uint lastIDGiven = 0;

Unit::Unit( )
	: Job("something")
{

	ID = lastIDGiven++;
	pthread_mutex_init(&m_unitLock, NULL);
}

//Returns true if the given Unit is the same one as this
//	IE: The actual same unit. Not another unit with the same properties
//	Does this by comparing ID's
bool Unit::Equals(Unit *testUnit)
{
	if( testUnit->ID == ID )
	{
		return true;
	}
	else
	{
		return false;
	}
}

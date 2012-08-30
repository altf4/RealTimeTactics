//============================================================================
// Name        : GameEvents.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Game events which have happened on the server, and the
//					client is being notified about.
//============================================================================

#include "OgreGameEvents.h"

using namespace std;
using namespace RTT;

//Virtual function defined by the caller as a way to hook into this event
void OgreGameEvents::UI_UnitMovedDirectionSignal(uint32_t unitID, struct Coordinate source,
		enum Direction direction, enum Direction facing)
{

}

//A Unit has changed the direction it is facing
//	unitID - The ID of the unit moved
//	direction - The new direction to move the unit to face
//		NOTE: By default, the Unit should be facing the direction moved
//	returns - simple boolean result of whether the move succeeded
void OgreGameEvents::UI_UnitChangedFacingSignal(uint32_t unitID, enum Direction direction)
{

}

//********************************************
//				Action Events
//********************************************

//********************************************
//				Match Events
//********************************************

//The match has ended
void OgreGameEvents::UI_MatchEndedSignal()
{

}

//********************************************
//				Server Events
//********************************************

void OgreGameEvents::UI_MatchPausedSignal()
{

}

void OgreGameEvents::UI_MatchUnPaused()
{

}

void OgreGameEvents::UI_OpponentSurrenderedSignal()
{

}

//============================================================================
// Name        : OgreGameEvents.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Game events which have happened on the server, and the
//					client is being notified about.
//============================================================================

#ifndef OGREGAMEEVENTS_H_
#define OGREGAMEEVENTS_H_

#include "GameEvents.h"

namespace RTT
{

class OgreGameEvents : public GameEvents
{

public:

	//********************************************
	//				Movement Events
	//********************************************

	//Virtual function defined by the caller as a way to hook into this event
	void UI_UnitMovedDirectionSignal(uint32_t unitID, struct Coordinate source,
			enum Direction direction, enum Direction facing);

	void UI_UnitMovedDistantSignal(uint32_t unitID, struct Coordinate source, struct Coordinate dest);

	//A Unit has changed the direction it is facing
	//	unitID - The ID of the unit moved
	//	direction - The new direction to move the unit to face
	//		NOTE: By default, the Unit should be facing the direction moved
	//	returns - simple boolean result of whether the move succeeded
	void UI_UnitChangedFacingSignal(uint32_t unitID, enum Direction direction);

	//********************************************
	//				Action Events
	//********************************************

	//********************************************
	//				Match Events
	//********************************************

	//The match has ended
	void UI_MatchEndedSignal();

	//********************************************
	//				Server Events
	//********************************************

	void UI_MatchPausedSignal();

	void UI_MatchUnPaused();

	void UI_OpponentSurrenderedSignal();
};

}
#endif /* OGREGAMEEVENTS_H_ */

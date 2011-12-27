//============================================================================
// Name        : RTT_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : A team on one side of a match, which contains a collection
//		of players who share a victory condition
//============================================================================


#include "Team.h"

using namespace RTT;

Team::Team(enum TeamNumber newTeam)
{
	team = newTeam;
}

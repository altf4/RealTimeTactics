//============================================================================
// Name        : LoF_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : A single match (or game) that is being played.
//					A server can have many at one time
//============================================================================

#ifndef MATCH_H_
#define MATCH_H_

#include "Gameboard.h"
#include "Player.h"
#include "Action.h"

using namespace std;

namespace LoF
{

enum Status
{
	WAITING_FOR_PLAYERS,
	IN_PROGRESS,
};

class Match
{

public:
	//The gameboard for this match
	Gameboard *gameboard;

	//Players involved
	vector <Player*> players;

	enum Status status;

	// chargingActions is the global list of actions which have not yet been triggered
	// chargedActions is the list of actions which are on queue to be triggered
	vector <Action*> chargingActions, chargedActions;

	Match();
};

}

#endif /* MATCH_H_ */

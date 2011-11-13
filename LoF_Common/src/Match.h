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

using namespace std;

namespace LoF
{

class Match
{
	//The gameboard for this match
	Gameboard *gameboard;

	//Players involved
	vector <Player*> players;

	Match();
};

}

#endif /* MATCH_H_ */

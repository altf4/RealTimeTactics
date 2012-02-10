//============================================================================
// Name        : MatchLoop.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : The main loop for a single RTT match
//============================================================================

#ifndef MATCHLOOP_H_
#define MATCHLOOP_H_

#include "Enums.h"

using namespace std;
using namespace RTT;

enum MatchLoopResult: char
{
	//The player won the match
	MATCH_VICTORY = 0,
	//The player lost the match
	MATCH_DEFEAT,
	//The match finished in a draw
	MATCH_DRAW,
	//The match did not finish
	MATCH_NO_CONTEST,
	//The match never started
	MATCH_ERROR,
};

//The main loop for a single RTT match
// The "realtime" thread that signals
// speed - The speed at which this match should run
// Returns: an enumeration describing
enum MatchLoopResult MatchLoop(enum GameSpeed speed);

//Thread that gets launched every timer tick in order to handle
void *TimerTick(void *param);

#endif /* MATCHLOOP_H_ */

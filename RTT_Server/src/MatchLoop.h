//============================================================================
// Name        : MatchLoop.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : The main loop for a single RTT match
//============================================================================

#ifndef MATCHLOOP_H_
#define MATCHLOOP_H_

#include "Enums.h"
#include "Match.h"

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
// The "realtime" thread that signals the timer ticks
void MatchLoop(RTT::Match *match);

// ptr - Void pointer to the match to start
void *MatchLoopHelper(void *ptr);

//Thread that gets launched every timer tick in order to handle
void *TimerTick(void *param);

#endif /* MATCHLOOP_H_ */

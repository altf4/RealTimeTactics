//============================================================================
// Name        : MatchLoop.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : The main loop for a single RTT match
//============================================================================

#include "MatchLoop.h"
#include "Match.h"
#include "pthread.h"
#include <iostream>

using namespace std;

//The main loop for a single RTT match
// The "realtime" thread that signals
// speed - The speed at which this match should run
// Returns: an enumeration describing the match's result status
enum MatchLoopResult MatchLoop(enum GameSpeed speed)
{
	uint tickDelta = Match::GameSpeedTouSeconds(speed);
	//Lock to ensure that only one timer tick thread can happen at once
	pthread_mutex_t tickLock;
	pthread_mutex_init(&tickLock, NULL);

	enum MatchLoopResult result;
	uint count = 0;

	while(true)
	{
		usleep(tickDelta);

		count++;
		//TODO: If the match has ended, somehow
		if(count == 12)
		{
			result = MATCH_VICTORY;
			return result;
		}

		//Thread that gets called when the timer tick hits
		pthread_t timerTickThreadID;
		pthread_create(&timerTickThreadID, NULL, TimerTick, (void*)&tickLock);
	}

	//return result;
}

void *TimerTick(void *param)
{
	pthread_mutex_t *tickLock = (pthread_mutex_t *)param;
	if(pthread_mutex_trylock(tickLock) != 0)
	{
		//If the lock is already taken, give off a warning, then just wait for the lock
		cerr << "WARNING: Timer tick threads overlapped. Server is too slow to handle!\n";
		pthread_mutex_lock(tickLock);
	}

	//TODO: Important game stuff goes here!
	cout << "Tick!\n";

	pthread_mutex_unlock(tickLock);

	return NULL;
}

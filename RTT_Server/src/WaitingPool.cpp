//============================================================================
// Name        : WaitingPool.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : A class to contain players as they wait for a match to begin
//============================================================================

#include "WaitingPool.h"

using namespace RTT;

WaitingPool::WaitingPool()
{
	pool.set_empty_key(-1);
	pool.set_deleted_key(-2);
}

//Register a player with this pool
// blocking call on success
// Returns: an enumeration describing the match's result status
enum MatchLoopResult WaitingPool::Register(uint playerID, uint matchID, uint maxPlayers,
		int callbackSocket, int receiveSocket, enum GameSpeed speed)
{
	struct PoolEntry entry;
	entry.playerID = playerID;
	entry.callbackSocket = callbackSocket;
	entry.receiveSocket = receiveSocket;
	entry.maxPlayers = maxPlayers;

	pthread_mutex_t returnLock;
	entry.returnLock = returnLock;

	pthread_mutex_init(&(entry.returnLock), NULL);

	if(pool.count(matchID) == 0)
	{
		pool[matchID].push_back(entry);
	}
	else
	{
		//If the match is full...
		if(pool[matchID].size() >= maxPlayers)
		{
			return MATCH_ERROR;
		}
		else
		{
			pool[matchID].push_back(entry);
		}
	}

	//If this is the last player in, then launch the MatchLoop!
	if(pool[matchID].size() == maxPlayers)
	{
		return MatchLoop(speed);
	}
	else
	{
		//Double lock. Seems strange, but we WANT to block, here.
		//	We'll get woken up when the match is done.
		pthread_mutex_lock(&(entry.returnLock));
		pthread_mutex_lock(&(entry.returnLock));
	}

	//TODO: What?!
	return MATCH_VICTORY;

}

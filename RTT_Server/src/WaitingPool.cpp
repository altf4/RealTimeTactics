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
	pthread_mutex_init(&poolLock, NULL);
}

//Register a player with this pool
// blocking call on success
// Returns: an enumeration describing the match's result status
enum MatchLoopResult WaitingPool::Register(uint playerID, uint matchID, uint currentPlayers,
		int callbackSocket, int receiveSocket, enum GameSpeed speed)
{
	cout << "Registering " << playerID << endl;
	struct PoolEntry entry;
	entry.playerID = playerID;
	entry.callbackSocket = callbackSocket;
	entry.receiveSocket = receiveSocket;
	entry.currentPlayers = currentPlayers;

	pthread_mutex_t returnLock;
	entry.returnLock = returnLock;

	pthread_mutex_init(&(entry.returnLock), NULL);

	if( !SetEntry(matchID, entry, currentPlayers))
	{
		return MATCH_ERROR;
	}

	//If this is the last player in, then launch the MatchLoop!
	if(GetMatchPlayerCount(matchID) == currentPlayers)
	{
		enum MatchLoopResult result = MatchLoop(speed);
		WakePlayers(matchID);
		DeleteMatch(matchID);
		return result;
	}
	else
	{
		//Double lock. Seems strange, but we WANT to block, here.
		//	We'll get woken up when the match is done.
		cout << "First lock by " << playerID << endl;
		LockPlayer(matchID, playerID);
		cout << "Second lock by " << playerID << endl;
		LockPlayer(matchID, playerID);
		cout << "Outside locks by " << playerID << endl;
	}

	//TODO: What?!
	return MATCH_VICTORY;

}

bool WaitingPool::SetEntry(uint matchID, PoolEntry entry, uint currentPlayers)
{
	pthread_mutex_lock(&poolLock);
	if(pool.count(matchID) == 0)
	{
		vector<PoolEntry> entryList;
		pool[matchID] = entryList;
		pool[matchID].push_back(entry);
	}
	else
	{
		//If the match is full...
		if(pool[matchID].size() >= currentPlayers)
		{
			pthread_mutex_unlock(&poolLock);
			return false;
		}
		else
		{
			pool[matchID].push_back(entry);
		}
	}
	pthread_mutex_unlock(&poolLock);
	return true;
}

void WaitingPool::DeleteMatch(uint matchID)
{
	pthread_mutex_lock(&poolLock);
	pool.erase(matchID);
	pthread_mutex_unlock(&poolLock);
}

PoolEntry WaitingPool::GetEntry(uint matchID, uint playerID)
{
	pthread_mutex_lock(&poolLock);
	PoolEntry result;
	result.playerID = 0;
	for(uint i=0; i < pool[matchID].size(); i++)
	{
		if(pool[matchID][i].playerID == playerID)
		{
			result = pool[matchID][i];
			pthread_mutex_unlock(&poolLock);
			return result;
		}
	}
	pthread_mutex_unlock(&poolLock);
	return result;
}

uint WaitingPool::GetMatchPlayerCount(uint matchID)
{
	pthread_mutex_lock(&poolLock);
	uint result = pool[matchID].size();
	pthread_mutex_unlock(&poolLock);
	return result;
}

void WaitingPool::WakePlayers(uint matchID)
{
	pthread_mutex_lock(&poolLock);
	for(uint i=0; i < pool[matchID].size(); i++)
	{
		pthread_mutex_unlock(&(pool[matchID][i].returnLock));
	}
	pthread_mutex_unlock(&poolLock);
}

void WaitingPool::LockPlayer(uint matchID, uint playerID)
{
	pthread_mutex_lock(&poolLock);
	pthread_mutex_t *lock;
	for(uint i=0; i < pool[matchID].size(); i++)
	{
		if(pool[matchID][i].playerID == playerID)
		{
			lock = &(pool[matchID][i].returnLock);
		}
	}
	pthread_mutex_unlock(&poolLock);

	pthread_mutex_lock(lock);
}

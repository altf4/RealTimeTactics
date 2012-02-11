//============================================================================
// Name        : WaitingPool.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : A class to contain players as they wait for a match to begin
//============================================================================

#ifndef WAITINGPOOL_H_
#define WAITINGPOOL_H_

#include <vector>
#include <sys/types.h>
#include <stdint.h>
#include <google/dense_hash_map>
#include "MatchLoop.h"

using namespace std;
using google::dense_hash_map;
using tr1::hash;

struct eqint2
{
  bool operator()(int s1, int s2) const
  {
    return (s1 == s2);
  }
};

struct PoolEntry
{
	uint currentPlayers;
	uint playerID;
	int callbackSocket;
	int receiveSocket;
	pthread_mutex_t returnLock;
};

//Key : Match unique ID
typedef dense_hash_map<int, vector<struct PoolEntry>, hash<int>, eqint2> PlayerWaitingPool;

namespace RTT
{

class WaitingPool
{

public:
	//Initializes the pool
	WaitingPool();

	//Register a player with this pool
	// blocking call on success
	// Returns: an enumeration describing the match's result status
	enum MatchLoopResult Register(uint playerID, uint matchID, uint currentPlayers,
			int callbackSocket, int receiveSocket, enum GameSpeed speed);

private:
	PlayerWaitingPool pool;
	pthread_mutex_t poolLock;

	bool SetEntry(uint matchID, PoolEntry entry, uint currentPlayers);
	void DeleteMatch(uint matchID);
	//Sets playerID = 0 if no Entry found. Ugly. Sorry.
	PoolEntry GetEntry(uint matchID, uint playerID);
	uint GetMatchPlayerCount(uint matchID);
	void WakePlayers(uint matchID);
	void LockPlayer(uint matchID, uint playerID);

};


}

#endif /* WAITINGPOOL_H_ */

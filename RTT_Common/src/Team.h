//============================================================================
// Name        : RTT_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : A team on one side of a match, which contains a collection
//		of players who share a victory condition
//============================================================================

#ifndef TEAM_H_
#define TEAM_H_

//Includes Spectators and Referees
#define MAX_TEAMS 10

#include <vector>
#include "Player.h"
#include "Enums.h"

using namespace std;
using namespace RTT;

namespace RTT
{

//Forward declaration
class Player;

class Team
{

public:

	enum TeamNumber team;

	Team(enum TeamNumber newTeam);

	Player *GetPlayer(uint playerID);
	bool RemovePlayer(uint playerID);
	void AddPlayer(Player *newPlayer);
	uint GetFirstPlayerID();
	vector <Player*> GetPlayers();

	vector<struct PlayerDescription> GetPlayerDescriptions();

private:
	//Lock for the players list
	pthread_rwlock_t lock;

	vector <Player*> players;
};

}


#endif /* TEAM_H_ */

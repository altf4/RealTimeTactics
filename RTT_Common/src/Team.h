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
	vector <Player*> players;

	Team(enum TeamNumber newTeam);
};

}


#endif /* TEAM_H_ */

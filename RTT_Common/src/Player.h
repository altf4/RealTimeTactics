//============================================================================
// Name        : RTT_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Collects all information specific to one player
//============================================================================

#ifndef PLAYER_H_
#define PLAYER_H_

#include <vector>
#include "Unit.h"
#include "Match.h"
#include <string>
#include "Enums.h"

using namespace std;

namespace RTT
{

//Forward declaration to avoid self-reference
class Match;

//A fixed size representation of the Player suitable for providing
//	to other Players for display
struct PlayerDescription
{
	uint ID;
	char name[20];
	enum TeamNumber team;
	enum TeamColor color;
};

class Player
{
public:
	string name;

	vector <Unit*> units;

	uint ID;

	//The match that this player is currently in
	Match *currentMatch;

	Player();

	//For when c strings are easier, or std::strings are
	Player(char *username, uint newID);
	Player(string username, uint newID);

};

}
#endif /* PLAYER_H_ */

//============================================================================
// Name        : LoF_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Collects all information specific to one player
//============================================================================

#ifndef PLAYER_H_
#define PLAYER_H_

#include <vector>
#include "Unit.h"
#include <string>

using namespace std;

namespace LoF
{

class Player
{
public:
	string name;

	vector <Unit*> units;

	Player();

	//For when c strings are easier, or std::strings are
	Player(char *username);
	Player(string username);

};

}
#endif /* PLAYER_H_ */

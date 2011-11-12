//============================================================================
// Name        : LoF_Server.cpp
// Author      : AltF4
// Copyright   : GNU GPLv3
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

};

}
#endif /* PLAYER_H_ */

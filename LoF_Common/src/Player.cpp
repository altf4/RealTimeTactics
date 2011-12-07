//============================================================================
// Name        : LoF_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Collects all information specific to one player
//============================================================================

#include "Player.h"

using namespace LoF;

Player::Player()
{

}

Player::Player(char *username)
{
	name = string(username);
}

Player::Player(string username)
{
	name = username;
}


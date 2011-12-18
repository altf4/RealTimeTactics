//============================================================================
// Name        : RTT_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Collects all information specific to one player
//============================================================================

#include "Player.h"

using namespace RTT;

Player::Player()
{
	currentMatch = NULL;
}

Player::Player(char *username)
{
	name = string(username);
	currentMatch = NULL;
}

Player::Player(string username)
{
	name = username;
	currentMatch = NULL;
}


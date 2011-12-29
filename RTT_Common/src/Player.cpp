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

Player::Player(char *username, uint newID)
{
	name = string(username);
	currentMatch = NULL;
	ID = newID;
}

Player::Player(string username, uint newID)
{
	name = username;
	currentMatch = NULL;
	ID = newID;
}

string Player::GetName()
{
	return name;
}

uint Player::GetID()
{
	return ID;
}

enum TeamNumber Player::GetTeam()
{
	return team;
}

enum TeamColor Player::GetColor()
{
	return color;
}


void Player::SetName(string newName)
{
	name = newName;
	strncpy(description.name, newName.c_str(), sizeof(description.name));
}

void Player::SetID(uint newID)
{
	ID = newID;
	description.ID = newID;
}

void Player::SetTeam(enum TeamNumber newTeam)
{
	team = newTeam;
	description.team = newTeam;
}

void Player::SetColor(enum TeamColor newColor)
{
	color = newColor;
	description.color = newColor;
}

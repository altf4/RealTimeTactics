//============================================================================
// Name        : RTT_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : A single match (or game) that is being played.
//					A server can have many at one time
//============================================================================s

#include "Match.h"

using namespace std;
using namespace RTT;

Match::Match()
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	timeCreated = tv.tv_sec;
	description.timeCreated = tv.tv_sec;
}

//SET methods
void Match::SetID(uint newID)
{
	ID = newID;
	description.ID = newID;
}

void Match::SetStatus(enum Status newStatus)
{
	status = newStatus;
	description.status = newStatus;
}

void Match::SetMaxPlayers(uint newMaxPlayers)
{
	maxPlayers = newMaxPlayers;
	description.maxPlayers = newMaxPlayers;
}

void Match::SetCurrentPlayerCount(uint newPlayerCount)
{
	currentPlayerCount = newPlayerCount;
	description.currentPlayerCount = newPlayerCount;
}

void Match::SetName(string newName)
{
	name = newName;
	name.resize(MAX_MATCHNAME_LEN);
	strncpy(description.name, newName.c_str(), MAX_MATCHNAME_LEN);
}

//GET methods
enum Status Match::GetStatus()
{
	return status;
}

uint Match::GetID()
{
	return ID;
}

uint Match::GetMaxPlayers()
{
	return maxPlayers;
}

uint Match::GetCurrentPlayerCount()
{
	return currentPlayerCount;
}

string Match::GetName()
{
	return name;
}

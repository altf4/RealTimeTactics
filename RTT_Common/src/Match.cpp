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

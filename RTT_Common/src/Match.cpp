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
	for(uint i = 0; i < MAX_TEAMS; i++)
	{
		teams[i] = new Team((enum TeamNumber)i);
	}
}

Match::~Match()
{
	for(uint i = 0; i < MAX_TEAMS; i++)
	{
		if( teams[i] != NULL )
		{
			delete teams[i];
		}
	}
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

bool Match::AddPlayer(Player *player, enum TeamNumber teamNum)
{
	if( currentPlayerCount >= maxPlayers )
	{
		return false;
	}
	if( teamNum > REFEREE)
	{
		return false;
	}
	teams[teamNum]->players.push_back(player);
	currentPlayerCount++;
	description.currentPlayerCount++;

	return true;
}

bool Match::RemovePlayer( uint playerID )
{
	for(uint i = 0; i < MAX_TEAMS; i++)
	{
		vector<Player*>::iterator it = teams[i]->players.begin();
		for( ; it != teams[i]->players.end(); it++ )
		{
			if( (*it)->ID == playerID )
			{
				teams[i]->players.erase(it);
				currentPlayerCount--;
				description.currentPlayerCount--;
				return true;
			}
		}
	}
	return false;
}

Player* Match::GetPlayer( uint playerID )
{
	for(uint i = 0; i < MAX_TEAMS; i++)
	{
		vector<Player*>::iterator it = teams[i]->players.begin();
		for( ; it != teams[i]->players.end(); it++ )
		{
			if( (*it)->ID == playerID )
			{
				return (*it);
			}
		}
	}
	return false;
}

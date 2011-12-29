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

Match::Match(Player *player)
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	timeCreated = tv.tv_sec;
	description.timeCreated = tv.tv_sec;
	for(uint i = 0; i < MAX_TEAMS; i++)
	{
		teams[i] = new Team((enum TeamNumber)i);
	}
	leader = player;
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
			if( (*it)->GetID() == playerID )
			{
				teams[i]->players.erase(it);
				currentPlayerCount--;
				description.currentPlayerCount--;
				leader = GetFirstPlayer();
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
			if( (*it)->GetID() == playerID )
			{
				return (*it);
			}
		}
	}
	return false;
}

//Get the first Player in the teams lists
//	For use in getting the next leader when one leaves
//	Returns NULL if there are no more players
Player* Match::GetFirstPlayer()
{
	for(uint i = 0; i < MAX_TEAMS; i++)
	{
		vector<Player*>::iterator it = teams[i]->players.begin();
		for( ; it != teams[i]->players.end(); it++ )
		{
			return *it;
		}
	}
	return NULL;
}

bool Match::ChangeTeam(uint playerID, enum TeamNumber newTeam)
{
	if( newTeam > REFEREE)
	{
		return false;
	}
	for(uint i = 0; i < MAX_TEAMS; i++)
	{
		vector<Player*>::iterator it = teams[i]->players.begin();
		for( ; it != teams[i]->players.end(); it++ )
		{
			if( (*it)->GetID() == playerID )
			{
				//Remove player from existing team
				teams[i]->players.erase(it);
				//Add to new team
				teams[newTeam]->players.push_back(*it);
				(*it)->SetTeam(newTeam);
				return true;
			}
		}
	}
	return false;
}

bool Match::StartMatch()
{

}

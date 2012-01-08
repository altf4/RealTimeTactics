//============================================================================
// Name        : RTT_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : A team on one side of a match, which contains a collection
//		of players who share a victory condition
//============================================================================


#include "Team.h"

using namespace RTT;

Team::Team(enum TeamNumber newTeam)
{
	team = newTeam;
}

Player *Team::GetPlayer(uint playerID)
{
	pthread_rwlock_rdlock(&lock);
	for(uint i = 0; i < players.size(); i++)
	{
		if( players[i]->GetID() == playerID )
		{
			pthread_rwlock_unlock(&lock);
			return players[i];
		}
	}
	pthread_rwlock_unlock(&lock);
	return NULL;
}

bool Team::RemovePlayer(uint playerID)
{
	pthread_rwlock_wrlock(&lock);

	vector<Player*>::iterator it = players.begin();
	for( ; it != players.end(); it++ )
	{
		if( (*it)->GetID() == playerID)
		{
			players.erase(it);
			pthread_rwlock_unlock(&lock);
			return true;
		}
	}
	pthread_rwlock_unlock(&lock);
	return false;
}

void Team::AddPlayer(Player *newPlayer)
{
	pthread_rwlock_wrlock(&lock);
	players.push_back(newPlayer);
	pthread_rwlock_unlock(&lock);
}

uint Team::GetFirstPlayerID()
{
	pthread_rwlock_rdlock(&lock);
	if(players.size() > 0)
	{
		uint temp = players[0]->GetID();
		pthread_rwlock_unlock(&lock);
		return temp;
	}
	pthread_rwlock_unlock(&lock);
	return 0;
}

vector<struct PlayerDescription> Team::GetPlayerDescriptions()
{
	pthread_rwlock_rdlock(&lock);
	vector<struct PlayerDescription> retVector;
	for( uint i = 0; i < players.size(); i++ )
	{
		retVector.push_back( players[i]->GetDescription() );
	}
	pthread_rwlock_unlock(&lock);

	return retVector;
}

vector <Player*> Team::GetPlayers()
{
	pthread_rwlock_rdlock(&lock);
	vector <Player*> temp = players;
	pthread_rwlock_unlock(&lock);
	return temp;
}

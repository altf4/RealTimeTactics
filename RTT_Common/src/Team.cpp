//============================================================================
// Name        : RTT_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : A team on one side of a match, which contains a collection
//		of players who share a victory condition
//============================================================================


#include "Team.h"

using namespace RTT;
using namespace std;

Team::Team(enum TeamNumber newTeam)
{
	m_team = newTeam;
	pthread_rwlock_init(&m_lock, NULL);
}

Player *Team::GetPlayer(uint playerID)
{
	pthread_rwlock_rdlock(&m_lock);
	for(uint i = 0; i < m_players.size(); i++)
	{
		if( m_players[i]->GetID() == playerID )
		{
			pthread_rwlock_unlock(&m_lock);
			return m_players[i];
		}
	}
	pthread_rwlock_unlock(&m_lock);
	return NULL;
}

bool Team::RemovePlayer(uint playerID)
{
	pthread_rwlock_wrlock(&m_lock);

	vector<Player*>::iterator it = m_players.begin();
	for( ; it != m_players.end(); it++ )
	{
		if( (*it)->GetID() == playerID)
		{
			m_players.erase(it);
			pthread_rwlock_unlock(&m_lock);
			return true;
		}
	}
	pthread_rwlock_unlock(&m_lock);
	return false;
}

void Team::AddPlayer(Player *newPlayer)
{
	pthread_rwlock_wrlock(&m_lock);
	m_players.push_back(newPlayer);
	pthread_rwlock_unlock(&m_lock);
}

uint Team::GetFirstPlayerID()
{
	pthread_rwlock_rdlock(&m_lock);
	if(m_players.size() > 0)
	{
		Player *temp = m_players[0];
		pthread_rwlock_unlock(&m_lock);
		return temp->GetID();
	}
	pthread_rwlock_unlock(&m_lock);
	return 0;
}

vector<struct PlayerDescription> Team::GetPlayerDescriptions()
{
	pthread_rwlock_rdlock(&m_lock);
	vector<struct PlayerDescription> retVector;
	for( uint i = 0; i < m_players.size(); i++ )
	{
		retVector.push_back( m_players[i]->GetDescription() );
	}
	pthread_rwlock_unlock(&m_lock);

	return retVector;
}

vector <Player*> Team::GetPlayers()
{
	pthread_rwlock_rdlock(&m_lock);
	vector <Player*> temp = m_players;
	pthread_rwlock_unlock(&m_lock);
	return temp;
}

string Team::TeamNumberToString(enum TeamNumber teamNum)
{
	switch(teamNum)
	{
		case SPECTATOR:
		{
			return "Spectator";
		}
		case TEAM_1:
		{
			return "Team 1";
		}
		case TEAM_2:
		{
			return "Team 2";
		}
		case TEAM_3:
		{
			return "Team 3";
		}
		case TEAM_4:
		{
			return "Team 4";
		}
		case TEAM_5:
		{
			return "Team 5";
		}
		case TEAM_6:
		{
			return "Team 6";
		}
		case TEAM_7:
		{
			return "Team 7";
		}
		case TEAM_8:
		{
			return "Team 8";
		}
		case REFEREE:
		{
			return "Referee";
		}
	}
	return "WTF, This is not a team";
}

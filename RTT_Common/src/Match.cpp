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
	memset(&m_description, '\0', sizeof(m_description));
	struct timeval tv;
	gettimeofday(&tv,NULL);
	m_timeCreated = tv.tv_sec;
	m_description.m_timeCreated = tv.tv_sec;
	for(uint i = 0; i < MAX_TEAMS; i++)
	{
		m_teams[i] = new Team((enum TeamNumber)i);
	}
	m_leaderID = player->GetID();
	m_description.m_leaderID = m_leaderID;
	m_currentPlayerCount = 0;
	pthread_rwlock_init(&m_lock, NULL);
}

Match::~Match()
{
	pthread_rwlock_wrlock(&m_lock);
	for(uint i = 0; i < MAX_TEAMS; i++)
	{
		if( m_teams[i] != NULL )
		{
			delete m_teams[i];
		}
	}
	pthread_rwlock_unlock(&m_lock);
}

//SET methods
void Match::SetID(uint newID)
{
	pthread_rwlock_wrlock(&m_lock);
	m_ID = newID;
	m_description.m_ID = newID;
	pthread_rwlock_unlock(&m_lock);
}

void Match::SetStatus(enum Status newStatus)
{
	pthread_rwlock_wrlock(&m_lock);
	m_status = newStatus;
	m_description.m_status = newStatus;
	pthread_rwlock_unlock(&m_lock);
}

void Match::SetMaxPlayers(uint newMaxPlayers)
{
	pthread_rwlock_wrlock(&m_lock);
	m_maxPlayers = newMaxPlayers;
	m_description.m_maxPlayers = newMaxPlayers;
	pthread_rwlock_unlock(&m_lock);
}

void Match::SetName(string newName)
{
	pthread_rwlock_wrlock(&m_lock);
	m_name = newName;
	m_name.resize(MAX_MATCHNAME_LEN);
	strncpy(m_description.m_name, newName.c_str(), MAX_MATCHNAME_LEN);
	pthread_rwlock_unlock(&m_lock);
}

void Match::SetMap(struct MapDescription newMap)
{
	pthread_rwlock_wrlock(&m_lock);
	m_map = newMap;
	pthread_rwlock_unlock(&m_lock);
}

void Match::SetVictoryCondition(enum VictoryCondition newVict)
{
	pthread_rwlock_wrlock(&m_lock);
	m_victoryCondition = newVict;
	pthread_rwlock_unlock(&m_lock);
}

void Match::SetGamespeed(enum GameSpeed newSpeed)
{
	pthread_rwlock_wrlock(&m_lock);
	m_gameSpeed = newSpeed;
	pthread_rwlock_unlock(&m_lock);
}

//Returns false if the given ID is not in this match
bool Match::SetLeader(uint newID)
{
	pthread_rwlock_wrlock(&m_lock);
	if( GetPlayer(newID) == NULL)
	{
		pthread_rwlock_unlock(&m_lock);
		return false;
	}
	else
	{
		m_leaderID = newID;
		m_description.m_leaderID = newID;
		pthread_rwlock_unlock(&m_lock);
		return true;
	}
}

//GET methods
enum Status Match::GetStatus()
{
	pthread_rwlock_rdlock(&m_lock);
	enum Status tempStatus = m_status;
	pthread_rwlock_unlock(&m_lock);
	return tempStatus;
}

uint Match::GetID()
{
	pthread_rwlock_rdlock(&m_lock);
	uint tempID = m_ID;
	pthread_rwlock_unlock(&m_lock);
	return tempID;
}

uint Match::GetMaxPlayers()
{
	pthread_rwlock_rdlock(&m_lock);
	uint tempMax = m_maxPlayers;
	pthread_rwlock_unlock(&m_lock);
	return tempMax;
}

uint Match::GetCurrentPlayerCount()
{
	pthread_rwlock_rdlock(&m_lock);
	uint tempCurr = m_currentPlayerCount;
	pthread_rwlock_unlock(&m_lock);
	return tempCurr;
}

string Match::GetName()
{
	pthread_rwlock_rdlock(&m_lock);
	string tempName = m_name;
	pthread_rwlock_unlock(&m_lock);
	return tempName;
}

uint Match::GetLeaderID()
{
	pthread_rwlock_rdlock(&m_lock);
	uint tempID = m_leaderID;
	pthread_rwlock_unlock(&m_lock);
	return tempID;
}

struct MatchDescription Match::GetDescription()
{
	pthread_rwlock_rdlock(&m_lock);
	struct MatchDescription tempDesc = m_description;
	pthread_rwlock_unlock(&m_lock);
	return tempDesc;
}

struct MapDescription Match::GetMap()
{
	pthread_rwlock_rdlock(&m_lock);
	struct MapDescription tempMap = m_map;
	pthread_rwlock_unlock(&m_lock);
	return tempMap;
}

enum VictoryCondition Match::GetVictoryCondition()
{
	pthread_rwlock_rdlock(&m_lock);
	enum VictoryCondition tempVict = m_victoryCondition;
	pthread_rwlock_unlock(&m_lock);
	return tempVict;
}

enum GameSpeed Match::GetGamespeed()
{
	pthread_rwlock_rdlock(&m_lock);
	enum GameSpeed tempSpeed = m_gameSpeed;
	pthread_rwlock_unlock(&m_lock);
	return tempSpeed;
}

bool Match::AddPlayer(Player *player, enum TeamNumber teamNum)
{
	pthread_rwlock_rdlock(&m_lock);
	if( m_currentPlayerCount >= m_maxPlayers )
	{
		pthread_rwlock_unlock(&m_lock);
		return false;
	}
	if( teamNum > REFEREE)
	{
		pthread_rwlock_unlock(&m_lock);
		return false;
	}
	pthread_rwlock_unlock(&m_lock);

	m_teams[teamNum]->AddPlayer(player);

	pthread_rwlock_wrlock(&m_lock);
	m_currentPlayerCount++;
	m_description.m_currentPlayerCount++;
	pthread_rwlock_unlock(&m_lock);

	return true;
}

bool Match::RemovePlayer( uint playerID )
{

	for(uint i = 0; i < MAX_TEAMS; i++)
	{
		if( m_teams[i]->RemovePlayer(playerID))
		{
			uint nextID = GetFirstPlayerID();
			pthread_rwlock_wrlock(&m_lock);
			if( m_leaderID == playerID )
			{
				m_leaderID = nextID;
				m_description.m_leaderID = nextID;
			}
			m_currentPlayerCount--;
			m_description.m_currentPlayerCount--;
			pthread_rwlock_unlock(&m_lock);

			return true;
		}
	}
	return false;
}

Player* Match::GetPlayer( uint playerID )
{
	for(uint i = 0; i < MAX_TEAMS; i++)
	{
		Player *player = m_teams[i]->GetPlayer(playerID);
		if( player != NULL)
		{
			return player;
		}
	}
	return NULL;
}

//Get the first Player in the teams lists
//	For use in getting the next leader when one leaves
//	Returns 0 if there are no more players
uint Match::GetFirstPlayerID()
{
	for(uint i = 0; i < MAX_TEAMS; i++)
	{
		uint retID = m_teams[i]->GetFirstPlayerID();
		if( retID > 0 )
		{
			return retID;
		}
	}
	return 0;
}

bool Match::ChangeTeam(Player *player, enum TeamNumber newTeam)
{
	if( newTeam > REFEREE)
	{
		return false;
	}
	//TODO: Probably don't need to loop through all teams. The player keeps a record
	//		of what the old team number is
	for(uint i = 0; i < MAX_TEAMS; i++)
	{
		if( m_teams[i]->RemovePlayer(player->GetID()) == true)
		{
			m_teams[newTeam]->AddPlayer(player);
			player->SetTeam(newTeam);
			return true;
		}
	}
	return false;
}

bool Match::StartMatch()
{
	return true;
}

string Match::GameSpeedToString(enum GameSpeed speed)
{
	switch(speed)
	{
		case SPEED_SLOW:
		{
			return "Slow";
		}
		case SPEED_NORMAL:
		{
			return "Normal";
		}
		case SPEED_FAST:
		{
			return "Fast";
		}
		case SPEED_FASTEST:
		{
			return "Fastest";
		}
		default:
		{
			return "WTF, this is not a speed";
		}
	}
}

//Converts a GameSpeed enum into the number of microseconds between game ticks
uint Match::GameSpeedTouSeconds(enum GameSpeed speed)
{
	switch(speed)
	{
		case SPEED_SLOW:
		{
			return 1000000;
		}
		case SPEED_NORMAL:
		{
			return 844000;
		}
		case SPEED_FAST:
		{
			return 666000;
		}
		case SPEED_FASTEST:
		{
			return 500000;
		}
		default:
		{
			return 844000;
		}
	}
}

string Match::VictoryConditionToString(enum VictoryCondition victory)
{
	switch(victory)
	{
		case DEATHMATCH:
		{
			return "Deathmatch";
		}
		case CAPTURE_THE_FLAG:
		{
			return "Capture the Flag";
		}
		case KING_OF_THE_HILL:
		{
			return "King of the Hill";
		}
		default:
		{
			return "WTF, this is not a victory condition";
		}
	}
}

//Register that the given player is ready to start the match
//	returns - true if the player is the last one in
//		IE: This fact is important for the server to spawn a match loop thread
bool Match::RegisterPlayer(uint playerID)
{
	pthread_rwlock_wrlock(&m_lock);

	m_registeredPlayers.push_back(playerID);

	//If now full:
	if(m_registeredPlayers.size() == m_currentPlayerCount)
	{
		pthread_rwlock_unlock(&m_lock);
		return true;
	}

	pthread_rwlock_unlock(&m_lock);
	return false;


}

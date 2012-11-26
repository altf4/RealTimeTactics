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
	Lock lock(&m_lock, WRITE_LOCK);
	for(uint i = 0; i < MAX_TEAMS; i++)
	{
		if( m_teams[i] != NULL )
		{
			delete m_teams[i];
		}
	}
}

//SET methods
void Match::SetID(uint newID)
{
	Lock lock(&m_lock, WRITE_LOCK);
	m_ID = newID;
	m_description.m_ID = newID;
}

void Match::SetStatus(enum Status newStatus)
{
	Lock lock(&m_lock, WRITE_LOCK);
	m_status = newStatus;
	m_description.m_status = newStatus;
}

void Match::SetMaxPlayers(uint newMaxPlayers)
{
	Lock lock(&m_lock, WRITE_LOCK);
	m_maxPlayers = newMaxPlayers;
	m_description.m_maxPlayers = newMaxPlayers;
}

void Match::SetName(string newName)
{
	Lock lock(&m_lock, WRITE_LOCK);
	m_name = newName;
	m_name.resize(MAX_MATCHNAME_LEN);
	strncpy(m_description.m_name, newName.c_str(), MAX_MATCHNAME_LEN);
}

void Match::SetMap(struct MapDescription newMap)
{
	Lock lock(&m_lock, WRITE_LOCK);
	m_map = newMap;
}

void Match::SetVictoryCondition(enum VictoryCondition newVict)
{
	Lock lock(&m_lock, WRITE_LOCK);
	m_victoryCondition = newVict;
}

void Match::SetGamespeed(enum GameSpeed newSpeed)
{
	Lock lock(&m_lock, WRITE_LOCK);
	m_gameSpeed = newSpeed;
}

//Returns false if the given ID is not in this match
bool Match::SetLeader(uint newID)
{
	Lock lock(&m_lock, WRITE_LOCK);
	if( GetPlayer(newID) == NULL)
	{
		return false;
	}
	else
	{
		m_leaderID = newID;
		m_description.m_leaderID = newID;
		return true;
	}
}

//GET methods
enum Status Match::GetStatus()
{
	Lock lock(&m_lock, READ_LOCK);
	return m_status;
}

uint Match::GetID()
{
	Lock lock(&m_lock, READ_LOCK);
	return m_ID;
}

uint Match::GetMaxPlayers()
{
	Lock lock(&m_lock, READ_LOCK);
	return m_maxPlayers;
}

uint Match::GetCurrentPlayerCount()
{
	Lock lock(&m_lock, READ_LOCK);
	return m_currentPlayerCount;
}

string Match::GetName()
{
	Lock lock(&m_lock, READ_LOCK);
	return m_name;
}

uint Match::GetLeaderID()
{
	Lock lock(&m_lock, READ_LOCK);
	return m_leaderID;
}

struct MatchDescription Match::GetDescription()
{
	Lock lock(&m_lock, READ_LOCK);
	return m_description;
}

struct MapDescription Match::GetMap()
{
	Lock lock(&m_lock, READ_LOCK);
	return m_map;
}

enum VictoryCondition Match::GetVictoryCondition()
{
	Lock lock(&m_lock, READ_LOCK);
	return m_victoryCondition;
}

enum GameSpeed Match::GetGamespeed()
{
	Lock lock(&m_lock, READ_LOCK);
	return m_gameSpeed;
}

bool Match::AddPlayer(Player *player, enum TeamNumber teamNum)
{
	{
		Lock lock(&m_lock, WRITE_LOCK);
		if( m_currentPlayerCount >= m_maxPlayers )
		{
			return false;
		}
		if( teamNum > REFEREE)
		{
			return false;
		}
	}
	m_teams[teamNum]->AddPlayer(player);
	{
		Lock lock(&m_lock, WRITE_LOCK);
		m_currentPlayerCount++;
		m_description.m_currentPlayerCount++;
	}

	return true;
}

bool Match::RemovePlayer( uint playerID )
{

	for(uint i = 0; i < MAX_TEAMS; i++)
	{
		if( m_teams[i]->RemovePlayer(playerID))
		{
			uint nextID = GetFirstPlayerID();
			Lock lock(&m_lock, WRITE_LOCK);
			if( m_leaderID == playerID )
			{
				m_leaderID = nextID;
				m_description.m_leaderID = nextID;
			}
			m_currentPlayerCount--;
			m_description.m_currentPlayerCount--;
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
	Lock lock(&m_lock, WRITE_LOCK);
	m_registeredPlayers.push_back(playerID);

	//If now full:
	if(m_registeredPlayers.size() == m_currentPlayerCount)
	{
		return true;
	}

	return false;
}

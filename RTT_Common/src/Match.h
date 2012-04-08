//============================================================================
// Name        : RTT_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : A single match (or game) that is being played.
//					A server can have many at one time
//============================================================================

#ifndef MATCH_H_
#define MATCH_H_

#include "Gameboard.h"
#include "Team.h"
#include "Action.h"
#include "Player.h"
#include "Map.h"
#include "Enums.h"
#include <sys/time.h>

#define MAX_MATCHNAME_LEN 20
#define MATCH_DESCR_SIZE sizeof(enum Status) + (sizeof(uint32_t)*4) + \
		MAX_MATCHNAME_LEN + sizeof(int64_t)
#define MATCH_OPTIONS_SIZE sizeof(uint32_t) + MAX_MATCHNAME_LEN

namespace RTT
{
//Forward declaration to avoid self-reference
class Team;
class Player;

enum Status: uint32_t
{
	WAITING_FOR_PLAYERS,
	IN_PROGRESS,
};

//A fixed size description of the match
//	Suitable for sending to others to give match info
struct MatchDescription
{
	enum Status m_status;
	uint32_t m_ID;
	uint32_t m_maxPlayers;
	uint32_t m_currentPlayerCount;
	char m_name[MAX_MATCHNAME_LEN];
	int64_t m_timeCreated;
	uint32_t m_leaderID;
};

//A fixed size collection of options for match creation
//	List of options necessary upon creation of the Match
struct MatchOptions
{
	uint32_t m_maxPlayers;
	char m_name[MAX_MATCHNAME_LEN];
};

class Match
{

public:
	//The gameboard for this match
	Gameboard *m_gameboard;

	//Teams involved
	Team* m_teams[MAX_TEAMS];

	// chargingActions is the global list of actions which have not yet been triggered
	// chargedActions is the list of actions which are on queue to be triggered
	std::vector <Action*> m_chargingActions, m_chargedActions;

	Match(Player *player);
	~Match();

	void SetID(uint newID);
	void SetStatus(enum Status newStatus);
	void SetMaxPlayers(uint maxPlayers);
	void SetName(std::string newName);
	void SetLeaderID(uint nextLeader);
	void SetMap(struct MapDescription newMap);
	void SetVictoryCondition(enum VictoryCondition newVict);
	void SetGamespeed(enum GameSpeed newSpeed);
	//Returns false if the given ID is not in this match
	bool SetLeader(uint newID);

	enum Status GetStatus();
	uint GetID();
	uint GetMaxPlayers();
	uint GetCurrentPlayerCount();
	std::string GetName();
	time_t GetTimeCreated();
	uint GetLeaderID();
	struct MatchDescription GetDescription();
	struct MapDescription GetMap();
	enum VictoryCondition GetVictoryCondition();
	enum GameSpeed GetGamespeed();

	bool AddPlayer(Player *player, enum TeamNumber teamNum);
	bool RemovePlayer( uint playerID );
	Player *GetPlayer( uint playerID );
	bool ChangeTeam(Player *player, enum TeamNumber newTeam);
	bool StartMatch();

	//TODO: Evaluate if this is the best place for these functions.
	static std::string GameSpeedToString(enum GameSpeed speed);
	static std::string VictoryConditionToString(enum VictoryCondition victory);

private:
	//Lock for this match
	pthread_rwlock_t m_lock;

	//The current status of the match
	enum Status m_status;
	//Globally unique identifier for the match on this server
	uint32_t m_ID;
	//Maximum number of players allowed for this match
	uint32_t m_maxPlayers;
	uint32_t m_currentPlayerCount;
	//Truncated at MAX_MATCHNAME_LEN chars
	std::string m_name;
	int64_t m_timeCreated;

	//The "Leader" player who can make changes to match settings
	uint m_leaderID;
	struct MatchDescription m_description;
	//Map to be used in the match
	//TODO: Make this a full map object
	struct MapDescription m_map;
	enum VictoryCondition m_victoryCondition;
	enum GameSpeed m_gameSpeed;

	uint GetFirstPlayerID();

};

}

#endif /* MATCH_H_ */

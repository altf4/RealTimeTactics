 //============================================================================
// Name        : Match.h
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

namespace RTT
{

class Match
{

public:

	//Teams involved
	Team* m_teams[MAX_TEAMS];

	// chargingActions is the global list of actions which have not yet been triggered
	// chargedActions is the list of actions which are on queue to be triggered
	std::vector <Action*> m_chargingActions, m_chargedActions;

	Match(Player *player);
	~Match();


	//*******************************
	//	Public Getters and Setters
	//*******************************
	void SetID(uint newID);
	void SetStatus(enum MatchStatus newStatus);
	void SetMaxPlayers(uint maxPlayers);
	void SetName(std::string newName);
	void SetLeaderID(uint nextLeader);
	void SetMap(struct MapDescription newMap);
	void SetVictoryCondition(enum VictoryCondition newVict);
	void SetGamespeed(enum GameSpeed newSpeed);
	//Returns false if the given ID is not in this match
	bool SetLeader(uint newID);

	enum MatchStatus GetStatus();
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


	//*****************************
	// Match Progress Actions
	//*****************************

	//Starts the match (in a new thread)
	// returns - true
	bool Start();

	//Stops the match.
	//	returns - The winning team. In the case of a draw or no contest (game ended before a team won), then SPECTATOR is returned
	enum TeamNumber Stop();
	bool Pause();
	bool Unpause();

	bool Surrender(uint32_t losingPlayerID);

	//Register that the given player is ready to start the match
	//	returns - true if the player is the last one in
	//		IE: This fact is important for the server to spawn a match loop thread
	bool RegisterPlayer(uint playerID);

	//TODO: Evaluate if this is the best place for these functions.
	static std::string GameSpeedToString(enum GameSpeed speed);
	//Converts a GameSpeed enum into the number of microseconds between game ticks
	static uint GameSpeedTouSeconds(enum GameSpeed speed);
	static std::string VictoryConditionToString(enum VictoryCondition victory);

	void *MatchLoop();

private:

	uint GetFirstPlayerID();

	void TimerTick();

	bool IsVictoryCondSatisfied();

	//Lock for this match
	pthread_rwlock_t m_lock;

	//The current status of the match
	enum MatchStatus m_status;
	//Globally unique identifier for the match on this server
	uint32_t m_ID;
	//Maximum number of players allowed for this match
	uint32_t m_maxPlayers;
	uint32_t m_currentPlayerCount;
	//Truncated at MAX_MATCHNAME_LEN chars
	std::string m_name;
	int64_t m_timeCreated;
	vector<uint32_t> m_registeredPlayers;

	//The "Leader" player who can make changes to match settings
	uint m_leaderID;
	struct MatchDescription m_description;
	//Map to be used in the match
	//TODO: Make this a full map object
	struct MapDescription m_map;
	enum VictoryCondition m_victoryCondition;
	enum GameSpeed m_gameSpeed;

	pthread_t m_thread;

	Gameboard *m_gameboard;

};

}

#endif /* MATCH_H_ */

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
#define MATCH_DESCR_SIZE sizeof(enum Status) + (sizeof(uint)*3) + MAX_MATCHNAME_LEN + sizeof(time_t)
#define MATCH_OPTIONS_SIZE sizeof(uint)
using namespace std;

namespace RTT
{
//Forward declaration to avoid self-reference
class Team;
class Player;

enum Status
{
	WAITING_FOR_PLAYERS,
	IN_PROGRESS,
};

//A fixed size description of the match
//	Suitable for sending to others to give match info
struct MatchDescription
{
	enum Status status;
	uint ID;
	uint maxPlayers;
	uint currentPlayerCount;
	char name[MAX_MATCHNAME_LEN];
	time_t timeCreated;
};

//A fixed size collection of options for match creation
//	List of options necessary upon creation of the Match
struct MatchOptions
{
	uint maxPlayers;
};

class Match
{

public:
	//The gameboard for this match
	Gameboard *gameboard;

	//Teams involved
	Team* teams[MAX_TEAMS];

	//The "Leader" player who can make changes to match settings
	Player *leader;

	struct MatchDescription description;

	//Map to be used in the match
	//TODO: Make this a full map object
	struct MapDescription map;

	enum VictoryCondition victoryCondition;

	enum GameSpeed gameSpeed;

	// chargingActions is the global list of actions which have not yet been triggered
	// chargedActions is the list of actions which are on queue to be triggered
	vector <Action*> chargingActions, chargedActions;

	Match(Player *player);
	~Match();

	void SetID(uint newID);
	void SetStatus(enum Status newStatus);
	void SetMaxPlayers(uint maxPlayers);
	void SetName(string newName);

	enum Status GetStatus();
	uint GetID();
	uint GetMaxPlayers();
	uint GetCurrentPlayerCount();
	string GetName();
	time_t GetTimeCreated();

	bool AddPlayer(Player *player, enum TeamNumber teamNum);
	bool RemovePlayer( uint playerID );
	Player *GetPlayer( uint playerID );
	bool ChangeTeam(uint playerID, enum TeamNumber newTeam);
	bool StartMatch();

private:

	//The current status of the match
	enum Status status;
	//Globally unique identifier for the match on this server
	uint ID;
	//Maximum number of players allowed for this match
	uint maxPlayers;
	uint currentPlayerCount;
	//Truncated at MAX_MATCHNAME_LEN chars
	string name;
	time_t timeCreated;

	Player *GetFirstPlayer();

};

}

#endif /* MATCH_H_ */

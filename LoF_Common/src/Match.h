//============================================================================
// Name        : LoF_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : A single match (or game) that is being played.
//					A server can have many at one time
//============================================================================

#ifndef MATCH_H_
#define MATCH_H_

#include "Gameboard.h"
#include "Player.h"
#include "Action.h"

using namespace std;

namespace LoF
{

//Forward declaration to avoid self-reference
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

	//Players involved
	vector <Player*> players;

	struct MatchDescription description;

	// chargingActions is the global list of actions which have not yet been triggered
	// chargedActions is the list of actions which are on queue to be triggered
	vector <Action*> chargingActions, chargedActions;

	Match();

	void SetID(uint newID);
	void SetStatus(enum Status newStatus);
	void SetMaxPlayers(uint maxPlayers);

	enum Status GetStatus();
	uint GetID();
	uint GetMaxPlayers();

private:

	//The current status of the match
	enum Status status;
	//Globally unique identifier for the match on this server
	uint ID;
	//Maximum number of players allowed for this match
	uint maxPlayers;

};

}

#endif /* MATCH_H_ */

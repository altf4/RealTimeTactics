//============================================================================
// Name        : ClientGameState.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Class which manages and encapsulates the current game state
//	from the perspective of this client. IE: May contain incomplete information
//	compared to the server's GameState
//============================================================================

#ifndef CLIENTGAMESTATE_H_
#define CLIENTGAMESTATE_H_

#include "Gameboard.h"
#include "messaging/messages/GameMessage.h"
#include "Player.h"

#include "pthread.h"

namespace RTT
{

class ClientGameState
{

public:

	//NOTE: All functions in this class are threadsafe. So PLEASE: have as many threads accessing
	//	the game state as possible. It will speed things up.

	static ClientGameState &Instance();

	//********************************************
	//				Query Functions
	//********************************************

	//NOTE: If the Unit does not exist, returns NULL
	Unit *GetUnit(uint32_t ID);
	bool HasUnit(uint32_t ID);

	//Returns a list of Unit ID's that are currently used in the game
	//	NOTE: It is entirely possible that a new unit is added by another
	//	thread immediately after calling this function. So the list may
	//	always be out of date by the time you get it. Deal with it.
	vector<uint32_t> GetUnits();

	//NOTE: If the player doesn't exist, returns NULL
	Player *GetPlayer(uint32_t ID);
	bool HasPlayer(uint32_t ID);

	//Returns a list of Player ID's that are currently in the game
	//	NOTE: It is entirely possible that a new Player is added by another
	//	thread immediately after calling this function. So the list may
	//	always be out of date by the time you get it. Deal with it.
	vector<uint32_t> GetPlayers();

	uint32_t GetOurPlayerID();
	void SetOurPlayerID(uint32_t ID);

	//********************************************
	//				Modify Functions
	//********************************************

	//Adds a new unit to the game state
	//	newUnit - A copy of the Unit to add
	//	returns - True if the Unit was added successfully, false on error
	bool AddUnit(Unit *newUnit);

	//Add a new Player to the game state
	//	returns - True on success, false if player already exists
	bool AddPlayer(Player *newPlayer);

	//Removes Player with the given ID
	//	NOTE: Safely does nothing if player doesn't exist
	void RemovePlayer(uint32_t ID);

	//Move a Unit to a new specified (possibly distant) tile
	//	unitID - The ID of the unit moved
	//	source - The Coordinate of the tile being moved from
	//	destination - The Coordinate of the tile being moved to
	enum MoveResult MoveUnitDistant(uint32_t unitID, struct Coordinate source,
			struct Coordinate destination);

	//Move a Unit to a new adjacent tile
	//	unitID - The ID of the unit moved
	//	source - The Coordinate of the tile being moved from
	//	direction - The direction to move the Unit
	//	facing - The direction to have the Unit face at the end of the move
	enum MoveResult MoveUnitDirection(uint32_t unitID, struct Coordinate source,
			enum Direction direction, enum Direction facing);

	//Clear all game data and start over again
	//WARNING: This is not (yet) threadsafe. Make sure no other threads are using
	//	And Unit objects before this is called. Since this will delete those
	//	objects out from underneath them
	void Reset();

private:

	//Removes unit with the given ID
	//	NOTE: Safely does nothing if unit doesn't exist
	void RemoveUnit(uint32_t ID);

	static ClientGameState *m_instance;

	Gameboard m_gameboard;

	vector <Unit*> m_units;			//TODO: Maybe use a hash map?
	pthread_mutex_t m_unitsLock;	//Lock on reading/writing from the units list

	vector <Player*> m_players;		//TODO: Maybe use a hash map?
	pthread_mutex_t m_playersLock;	//Lock on reading/writing from the players list

	uint32_t m_ourPlayerID;

	//Private constructor, since you should just use Instance()
	ClientGameState();
};

}

#endif /* CLIENTGAMESTATE_H_ */

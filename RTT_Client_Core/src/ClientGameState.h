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

	//NOTE: If the Unit does not exist, the returned Unit will have ID set to 0
	Unit GetUnit(uint32_t ID);
	bool HasUnit(uint32_t ID);


	//********************************************
	//				Set Functions
	//********************************************

	//Adds a new unit to the game state
	//	newUnit - A copy of the Unit to add
	//	returns - True if the Unit was added successfully, false on error
	bool AddUnit(Unit newUnit);

	//Move a Unit to a new specified (possibly distant) tile
	//	unitID - The ID of the unit moved
	//	source - The Coordinate of the tile being moved from
	//	destination - The Coordinate of the tile being moved to
	//	facing - The direction to have the Unit face at the end of the move
	enum MoveResult MoveUnit(uint32_t unitID, struct Coordinate source,
			struct Coordinate destination, enum Direction facing);


	//Move a Unit to a new adjacent tile
	//	unitID - The ID of the unit moved
	//	source - The Coordinate of the tile being moved from
	//	direction - The direction to move the Unit
	//	facing - The direction to have the Unit face at the end of the move
	enum MoveResult MoveUnitDirection(uint32_t unitID, struct Coordinate source,
			enum Direction direction, enum Direction facing);

	//Clear all game data and start over again
	void Reset();


	//********************************************
	//				Synchronization Functions
	//********************************************

	//Concurrency functions for use when the Unit is in a list
	//If the returned Unit's ID is zero, then there was an error and no lock was set.
	//	So don't try checking it back in
	Unit CheckOutUnit(uint32_t ID);
	void CheckInUnit(Unit newUnit);


private:

	static ClientGameState *m_instance;

	Gameboard m_gameboard;

	vector <Unit> m_units;			//TODO: Maybe use a hash map?
	pthread_mutex_t m_unitsLock;	//Lock on reading/writing from the units list

	//Private constructor, since you should just use Instance()
	ClientGameState();
};

}

#endif /* CLIENTGAMESTATE_H_ */

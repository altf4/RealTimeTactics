//============================================================================
// Name        : ClientGameState.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Class which manages and encapsulates the current game state
//	from the perspective of this client. IE: May contain incomplete information
//	compared to the server's GameState
//============================================================================

#include "ClientGameState.h"
#include "Lock.h"

using namespace RTT;

//Initialize the pointer
ClientGameState *ClientGameState::m_instance = NULL;

ClientGameState &ClientGameState::Instance()
{
	if(m_instance == NULL)
	{
		m_instance = new ClientGameState();
	}
	return *m_instance;
}


ClientGameState::ClientGameState()
	:m_gameboard(10, 10)				//TODO: Un-harcode the gameboard size!!!
{
	pthread_mutex_init(&m_unitsLock, NULL);
}

//Adds a new unit to the game state
//	newUnit - A copy of the Unit to add
//	returns - True if the Unit was added successfully, false on error
bool ClientGameState::AddUnit(Unit *newUnit)
{
	if(newUnit == NULL)
	{
		return false;
	}

	//If the unit ID is already here, then don't add a new one
	if(HasUnit(newUnit->m_ID))
	{
		return false;
	}

	Lock lock(&m_unitsLock);
	m_units.push_back(newUnit);
	return true;
}

Unit *ClientGameState::GetUnit(uint32_t ID)
{
	Lock lock(&m_unitsLock);
	for(uint i = 0; i < m_units.size(); i++)
	{
		if(m_units[i]->m_ID == ID)
		{
			return m_units[i];
		}
	}

	return NULL;
}

bool ClientGameState::HasUnit(uint32_t ID)
{
	bool successResult = false;
	Lock lock(&m_unitsLock);

	for(uint i = 0; i < m_units.size(); i++)
	{
		if(m_units[i]->m_ID == ID)
		{
			successResult = true;
			break;
		}
	}
	return successResult;
}

//A Unit has moved
//	unitID - The ID of the unit moved
//	source - The Coordinate of the tile being moved from
//	destination - The Coordinate of the tile being moved to
//	returns - True on successfully moving unit, false on error
enum MoveResult ClientGameState::MoveUnitDistant(uint32_t unitID, struct Coordinate source, struct Coordinate destination)
{
	Unit *movingUnit = GetUnit(unitID);
	if(movingUnit == NULL)
	{
		return MOVE_NO_SUCH_UNIT;
	}
	Lock lock = movingUnit->LockUnit();

	//If the unit isn't at the source we expected...
	if((movingUnit->m_x != source.m_x) || (movingUnit->m_y != source.m_y))
	{
		return MOVE_WRONG_SOURCE;
	}

	//TODO: Legality checks on the movement

	movingUnit->m_x = destination.m_x;
	movingUnit->m_y = destination.m_y;

	return MOVE_SUCCESS;
}

//Move a Unit to a new adjacent tile
//	unitID - The ID of the unit moved
//	source - The Coordinate of the tile being moved from
//	direction - The direction to move the Unit
//	facing - The direction to have the Unit face at the end of the move
enum MoveResult ClientGameState::MoveUnitDirection(uint32_t unitID, struct Coordinate source,
		enum Direction direction, enum Direction facing)
{
	Unit *movingUnit = GetUnit(unitID);
	if(movingUnit == NULL)
	{
		return MOVE_NO_SUCH_UNIT;
	}
	Lock lock = movingUnit->LockUnit();


	//If the unit isn't at the source we expected...
	if((movingUnit->m_x != source.m_x) || (movingUnit->m_y != source.m_y))
	{
		return MOVE_WRONG_SOURCE;
	}

	//TODO: Legality checks on the movement

	switch(direction)
	{
		case NORTHEAST:
		{
			//if odd
			if((movingUnit->m_y % 2) != 0)
			{
				movingUnit->m_x++;
			}
			movingUnit->m_y++;
			break;
		}
		case NORTHWEST:
		{
			//if even
			if((movingUnit->m_y % 2) == 0)
			{
				movingUnit->m_x--;
			}
			movingUnit->m_y++;
			break;
		}
		case WEST:
		{
			movingUnit->m_x--;
			break;
		}
		case SOUTHWEST:
		{
			//if even
			if((movingUnit->m_y % 2) == 0)
			{
				movingUnit->m_x--;
			}
			movingUnit->m_y--;
			break;
		}
		case SOUTHEAST:
		{
			//if odd
			if((movingUnit->m_y % 2) != 0)
			{
				movingUnit->m_x++;
			}
			movingUnit->m_y--;
			break;
		}
		case EAST:
		{
			movingUnit->m_x++;
			break;
		}
	}

	movingUnit->m_directionFacing = facing;

	return MOVE_SUCCESS;
}

void ClientGameState::Reset()
{
	Lock lock(&m_unitsLock);
	for(uint i = 0; i < m_units.size(); i++)
	{
		delete m_units[i];
	}
	m_units.clear();
}

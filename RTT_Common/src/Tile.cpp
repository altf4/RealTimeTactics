//============================================================================
// Name        : RTT_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : One Hex on the gameboard
//============================================================================

#include <iostream>
#include "Tile.h"
#include "exceptions/IllegalMoveException.h"


using namespace std;
using namespace RTT;

Tile::Tile(uint x_arg, uint y_arg)
{
	x = x_arg;
	y = y_arg;
	isPassable = true;
}

//Moves the Unit (who is currently on this tile) to the specified tile
void Tile::MoveUnitTo(Unit *unit, Tile *tile)
{
	int tileIndex = this->IndexOfUnit( unit );
	if( tileIndex != -1)
	{
		//TODO: Find the shortest path

		tile->unitsPresent.push_back(unit);
		//Null it out first, so that erase doesn't call the destructor
		unitsPresent[tileIndex] = NULL;
		unitsPresent.erase( unitsPresent.begin()+tileIndex );
	}
	else
	{
		cerr << "Error: Unit '" << unit->ID << "' not present on tile ("
				<< x << ", " << y << ")\n";
		throw IllegalMoveException("Unit not present on tile to be moved from.");
	}

}

//Checks if the given unit is standing on this tile
//Returns the index of the Unit in the Tile's list
//Returns -1 if it's not there
int Tile::IndexOfUnit(Unit *unit)
{
	for( uint i = 0; i < unitsPresent.size(); i++ )
	{
		if( unitsPresent[i]->Equals(unit) )
		{
			return i;
		}
	}
	return -1;
}


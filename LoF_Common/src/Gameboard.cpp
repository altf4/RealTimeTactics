//============================================================================
// Name        : LoF_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Representation of the hex gameboard
//============================================================================

#include "Gameboard.h"
#include <iostream>
#include "InvalidGameboardException.h"
#include "IllegalMoveException.h"

using namespace std;
using namespace LoF;

Gameboard::Gameboard(uint x_arg, uint y_arg)
{
	//The gameboard must at least be 2x2
	if( x_arg < 2 || y_arg < 2)
	{
		cerr << "ERROR: Invalid gameboard size specified. Too small.\n";
		throw InvalidGameboardException(
				"ERROR: Invalid gameboard size specified. Too small.");
		return;
	}

	xMax = x_arg;
	yMax = y_arg;

	//Create the gameboard tiles
	for(uint i = 0; i < xMax; i++)
	{
		for(uint j = 0; j < yMax; j++)
		{
			tiles[i][j] = new Tile(i,j);
			tiles[i][j]->ID = (j*xMax) + i;
		}
	}
}

//Return the tile specified by the given coordinates
//Return NULL if there is no such tile
Tile *Gameboard::GetTile(int x, int y)
{
	if(x < 0 || y < 0)
	{
		return NULL;
	}
	if( (uint)x > xMax || (uint)y > yMax )
	{
		return NULL;
	}

	return tiles[x][y];

}

//Considering this board to be a directional graph,
//	How many edges are there?
uint Gameboard::NumEdges(Unit *unit)
{
	uint count = 0;
	//For every tile...
	for(uint i = 0; i < xMax; i++)
	{
		for(uint j = 0; j < yMax; j++)
		{
			//Accumulate how many outgoing edges there are
			if( GetMovementCost(tiles[i][j], NORTHEAST, unit ) != -1)
			{
				count++;
			}
		}
	}

	return count;
}

//Returns the weight (IE: Movement cost) of moving in the given direction
//	Returns -1 when impassable.
double Gameboard::GetMovementCost(Tile *fromTile, direction dir, Unit *unit)
{
	//If this tile is impassable, then don't bother even looking
	if( !fromTile->isPassable )
	{
		return -1;
	}

	//Set offsets for moving North and South
	//(Movement north and south gets weird in a hex map)

	//0 when even, 1 when odd
	uint yValEvenOdd = fromTile->y % 2;
	int eastXOffset, westXOffset;
	//Even row case
	if( yValEvenOdd == 0)
	{
		eastXOffset = 0;
		westXOffset = -1;
	}
	//Odd row case
	else
	{
		eastXOffset = 1;
		westXOffset = 0;
	}

	if( dir == NORTHEAST )
	{
		Tile *toTile = GetTile(fromTile->x+eastXOffset, fromTile->y+1);
		if( toTile == NULL)
		{
			return -1;
		}
		else if( toTile->isPassable == false )
		{
			return -1;
		}
		else if( unit->verticalMovement <
				ElevationDiff( fromTile->elevation, toTile->elevation) )
		{
			return -1;
		}
		else
		{
			return toTile->movementCost;
		}

	}
	if( dir == NORTHWEST )
	{
		Tile *toTile = GetTile(fromTile->x+westXOffset, fromTile->y+1);
		if( toTile == NULL)
		{
			return -1;
		}
		else if( toTile->isPassable == false )
		{
			return -1;
		}
		else if( unit->verticalMovement <
				ElevationDiff( fromTile->elevation, toTile->elevation) )
		{
			return -1;
		}
		else
		{
			return toTile->movementCost;
		}

	}
	if( dir == WEST )
	{
		Tile *toTile = GetTile( (int)(fromTile->x)-1, fromTile->y);
		if( toTile == NULL)
		{
			return -1;
		}
		else if( toTile->isPassable == false )
		{
			return -1;
		}
		else if( unit->verticalMovement <
				ElevationDiff( fromTile->elevation, toTile->elevation) )
		{
			return -1;
		}
		else
		{
			return toTile->movementCost;
		}

	}
	if( dir == SOUTHWEST )
	{
		Tile *toTile = GetTile(fromTile->x+westXOffset, fromTile->y-1);
		if( toTile == NULL)
		{
			return -1;
		}
		else if( toTile->isPassable == false )
		{
			return -1;
		}
		else if( unit->verticalMovement <
				ElevationDiff( fromTile->elevation, toTile->elevation) )
		{
			return -1;
		}
		else
		{
			return toTile->movementCost;
		}
	}
	if( dir == SOUTHEAST )
	{
		Tile *toTile = GetTile(fromTile->x+eastXOffset, fromTile->y-1);
		if( toTile == NULL)
		{
			return -1;
		}
		else if( toTile->isPassable == false )
		{
			return -1;
		}
		else if( unit->verticalMovement <
				ElevationDiff( fromTile->elevation, toTile->elevation) )
		{
			return -1;
		}
		else
		{
			return toTile->movementCost;
		}
	}
	if( dir == EAST )
	{
		Tile *toTile = GetTile(fromTile->x+1, fromTile->y);
		if( toTile == NULL)
		{
			return -1;
		}
		else if( toTile->isPassable == false )
		{
			return -1;
		}
		else if( unit->verticalMovement <
				ElevationDiff( fromTile->elevation, toTile->elevation) )
		{
			return -1;
		}
		else
		{
			return toTile->movementCost;
		}
	}

	throw IllegalMoveException("Tried to move in a direction not one of the valid 6.");
	return -1;

}

uint ElevationDiff(uint elevation1, uint elevation2)
{
	if( elevation1 > elevation2)
	{
		return (elevation1 - elevation2);
	}
	else
	{
		return (elevation2 - elevation1);
	}
}

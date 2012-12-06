//============================================================================
// Name        : RTT_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Representation of the hex gameboard
//============================================================================

#include "Gameboard.h"
#include <iostream>
#include "exceptions/InvalidGameboardException.h"
#include "exceptions/IllegalMoveException.h"

using namespace std;
using namespace RTT;

Gameboard::Gameboard(uint x_arg, uint y_arg)
{
	//The gameboard must at least be 2x2
	if( x_arg < 2 || y_arg < 2)
	{
		cerr << "ERROR: Invalid gameboard size specified. Too small.\n";
		throw InvalidGameboardException("ERROR: Invalid gameboard size specified. Too small.");
		return;
	}

	m_xMax = x_arg;
	m_yMax = y_arg;

	//Create the gameboard tiles
	for(uint i = 0; i < m_xMax; i++)
	{
		vector<Tile*> column;
		m_tiles.push_back(column);

		for(uint j = 0; j < m_yMax; j++)
		{
			m_tiles[i].push_back(new Tile(i,j));
			m_tiles[i][j]->m_ID = (j*m_xMax) + i;
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
	if( (uint)x > m_xMax || (uint)y > m_yMax )
	{
		return NULL;
	}

	return m_tiles[x][y];

}

//Build a directional graph based on this unit's movement ablities on the map
//	Returns the number of edges in the graph
int Gameboard::BuildGraph(Unit *unit, Edge *edge_array, double *weight_array)
{
	uint count = 0;
	vector <Edge> edges;
	vector <double> weights;

	double nextWeight;

	//For every tile...
	for(uint i = 0; i < m_xMax; i++)
	{
		for(uint j = 0; j < m_yMax; j++)
		{
			//For each direction
			for( int k = 0; k < 6; k++ )
			{
				nextWeight = GetMovementCost(m_tiles[i][j], static_cast<Direction>(k), unit);
				//Accumulate how many outgoing edges there are
				if( nextWeight != -1)
				{
					edges.push_back(Edge(i,j));
					weights.push_back(nextWeight);
					count++;
				}
			}
		}
	}

	//Declare space and copy the edge array over
	int edge_size = count * sizeof(Edge);
	edge_array = (Edge*)malloc(edge_size);
	memcpy(edge_array, edges.data(), edge_size);

	//Delcate space and copy the weight array over
	int weight_size = count * sizeof(double);
	weight_array = (double*)malloc(weight_size);
	memcpy(weight_array, weights.data(), weight_size);

	return count;
}

//Returns the weight (IE: Movement cost) of moving in the given direction
//	Returns -1 when impassable.
double Gameboard::GetMovementCost(Tile *fromTile, Direction dir, Unit *unit)
{
	//If this tile is impassable, then don't bother even looking
	if( !fromTile->m_isPassable )
	{
		return -1;
	}

	//Set offsets for moving North and South
	//(Movement north and south gets weird in a hex map)

	//0 when even, 1 when odd
	uint yValEvenOdd = fromTile->m_y % 2;
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
		Tile *toTile = GetTile(fromTile->m_x+eastXOffset, fromTile->m_y+1);
		if( toTile == NULL)
		{
			return -1;
		}
		else if( toTile->m_isPassable == false )
		{
			return -1;
		}
		else if( unit->m_verticalMovement <
				ElevationDiff( fromTile->m_elevation, toTile->m_elevation) )
		{
			return -1;
		}
		else
		{
			return toTile->m_movementCost;
		}

	}
	if( dir == NORTHWEST )
	{
		Tile *toTile = GetTile(fromTile->m_x+westXOffset, fromTile->m_y+1);
		if( toTile == NULL)
		{
			return -1;
		}
		else if( toTile->m_isPassable == false )
		{
			return -1;
		}
		else if( unit->m_verticalMovement <
				ElevationDiff( fromTile->m_elevation, toTile->m_elevation) )
		{
			return -1;
		}
		else
		{
			return toTile->m_movementCost;
		}

	}
	if( dir == WEST )
	{
		Tile *toTile = GetTile( (int)(fromTile->m_x)-1, fromTile->m_y);
		if( toTile == NULL)
		{
			return -1;
		}
		else if( toTile->m_isPassable == false )
		{
			return -1;
		}
		else if( unit->m_verticalMovement <
				ElevationDiff( fromTile->m_elevation, toTile->m_elevation) )
		{
			return -1;
		}
		else
		{
			return toTile->m_movementCost;
		}

	}
	if( dir == SOUTHWEST )
	{
		Tile *toTile = GetTile(fromTile->m_x+westXOffset, fromTile->m_y-1);
		if( toTile == NULL)
		{
			return -1;
		}
		else if( toTile->m_isPassable == false )
		{
			return -1;
		}
		else if( unit->m_verticalMovement <
				ElevationDiff( fromTile->m_elevation, toTile->m_elevation) )
		{
			return -1;
		}
		else
		{
			return toTile->m_movementCost;
		}
	}
	if( dir == SOUTHEAST )
	{
		Tile *toTile = GetTile(fromTile->m_x+eastXOffset, fromTile->m_y-1);
		if( toTile == NULL)
		{
			return -1;
		}
		else if( toTile->m_isPassable == false )
		{
			return -1;
		}
		else if( unit->m_verticalMovement <
				ElevationDiff( fromTile->m_elevation, toTile->m_elevation) )
		{
			return -1;
		}
		else
		{
			return toTile->m_movementCost;
		}
	}
	if( dir == EAST )
	{
		Tile *toTile = GetTile(fromTile->m_x+1, fromTile->m_y);
		if( toTile == NULL)
		{
			return -1;
		}
		else if( toTile->m_isPassable == false )
		{
			return -1;
		}
		else if( unit->m_verticalMovement <
				ElevationDiff( fromTile->m_elevation, toTile->m_elevation) )
		{
			return -1;
		}
		else
		{
			return toTile->m_movementCost;
		}
	}

	throw IllegalMoveException("Tried to move in a direction not one of the valid 6.");
	return -1;

}

uint Gameboard::ElevationDiff(uint elevation1, uint elevation2)
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

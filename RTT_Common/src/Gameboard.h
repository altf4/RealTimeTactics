//============================================================================
// Name        : RTT_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Representation of the hex gameboard
//============================================================================

#ifndef GAMEBOARD_H_
#define GAMEBOARD_H_

#include <stdlib.h>
#include <vector>
#include "Tile.h"
#include "Unit.h"
#include <string>
#include <string.h>

using namespace std;
typedef pair<int, int> Edge;
namespace RTT
{

/*
 * Note that a hex gameboard can be viewed as tiled squares laid out like so:
 *
 * -------------------------
 * |   |   |   |   |   |   |
 * -------------------------
 *   |   |   |   |   |   |
 * -------------------------
 * |   |   |   |   |   |   |
 * -------------------------
 *
 * This is equivalent to a hex board, and may be easier to visualize.
 *
 */

class Gameboard
{

public:
	//The size of the gameboard in both x and y directions
	uint m_xMax, m_yMax;

	//The 2D vector of tiles (vector of vectors)
	vector<vector<Tile*> > m_tiles;

	//Arguments:
	//	x - Set the maximum longitude of gameboard
	//	y - Set the maximum latitude of gameboard
	//Throws InvalidGameboardException on error
	Gameboard(uint x, uint y);

	//Constructor to set Gameboard from file
	//TODO: Define this file structure
	Gameboard(string filePath);

	//Return the tile specified by the given coordinates
	//Return NULL if there is no such tile
	Tile *GetTile(int x, int y);

	//Build a directional graph based on this unit's movement ablities on the map
	//	Returns the number of edges in the graph
	int BuildGraph(Unit *unit, Edge *edge_array, double *weight_array);

	//Returns the weight (IE: Movement cost) of moving in the given direction
	//	Returns -1 when impassible.
	double GetMovementCost(Tile *tile, Direction dir, Unit *unit);


private:

	//Find the difference between two elevations
	//IE: The distance. Always a positive integer.
	uint ElevationDiff(uint elevation1, uint elevation2);
};

}

#endif /* GAMEBOARD_H_ */

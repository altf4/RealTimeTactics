//============================================================================
// Name        : LoF_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Representation of the hex gameboard
//============================================================================

#ifndef GAMEBOARD_H_
#define GAMEBOARD_H_

#include <stdlib.h>
#include <vector>
#include "Tile.h"

using namespace std;

namespace LoF
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
	uint xMax, yMax;

	//The 2D vector of tiles (vector of vectors)
	vector<vector<Tile*> > tiles;

	//Arguments:
	//	x - Set the maximum longitude of gameboard
	//	y - Set the maximum latitude of gameboard
	//Throws InvalidGameboardException on error
	Gameboard(uint x, uint y);

};

}

#endif /* GAMEBOARD_H_ */

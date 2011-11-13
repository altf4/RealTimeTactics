//============================================================================
// Name        : LoF_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : One Hex on the gameboard
//============================================================================

#ifndef TILE_H_
#define TILE_H_

#include <stdlib.h>
#include <vector>
#include "Unit.h"

using namespace std;

namespace LoF
{

class Tile
{
public:
	//Coordinates of this tile
	//Note that for any tile (x,y), there are 6 bordering tiles:
	//	(x-1,y),(x-1,y-1),(x,y-1),(x+1,y),(x+1,y+1),(x,y+1)
	//	Unless, of course, this tile is on the edge of the gameboard
	uint x, y;
	uint elevation; //aka: z coordinate


	//Properties of this tile:

	//How many movement points it takes to move into this tile
	//	Normal = 1
	//	Easy terrain (road, etc..) = .5
	//	Rough terrain (swamp, etc..) = 2
	double movementCost;

	//List of units currently standing on this tile
	vector <Unit*> unitsPresent;

	//Whether or not a unit can stand on this tile
	bool isPassable;


	//Basic constructor
	Tile(uint x, uint y);

	//Moves the Unit (who is currently on this tile) to the specified tile
	void MoveUnitTo(Unit *unit, Tile *tile);

	//Checks if the given unit is standing on this tile
	//Returns the index of the Unit in the Tile's list
	//Returns -1 if it's not there
	int IndexOfUnit(Unit *unit);


};

}


#endif /* TILE_H_ */

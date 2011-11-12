//============================================================================
// Name        : LoF_Server.cpp
// Author      : AltF4
// Copyright   : GNU GPLv3
// Description : One Hex on the gameboard
//============================================================================

#ifndef TILE_H_
#define TILE_H_

#include <stdlib.h>

using namespace std;

namespace LoF
{

class Tile
{
	Tile();

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




};

}


#endif /* TILE_H_ */

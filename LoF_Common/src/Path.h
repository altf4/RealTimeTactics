//============================================================================
// Name        : Unit.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : A movement path along the hex map coordinate system
//============================================================================

#ifndef PATH_H_
#define PATH_H_

#include <vector>
#include "Tile.h"

using namespace std;

namespace LoF
{

class Path
{
	vector <Tile*> tilesMovedThrough;

	//The total number of tiles moved through
	uint tileDistance;

	//Total movement cost of the path
	double movementCost;

	Path(Tile *fromTile, Tile *toTile, uint jump);

};


}


#endif /* PATH_H_ */

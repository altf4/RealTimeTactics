//============================================================================
// Name        : RTT_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Representation and helper functions for game maps
//============================================================================

#ifndef MAP_H_
#define MAP_H_

#include "stdlib.h"

using namespace std;

namespace RTT
{

struct MapDescription
{
	char name[20];
	//X direction
	uint width;
	//Y direction
	uint length;
};

class Map
{
public:

	Map();
};


}


#endif /* MAP_H_ */

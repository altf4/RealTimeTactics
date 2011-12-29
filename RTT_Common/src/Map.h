//============================================================================
// Name        : RTT_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Representation and helper functions for game maps
//============================================================================

#ifndef MAP_H_
#define MAP_H_

#define MAP_NAME_LEN 20
#define MAP_DESCR_SIZE MAP_NAME_LEN + sizeof(uint) + sizeof(uint)

#include "stdlib.h"

using namespace std;

namespace RTT
{

struct MapDescription
{
	char name[MAP_NAME_LEN];
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

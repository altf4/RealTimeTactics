//============================================================================
// Name        : RTT_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Representation and helper functions for game maps
//============================================================================

#ifndef MAP_H_
#define MAP_H_

#define MAP_NAME_LEN 20
#define MAP_DESCR_SIZE MAP_NAME_LEN + sizeof(uint32_t) + sizeof(uint32_t)

#include "stdlib.h"
#include <stdint.h>

using namespace std;

namespace RTT
{

struct MapDescription
{
	char name[MAP_NAME_LEN];
	//X direction
	uint32_t width;
	//Y direction
	uint32_t length;
};

class Map
{
public:

	Map();
};


}


#endif /* MAP_H_ */

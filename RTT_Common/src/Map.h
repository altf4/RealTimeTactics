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

namespace RTT
{

struct MapDescription
{
	char m_name[MAP_NAME_LEN];
	//X direction
	uint32_t m_width;
	//Y direction
	uint32_t m_length;
};

class Map
{
public:

	Map();
};


}


#endif /* MAP_H_ */

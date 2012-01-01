//============================================================================
// Name        : Unit.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Global enum types used to evade circular dependency
//============================================================================

#ifndef ENUMS_H_
#define ENUMS_H_

#include <stdint.h>

namespace RTT
{

enum TeamNumber: uint32_t
{
	SPECTATOR = 0,
	TEAM_1 = 1,
	TEAM_2 = 2,
	TEAM_3 = 3,
	TEAM_4 = 4,
	TEAM_5 = 5,
	TEAM_6 = 6,
	TEAM_7 = 7,
	TEAM_8 = 8,
	REFEREE = 9,
};

enum TeamColor: char
{
	COLOR_BLUE = 0,
	COLOR_RED,
	COLOR_GREEN,
	COLOR_PURPLE,
	COLOR_BLACK,
	COLOR_WHITE,
	COLOR_YELLOW,
	COLOR_ORANGE,
};

enum VictoryCondition: char
{
	DEATHMATCH,
};

enum GameSpeed: char
{
	SPEED_SLOW,
	SPEED_NORMAL,
	SPEED_FAST,
	SPEED_FASTEST,
};

}


#endif /* ENUMS_H_ */

 //============================================================================
// Name        : MatchTypes.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Represents some types that are used by Matches and other classes
//		that use Matches
//============================================================================

#ifndef MATCHTYPES_H_
#define MATCHTYPES_H_

#include "stdint.h"

#define MAX_MATCHNAME_LEN 20
#define MATCH_DESCR_SIZE sizeof(enum MatchStatus) + (sizeof(uint32_t)*4) + \
		MAX_MATCHNAME_LEN + sizeof(int64_t)
#define MATCH_OPTIONS_SIZE sizeof(uint32_t) + MAX_MATCHNAME_LEN

namespace RTT
{
//Forward declaration to avoid self-reference
class Team;
class Player;

enum MatchStatus: uint32_t
{
	MATCH_WAITING_TO_START,
	MATCH_IN_PROGRESS,
	MATCH_FINISHED
};

//A fixed size description of the match
//	Suitable for sending to others to give match info
struct MatchDescription
{
	enum MatchStatus m_status;
	uint32_t m_ID;
	uint32_t m_maxPlayers;
	uint32_t m_currentPlayerCount;
	char m_name[MAX_MATCHNAME_LEN];
	int64_t m_timeCreated;
	uint32_t m_leaderID;
};

//A fixed size collection of options for match creation
//	List of options necessary upon creation of the Match
struct MatchOptions
{
	uint32_t m_maxPlayers;
	char m_name[MAX_MATCHNAME_LEN];
};

}

#endif /* MATCHTYPES_H_ */

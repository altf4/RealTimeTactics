//============================================================================
// Name        : Unit.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Message class sent while player is in the server lobby
//============================================================================

#define SERVER_STATS_SIZE sizeof(uint32_t) + sizeof(uint32_t)

#include "Message.h"
#include "../Match.h"

namespace RTT
{

enum LobbyResult: char
{
	//Success
	LOBBY_SUCCESS = 0,

	//Failure joining
	LOBBY_MATCH_IS_FULL,
	LOBBY_MATCH_DOESNT_EXIST,
	LOBBY_NOT_ALLOWED_IN,
	LOBBY_ALREADY_IN_MATCH,
	LOBBY_PLAYER_NULL,
};

struct ServerStats
{
	//Total number of matches on server right now
	uint32_t m_numMatches;
	//Total number of players on server right now
	uint32_t m_numPlayers;
};

class LobbyMessage: public Message
{
public:
	uint32_t m_requestedPage;
	uint32_t m_returnedMatchesCount;
	uint32_t m_returnedPlayersCount;
	//A list of queried matches
	struct MatchDescription *m_matchDescriptions;
	//Newly created match
	struct MatchDescription m_matchDescription;
	struct PlayerDescription *m_playerDescriptions;
	struct MatchOptions m_options;
	//ID of a match to join
	uint32_t m_ID;
	struct ServerStats m_serverStats;


	LobbyMessage();
	~LobbyMessage();
	LobbyMessage(char *buffer, uint length);
	char *Serialize(uint *length);
};

}

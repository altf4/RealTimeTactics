//============================================================================
// Name        : Unit.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Message class pertaining to the Match Lobby (not main lobby)
//============================================================================

#ifndef MATCHLOBBYMESSAGE_H_
#define MATCHLOBBYMESSAGE_H_

#include "Message.h"
#include "../Map.h"
#include "../Player.h"
#include "../Enums.h"

namespace RTT
{

class MatchLobbyMessage: public Message
{
public:

	enum TeamNumber m_newTeam;
	enum TeamColor m_newColor;
	enum VictoryCondition m_newVictCond;
	enum GameSpeed m_newSpeed;
	bool m_changeAccepted;
	struct MapDescription m_mapDescription;
	uint32_t m_playerID;
	struct PlayerDescription m_playerDescription;
	uint32_t m_portNum;
	uint32_t m_newLeaderID;

	MatchLobbyMessage();
	~MatchLobbyMessage();
	MatchLobbyMessage(char *buffer, uint length);
	char *Serialize(uint *length);

};

}

#endif /* MATCHLOBBYMESSAGE_H_ */

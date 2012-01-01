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

using namespace std;

namespace RTT
{

class MatchLobbyMessage: public Message
{
public:

	enum TeamNumber newTeam;
	enum TeamColor newColor;
	enum VictoryCondition newVictCond;
	enum GameSpeed newSpeed;
	bool changeAccepted;
	struct MapDescription mapDescription;
	uint32_t playerID;
	struct PlayerDescription playerDescription;
	uint32_t portNum;

	MatchLobbyMessage();
	~MatchLobbyMessage();
	MatchLobbyMessage(char *buffer, uint length);
	char *Serialize(uint *length);

};

}

#endif /* MATCHLOBBYMESSAGE_H_ */

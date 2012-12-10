//============================================================================
// Name        : OgreMatchLobbyEvents.h
// Author      : AltF4
// Copyright   : 2012, GNU GPLv3
// Description : Match Lobby events which have happened on the server, and the
//					client is being notified about.
//============================================================================

#ifndef OGREMATCHLOBBYEVENTS_H_
#define OGREMATCHLOBBYEVENTS_H_

#include "GameEvents.h"
#include "Enums.h"
#include "Player.h"
#include "Map.h"
#include "MatchLobbyEvents.h"

namespace RTT
{

class OgreMatchLobbyEvents : public MatchLobbyEvents
{

public:

	OgreMatchLobbyEvents(){};

	void UI_TeamChangedSignal(uint32_t playerID, enum TeamNumber newTeam);

	void UI_KickFromMatchSignal();

	void UI_PlayerLeftSignal(uint32_t playerID, uint32_t leaderID);

	void UI_PlayerJoinedSignal(struct PlayerDescription player);

	void UI_ColorChangedSignal(uint32_t playerID, enum TeamColor color);

	void UI_MapChangedSignal(struct MapDescription mapDesc);

	void UI_GamespeedChangedSignal(enum GameSpeed speed);

	void UI_VictoryCondChangedSignal(enum VictoryCondition victory);

	void UI_ChangeLeaderSignal(uint32_t playerID);

	void UI_MatchStartedSignal();

};

}

#endif /* OGREMATCHLOBBYEVENTS_H_ */

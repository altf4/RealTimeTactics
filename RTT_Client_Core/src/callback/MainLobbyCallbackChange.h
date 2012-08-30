//============================================================================
// Name        : MainLobbyCallbackChange.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Parent class for CallbackChanges
//
//============================================================================

#ifndef MAINLOBBYCALLBACKCHANGE_H_
#define MAINLOBBYCALLBACKCHANGE_H_

#include "CallbackChange.h"
#include "Player.h"

namespace RTT
{

enum MainLobbyCallbackType
{
	TEAM_CHANGE,
	COLOR_CHANGE,
	MAP_CHANGE,
	SPEED_CHANGE,
	VICTORY_CHANGE,
	PLAYER_LEFT,
	KICKED,
	PLAYER_JOINED,
	LEADER_CHANGE,
	MATCH_STARTED,
};

class MainLobbyCallbackChange : public CallbackChange
{

public:

	MainLobbyCallbackChange(enum MainLobbyCallbackType type) :CallbackChange(CHANGE_MAIN_LOBBY) {m_mainLobbyType = type;};

	//Type, which one of the following is used:
	enum MainLobbyCallbackType m_mainLobbyType;

	//Which of the following is used?

	uint m_playerID; //several

	//TEAM_CHANGE
	enum TeamNumber m_team;

	//COLOR_CHANGE
	enum TeamColor m_color;

	//MAP_CHANGE
	struct MapDescription m_mapDescription;

	//SPEED_CHANGE
	enum GameSpeed m_speed;

	//VICTORY_CHANGE
	enum VictoryCondition m_victory;

	//PLAYER_LEFT
	uint m_newLeaderID;

	//KICKED (nothing)

	//PLAYER_JOINED
	struct PlayerDescription m_playerDescription;
};

}

#endif /* MAINLOBBYCALLBACKCHANGE_H_ */

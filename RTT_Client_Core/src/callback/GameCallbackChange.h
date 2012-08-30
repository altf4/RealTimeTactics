//============================================================================
// Name        : MainLobbyCallbackChange.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Parent class for CallbackChanges
//
//============================================================================

#ifndef GAMECALLBACKCHANGE_H_
#define GAMECALLBACKCHANGE_H_

#include "CallbackChange.h"

namespace RTT
{

enum GameCallbackType
{
	LOL_WUT,
};

class GameCallbackChange : public CallbackChange
{

public:

	GameCallbackChange(enum MainLobbyCallbackType type) {m_gameType = type;};

	enum MainLobbyCallbackType m_gameType;
};

}

#endif /* GAMECALLBACKCHANGE_H_ */

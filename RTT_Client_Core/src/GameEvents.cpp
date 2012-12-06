//============================================================================
// Name        : GameEvents.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Game events which have happened on the server, and the
//					client is being notified about.
//============================================================================

#include "GameEvents.h"
#include "messaging/MessageManager.h"
#include "messaging/messages/Message.h"
#include "messaging/messages/GameMessage.h"
#include "ClientGameState.h"

#include <iostream>

using namespace RTT;

extern int socketFD;

GameEvents::~GameEvents()
{

}


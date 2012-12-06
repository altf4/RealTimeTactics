//============================================================================
// Name        : CallbackHandler.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Handles incoming callback messages from game server
//============================================================================

#ifndef CALLBACKHANDLER_H_
#define CALLBACKHANDLER_H_

#include <queue>
#include "pthread.h"

#include "Enums.h"
#include "ClientProtocolHandler.h"
#include "GameEvents.h"
#include "MatchLobbyEvents.h"

namespace RTT
{

class CallbackHandler
{
public:

	CallbackHandler(int socketFD, GameEvents *gameContext, MatchLobbyEvents *matchLobbyContext);
	~CallbackHandler();

	void Stop();

private:

	void *CallbackThread();

	static void *StartThreadHelper(void *ptr);

	pthread_t m_thread;
	int m_socketFD;

	GameEvents *m_gameContext;
	MatchLobbyEvents *m_matchLobbyContext;
};

}
#endif /* CALLBACKHANDLER_H_ */

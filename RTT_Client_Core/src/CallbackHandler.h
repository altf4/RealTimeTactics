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
#include "MainLobbyEvents.h"

namespace RTT
{

class CallbackHandler
{
public:

	CallbackHandler(GameEvents *gameContext, MatchLobbyEvents *matchLobbyContext, MainLobbyEvents *mainLobbycontext);
	~CallbackHandler();

	void Start(int socketFD);
	void Stop();

private:

	void *CallbackThread();

	static void *StartThreadHelper(void *ptr);

	pthread_t m_thread;
	int m_socketFD;
	bool m_isRunning;

	GameEvents *m_gameContext;
	MatchLobbyEvents *m_matchLobbyContext;
	MainLobbyEvents *m_mainLobbyEvents;
};

}
#endif /* CALLBACKHANDLER_H_ */

//============================================================================
// Name        : CallbackHandler.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Handles incoming callback messages from game server
//============================================================================

#include "CallbackHandler.h"
#include "messaging/MessageManager.h"

#include <iostream>

using namespace std;
using namespace RTT;

CallbackHandler::CallbackHandler(int socketFD, GameEvents *gameContext, MatchLobbyEvents *matchLobbyContext)
{
	m_socketFD = socketFD;
	m_gameContext = gameContext;
	m_matchLobbyContext = matchLobbyContext;
	pthread_create(&m_thread, NULL, &CallbackHandler::StartThreadHelper, this);
}

CallbackHandler::~CallbackHandler()
{
	pthread_join(m_thread, NULL);
}

void *CallbackHandler::StartThreadHelper(void *ptr)
{
	return reinterpret_cast<CallbackHandler*>(ptr)->CallbackThread();
}

void CallbackHandler::Stop()
{
	pthread_join(m_thread, NULL);
}

void *CallbackHandler::CallbackThread()
{
	enum LobbyReturn lobbyReturn = EXITING_SERVER;

	//TODO: Process the first callback message. Which is currently assumed to be a MatchLobby message. This is probably bad
	{
		Ticket ticket;
		if(!MessageManager::Instance().RegisterCallback(m_socketFD, ticket))
		{
			return NULL;
		}

		lobbyReturn = ProcessMatchLobbyEvent(ticket, m_matchLobbyContext);
	}

	//Not an infinite loop. Function returns internally under certain conditions
	while(true)
	{
		Ticket ticket;
		if(!MessageManager::Instance().RegisterCallback(m_socketFD, ticket))
		{
			return NULL;
		}

		switch(lobbyReturn)
		{
			case IN_MATCH_LOBBY:
			{
				cout << "xxxDEBUGxxx processing match lobby command " << endl;
				lobbyReturn = ProcessMatchLobbyEvent(ticket, m_matchLobbyContext);
				break;
			}
			case IN_GAME:
			{
				cout << "xxxDEBUGxxx processing game command " << endl;
				lobbyReturn = ProcessGameEvent(ticket, m_gameContext);
				break;
			}
			case EXITING_SERVER:
			{
				return NULL;
			}
			case IN_MAIN_LOBBY:
			{
				cout << "xxxDEBUGxxx processing main lobby command " << endl;
				lobbyReturn = ProcessMainLobbyEvent(ticket);
				break;
			}
		}
	}
	return NULL;
}

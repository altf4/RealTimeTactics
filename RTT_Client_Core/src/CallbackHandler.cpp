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

CallbackHandler::CallbackHandler(GameEvents *gameContext, MatchLobbyEvents *matchLobbyContext, MainLobbyEvents *mainLobbyContext)
{
	m_socketFD = -1;
	m_isRunning = false;
	m_gameContext = gameContext;
	m_matchLobbyContext = matchLobbyContext;
	m_mainLobbyEvents = mainLobbyContext;
}

CallbackHandler::~CallbackHandler()
{
	pthread_join(m_thread, NULL);
	delete m_gameContext;
	delete m_matchLobbyContext;
	delete m_mainLobbyEvents;
}

void *CallbackHandler::StartThreadHelper(void *ptr)
{
	return reinterpret_cast<CallbackHandler*>(ptr)->CallbackThread();
}

void CallbackHandler::Start(int socketFD)
{
	if(!m_isRunning)
	{
		m_isRunning = true;
		m_socketFD = socketFD;
		pthread_create(&m_thread, NULL, &CallbackHandler::StartThreadHelper, this);
	}
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

		cout << "xxxDEBUGxxx Processing Match Lobby Event..." << endl;
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
				cout << "xxxDEBUGxxx Processing Match Lobby Event..." << endl;
				lobbyReturn = ProcessMatchLobbyEvent(ticket, m_matchLobbyContext);
				break;
			}
			case IN_GAME:
			{
				cout << "xxxDEBUGxxx Processing Game Event..." << endl;
				lobbyReturn = ProcessGameEvent(ticket, m_gameContext);
				break;
			}
			case EXITING_SERVER:
			{
				m_isRunning = false;
				return NULL;
			}
			case IN_MAIN_LOBBY:
			{
				cout << "xxxDEBUGxxx Processing Main Lobby Event..." << endl;
				lobbyReturn = ProcessMainLobbyEvent(ticket, m_mainLobbyEvents);
				break;
			}
		}
	}
	return NULL;
}

//============================================================================
// Name        : RTTServerCallback.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : RTT Game Server
//============================================================================

#include "messaging/MessageManager.h"
#include "messaging/messages/Message.h"
#include "messaging/messages/ErrorMessage.h"
#include "RTTServerCallback.h"
#include "ServerProtocolHandler.h"
#include "RTT_Server.h"
#include "Enums.h"

#include "iostream"

using namespace std;

extern RTT::PlayerList playerList;
extern RTT::MatchList matchList;

extern pthread_rwlock_t matchListLock;
extern pthread_rwlock_t playerListLock;

namespace RTT
{

void RTTServerCallback::CallbackThread(int socketFD)
{
	Player *player;
	{
		Ticket ticket;
		if(!MessageManager::Instance().RegisterCallback(socketFD, ticket))
		{
			return;
		}

		//First, authenticate the client
		player = GetNewClient(ticket);
		if( player == NULL )
		{
			cout << "ERROR: Authentication Failure\n";
			return;
		}

		cout << "Client: " << player->GetName() << " Authenticated!\n";
		player->SetSocket(socketFD);
	}

	enum LobbyReturn lobbyReturn = EXITING_SERVER;
	{
		//We start out in the Main Lobby context. Do one iteration
		Ticket ticket;
		if(!MessageManager::Instance().RegisterCallback(socketFD, ticket))
		{
			cout << "Player: " << player->GetName() << " has left.\n";
			QuitServer(player);
			return;
		}

		lobbyReturn =  ProcessLobbyCommand(ticket, player);
	}

	//*************************************
	// Main Server Loop
	//*************************************
	while(true)
	{
		Ticket ticket;
		if(!MessageManager::Instance().RegisterCallback(socketFD, ticket))
		{
			cout << "Player: " << player->GetName() << " has left.\n";
			QuitServer(player);
			return;
		}

		switch(lobbyReturn)
		{
			case IN_MATCH_LOBBY:
			{
				cout << "xxxDEBUGxxx processing match lobby command " << endl;
				lobbyReturn = ProcessMatchLobbyCommand(ticket, player);
				break;
			}
			case IN_GAME:
			{
				cout << "xxxDEBUGxxx processing game command " << endl;
				lobbyReturn = ProcessGameCommand(ticket, player);
				break;
			}
			case EXITING_SERVER:
			{
				cout << "Player: " << player->GetName() << " has left.\n";
				QuitServer(player);
				return;
			}
			case IN_MAIN_LOBBY:
			{
				cout << "xxxDEBUGxxx processing main lobby command " << endl;
				lobbyReturn = ProcessLobbyCommand(ticket, player);
				break;
			}
		}
	}

}

//Player has quit the server, clean up any references to it
//	Deletes the player object
void RTTServerCallback::QuitServer(Player *player)
{
	if( player == NULL )
	{
		return;
	}
	if( player->GetCurrentMatchID() != 0)
	{
		//Leave any matches currently in
		LeaveMatch(player);
	}

	int ID = player->GetID();
	//Remove from the list of current players
	{
		Lock lock(&playerListLock, WRITE_LOCK);
		playerList[ID] = NULL;
		playerList.erase(ID);
	}
	delete player;
}

}

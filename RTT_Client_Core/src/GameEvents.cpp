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

//Parent function of all Game Events
//	This function listens on the callback socket for Game Events
//	When an event is read, it is processed by calling the sub-functions below
//	returns - true if the game is still running: IE: We should call this function again
//		a return of false indicates that the game is over, and we should enter the MainLobby
bool GameEvents::ProcessGameEvent()
{
	Ticket ticket;
	if(!MessageManager::Instance().RegisterCallback(socketFD, ticket))
	{
		return false;
	}

	Message *event_message = MessageManager::Instance().ReadMessage(ticket);
	if(event_message->m_messageType != MESSAGE_GAME)
	{
		cerr << "ERROR: Message read from server failed. Did it die?\n";
		delete event_message;
		return false;
	}

	GameMessage *game_message = (GameMessage*)event_message;
	switch(game_message->m_gameMessageType)
	{
		case UNIT_MOVED_DIRECTION_NOTICE:
		{
			//TODO: Specify a separate direction to face
			struct Coordinate source = {game_message->m_xOld, game_message->m_yOld};

			ClientGameState::Instance().MoveUnitDirection(game_message->m_unitID,
					source,	game_message->m_unitDirection, game_message->m_unitDirection);

			//Call the UI's movement code (IE: Move the unit on the screen)
			UI_UnitMovedDirectionSignal(game_message->m_unitID,
					source,	game_message->m_unitDirection, game_message->m_unitDirection);

			//Send back an acknowledgment of the move
			GameMessage unit_moved_reply(UNIT_MOVED_DIRECTION_ACK);
			MessageManager::Instance().WriteMessage(ticket, &unit_moved_reply);
			break;
		}
		default:
		{
			break;
		}
	}
	//TODO: Handle the unit moved message!

	delete game_message;
	return true;
}

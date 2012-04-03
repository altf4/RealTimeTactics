//============================================================================
// Name        : GameEvents.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Game events which have happened on the server, and the
//					client is being notified about.
//============================================================================

#include "GameEvents.h"
#include "messages/Message.h"
#include "messages/GameMessage.h"
#include "ClientGameState.h"

#include <iostream>

using namespace RTT;

extern int callbackSocket;

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
	Message *event_message = Message::ReadMessage(callbackSocket);
	if(event_message == NULL)
	{
		cerr << "ERROR: Message read from server failed. Did it die?\n";
		return false;
	}

	GameMessage *game_message = (GameMessage*)event_message;
	switch(game_message->m_type)
	{
		case UNIT_MOVED_DIRECTION_NOTICE:
		{
			//TODO: Specify a separate direction to face
			struct Coordinate source = {game_message->m_xOld, game_message->m_yOld};

			ClientGameState::Instance().MoveUnitDirection(game_message->m_unitID,
					source,	game_message->m_direction, game_message->m_direction);

			//Call the UI's movement code (IE: Move the unit on the screen)
			UI_UnitMovedDirectionSignal(game_message->m_unitID,
					source,	game_message->m_direction, game_message->m_direction);

			//Send back an acknowledgment of the move
			GameMessage unit_moved_reply = GameMessage();
			unit_moved_reply.m_type = UNIT_MOVED_DIRECTION_ACK;
			Message::WriteMessage(&unit_moved_reply, callbackSocket);
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

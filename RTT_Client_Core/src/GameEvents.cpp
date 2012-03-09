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

#include <iostream>

using namespace RTT;

extern int callbackSocket;

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

	//TODO: Handle the unit moved message!

	return true;
}

//============================================================================
// Name        : MainLobbyEvents.h
// Author      : AltF4
// Copyright   : 2012, GNU GPLv3
// Description : Main Lobby events which have happened on the server, and the
//					client is being notified about.
//============================================================================

#ifndef MAINLOBBYEVENTS_H_
#define MAINLOBBYEVENTS_H_

namespace RTT
{

//********************************************
//				How to Use This
//********************************************
//	This class defines a set of functions that represent events that have
//		happened on the server, and you (the client) are being notified of.
//		The non-virtual functions in this class are called and executed automatically
//		and take care of all the heavy lifting in terms of game-state and server
//		communication. The virtual functions are called at the end of their
//		respectively paired non-virtual function.
//
//	What you need to do is create override implementations of the virtual functions
//		That is where the UI code goes to "hook into" these events. So, for example,
//		you need to provide an implementation for the function UI_UnitMovedSignal.
//		That is an event telling you that an enemy Unit has moved, and so you need to
//		visually represent for the player

//		Each of the virtual functions begin with "UI_" to help you find them

class MainLobbyEvents
{

public:

	virtual ~MainLobbyEvents();

};

}

#endif /* MAINLOBBYEVENTS_H_ */

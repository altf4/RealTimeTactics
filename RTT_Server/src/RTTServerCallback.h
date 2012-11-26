//============================================================================
// Name        : RTTServerCallback.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : RTT Game Server
//============================================================================

#ifndef RTTSERVERCALLBACK_H_
#define RTTSERVERCALLBACK_H_

#include "messaging/ServerCallback.h"
#include "Player.h"

namespace RTT
{

class RTTServerCallback : public ServerCallback
{

protected:

	void CallbackThread(int socketFD);

	//Player has quit the server, clean up any references to it
	//	Deletes the player object
	void QuitServer(Player *player);
};

}

#endif /* RTTSERVERCALLBACK_H_ */

//============================================================================
// Name        : GameMessage.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Message class for in-game actions
//============================================================================

#ifndef GAMEMESSAGE_H_
#define GAMEMESSAGE_H_

#include "Message.h"

using namespace std;

namespace RTT
{

class GameMessage: public Message
{
public:

	GameMessage();
	GameMessage(char *buffer, uint length);
	char *Serialize(uint *length);

};

}

#endif /* GAMEMESSAGE_H_ */

//============================================================================
// Name        : GameMessage.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Message class for in-game actions
//============================================================================

#ifndef GAMEMESSAGE_H_
#define GAMEMESSAGE_H_

#include "Message.h"
#include "../Unit.h"

using namespace std;

enum MoveResult: char
{
	//Success!
	MOVE_SUCCESS = 0,

	//Failures...
	MOVE_NO_SUCH_UNIT,
	MOVE_NOT_YOUR_TURN,
	MOVE_IMPASSIBLE_DEST,
	MOVE_TOO_FAR,
	MOVE_WRONG_SOURCE,
	MOVE_UNIT_CANNOT_MOVE,
	MOVE_MESSAGE_SEND_ERROR,
};

namespace RTT
{

class GameMessage: public Message
{
public:

	GameMessage();
	GameMessage(char *buffer, uint length);
	char *Serialize(uint *length);


	uint32_t unitID;
	uint32_t xOld, yOld;
	enum Direction direction;

	enum MoveResult moveResult;
};

}

#endif /* GAMEMESSAGE_H_ */

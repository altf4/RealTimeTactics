//============================================================================
// Name        : GameMessage.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Message class for in-game actions
//============================================================================

#ifndef GAMEMESSAGE_H_
#define GAMEMESSAGE_H_

#include "Message.h"
#include "../../Unit.h"

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

enum GameMessageType: char
{
	MOVE_UNIT_DIRECTION_REQUEST = 0,
	MOVE_UNIT_DIRECTION_REPLY,
	MOVE_UNIT_DISTANT_REQUEST,
	MOVE_UNIT_DISTANT_REPLY,

	UNIT_MOVED_DIRECTION_NOTICE,
	UNIT_MOVED_DIRECTION_ACK,
	UNIT_MOVED_DISTANT_NOTICE,
	UNIT_MOVED_DISTANT_ACK,

	SURRENDER_NOTICE,
	SURRENDER_ACK,
};

namespace RTT
{

class GameMessage: public Message
{
public:

	GameMessage(enum GameMessageType type);
	GameMessage(char *buffer, uint length);
	char *Serialize(uint *length);

	enum GameMessageType m_gameMessageType;

	uint32_t m_unitID;
	uint32_t m_xOld, m_yOld, m_xNew, m_yNew;
	enum Direction m_unitDirection;

	enum MoveResult m_moveResult;
};

}

#endif /* GAMEMESSAGE_H_ */

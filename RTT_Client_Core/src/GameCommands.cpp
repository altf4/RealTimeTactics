//============================================================================
// Name        : GameCommands.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Commands sent to the game server by client
//============================================================================

#include "GameCommands.h"
#include "Gameboard.h"
#include "ClientProtocolHandler.h"
#include "messaging/MessageManager.h"

using namespace std;
using namespace RTT;
using google::dense_hash_map;
using tr1::hash;

//The current gameboard being used this match
Gameboard *gameboard;

UnitList units;

extern int socketFD;

//********************************************
//				Movement Commands
//********************************************

//Move a Unit to an adjacent tile
//	unitID - The ID of the unit to move.
//	direction - The direction to move the unit
//		NOTE: The unit will also by default made to face the direction moved
//	returns - A MovementResult struct describing the success or error of the move
struct MovementResult RTT::MoveUnit(uint32_t unitID, uint32_t xOld, uint32_t yOld, enum Direction direction)
{
	Ticket ticket = MessageManager::Instance().StartConversation(socketFD);

	struct MovementResult result;

	//First, do a quick legality check. So we don't bother the server if we know
	//	that this is invalid.
	if( units.count(unitID) == 0)
	{
		result.m_result = MOVE_NO_SUCH_UNIT;
		return result;
	}

	GameMessage moveRequest(MOVE_UNIT_DIRECTION_REQUEST);
	moveRequest.m_unitID = unitID;
	moveRequest.m_xOld = xOld;
	moveRequest.m_yOld = xOld;
	moveRequest.m_unitDirection = direction;
	if(!MessageManager::Instance().WriteMessage(ticket, &moveRequest))
	{
		result.m_result = MOVE_MESSAGE_SEND_ERROR;
		return result;
	}

	Message *reply = MessageManager::Instance().ReadMessage(ticket);
	if( reply->m_messageType != MESSAGE_GAME)
	{
		SendError(ticket, PROTOCOL_ERROR);
		result.m_result = MOVE_MESSAGE_SEND_ERROR;
		delete reply;
		return result;
	}
	GameMessage *moveReply = (GameMessage*)reply;
	if( moveReply->m_gameMessageType != MOVE_UNIT_DIRECTION_REPLY)
	{
		SendError(ticket, PROTOCOL_ERROR);
		result.m_result = MOVE_MESSAGE_SEND_ERROR;
		delete moveReply;
		return result;
	}
	result.m_result = moveReply->m_moveResult;
	result.m_originalX = moveReply->m_xOld;
	result.m_originalY = moveReply->m_yOld;

	delete moveReply;
	return result;
}

//Move a Unit to a distant tile
//	unitID - The ID of the unit to move.
//		NOTE: The unit will also by default made to face the direction of the last hop
//	returns - A MovementResult struct describing the success or error of the move
struct MovementResult RTT::MoveUnit(uint32_t unitID, uint32_t xOld, uint32_t yOld, uint32_t xNew, uint32_t yNew)
{
	struct MovementResult result;
	result.m_result = MOVE_NO_SUCH_UNIT;

	Ticket ticket = MessageManager::Instance().StartConversation(socketFD);

	GameMessage move_request(MOVE_UNIT_DISTANT_REQUEST);
	move_request.m_unitID = unitID;
	move_request.m_xNew = xNew;
	move_request.m_yNew = yNew;
	move_request.m_xOld = xOld;
	move_request.m_yOld = yOld;

	MessageManager::Instance().WriteMessage(ticket, &move_request);

	Message *move_reply = MessageManager::Instance().ReadMessage(ticket);
	if(move_reply->m_messageType != MESSAGE_GAME)
	{
		return result;
	}
	GameMessage *gameMessage = (GameMessage*)move_reply;
	if(gameMessage->m_gameMessageType != MOVE_UNIT_DISTANT_REPLY)
	{
		return result;
	}
	if((gameMessage->m_yNew == yNew) && (gameMessage->m_xNew == xNew))
	{
		result.m_result = gameMessage->m_moveResult;

	}
	else
	{
		//TODO: return where the unit SHOULD be
	}

	return result;
}

//Make a unit change the direction it is facing
//	unitID - The ID of the unit to move
//	direction - The new direction to move the unit to face
//	returns - simple boolean result of whether the move succeeded
bool RTT::ChangeUnitFacing(uint32_t unitID, enum Direction direction)
{
	Ticket ticket = MessageManager::Instance().StartConversation(socketFD);

	return false;
}

bool RTT::SurrenderMatch()
{
	Ticket ticket = MessageManager::Instance().StartConversation(socketFD);

	GameMessage surrender_notice(SURRENDER_NOTICE);
	if(!MessageManager::Instance().WriteMessage(ticket, &surrender_notice))
	{
		return false;
	}

	Message *message = MessageManager::Instance().ReadMessage(ticket);
	if(message->m_messageType != MESSAGE_GAME)
	{
		return false;
	}
	GameMessage *ack = (GameMessage*)message;
	if(ack->m_gameMessageType == SURRENDER_ACK)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//============================================================================
// Name        : EventQueue.h
// Author      : AltF4
// Copyright   : 2012, GNU GPLv3
// Description : A queue that the Ogre frame loop pulls from to process events
//	that the RTT server has sent us. Events are pushed to this queue in the
//	UI_*Signal functions of events classes
//============================================================================

#ifndef EVENTQUEUE_H_
#define EVENTQUEUE_H_

#include "queue"

#include "Player.h"
#include "Map.h"
#include "Enums.h"
#include "Tile.h"

namespace RTT
{

enum ServerEventType : char
{
	EVENT_NO_EVENT = 0,

	//MainLobby events

	//MatchLobby events
	EVENT_TEAM_CHANGED,
	EVENT_TEAM_COLOR_CHANGED,
	EVENT_LEADER_CHANGED,
	EVENT_MAP_CHANGED,
	EVENT_GAMESPEED_CHANGED,
	EVENT_VICTORY_CONDITION_CHANGED,
	EVENT_PLAYER_LEFT,
	EVENT_KICKED,
	EVENT_PLAYER_JOINED,
	EVENT_MATCH_STARTED,

	//Game Events
	EVENT_UNIT_MOVED_DIRECTION,
	EVENT_UNIT_MOVED_DISTANT,
	EVENT_UNIT_CHANGED_FACING,
	EVENT_PLAYER_SURRENDERED
};

struct ServerEvent
{
	//What state the message pertains to
	//	The client can throw the event out if it's not in their state
	enum LobbyReturn m_appState;

	enum ServerEventType m_type;


	uint32_t m_playerID, m_leaderID;
	uint32_t m_Oldx, mOldy, m_Newx, m_Newy;
	uint32_t m_unitID;
	struct Coordinate m_source, m_dest;
	enum TeamNumber m_newTeam;
	enum TeamColor m_newColor;
	enum GameSpeed m_newGamespeed;
	enum VictoryCondition m_victCondition;
	struct PlayerDescription m_newPlayer;
	struct MapDescription m_newMap;

};

class EventQueue
{

public:

	static EventQueue &Instance();

	void Enqueue(struct ServerEvent event);
	struct ServerEvent Dequeue();

	//Returns true if the queue is empty
	bool Empty();

	//Deletes all entries in the queue
	void Clear();


private:

	EventQueue();

	static EventQueue *m_instance;
	//TODO: locking on this class
	std::queue<struct ServerEvent> m_queue;

};

}

#endif /* EVENTQUEUE_H_ */

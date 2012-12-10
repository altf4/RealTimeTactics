//============================================================================
// Name    : OgreMatchLobbyEvents.cpp
// Author   : AltF4
// Copyright  : 2012, GNU GPLv3
// Description : Match Lobby events which have happened on the server, and the
//					client is being notified about.
//============================================================================

#include "OgreMatchLobbyEvents.h"
#include "EventQueue.h"

using namespace RTT;

void OgreMatchLobbyEvents::UI_TeamChangedSignal(uint32_t playerID, enum TeamNumber newTeam)
{
	struct ServerEvent event;
	event.m_appState = IN_MATCH_LOBBY;
	event.m_type = EVENT_TEAM_CHANGED;
	event.m_playerID = playerID;
	event.m_newTeam = newTeam;
	EventQueue::Instance().Enqueue(event);
}

void OgreMatchLobbyEvents::UI_KickFromMatchSignal()
{
	struct ServerEvent event;
	event.m_appState = IN_MATCH_LOBBY;
	event.m_type = EVENT_KICKED;
	EventQueue::Instance().Enqueue(event);
}

void OgreMatchLobbyEvents::UI_PlayerLeftSignal(uint32_t playerID, uint32_t leaderID)
{
	struct ServerEvent event;
	event.m_appState = IN_MATCH_LOBBY;
	event.m_type = EVENT_PLAYER_LEFT;
	event.m_playerID = playerID;
	event.m_leaderID = leaderID;
	EventQueue::Instance().Enqueue(event);
}

void OgreMatchLobbyEvents::UI_PlayerJoinedSignal(struct PlayerDescription player)
{
	struct ServerEvent event;
	event.m_appState = IN_MATCH_LOBBY;
	event.m_type = EVENT_PLAYER_JOINED;
	event.m_newPlayer = player;
	EventQueue::Instance().Enqueue(event);
}

void OgreMatchLobbyEvents::UI_ColorChangedSignal(uint32_t playerID, enum TeamColor color)
{
	struct ServerEvent event;
	event.m_appState = IN_MATCH_LOBBY;
	event.m_type = EVENT_TEAM_COLOR_CHANGED;
	event.m_playerID = playerID;
	event.m_newColor = color;
	EventQueue::Instance().Enqueue(event);
}

void OgreMatchLobbyEvents::UI_MapChangedSignal(struct MapDescription mapDesc)
{
	struct ServerEvent event;
	event.m_appState = IN_MATCH_LOBBY;
	event.m_type = EVENT_MAP_CHANGED;
	event.m_newMap = mapDesc;
	EventQueue::Instance().Enqueue(event);
}

void OgreMatchLobbyEvents::UI_GamespeedChangedSignal(enum GameSpeed speed)
{
	struct ServerEvent event;
	event.m_appState = IN_MATCH_LOBBY;
	event.m_type = EVENT_GAMESPEED_CHANGED;
	event.m_newGamespeed = speed;
	EventQueue::Instance().Enqueue(event);
}

void OgreMatchLobbyEvents::UI_VictoryCondChangedSignal(enum VictoryCondition victory)
{
	struct ServerEvent event;
	event.m_appState = IN_MATCH_LOBBY;
	event.m_type = EVENT_VICTORY_CONDITION_CHANGED;
	event.m_victCondition = victory;
	EventQueue::Instance().Enqueue(event);
}

void OgreMatchLobbyEvents::UI_ChangeLeaderSignal(uint32_t playerID)
{
	struct ServerEvent event;
	event.m_appState = IN_MATCH_LOBBY;
	event.m_type = EVENT_LEADER_CHANGED;
	event.m_leaderID = playerID;
	EventQueue::Instance().Enqueue(event);
}

void OgreMatchLobbyEvents::UI_MatchStartedSignal()
{
	struct ServerEvent event;
	event.m_appState = IN_MATCH_LOBBY;
	event.m_type = EVENT_MATCH_STARTED;
	EventQueue::Instance().Enqueue(event);
}

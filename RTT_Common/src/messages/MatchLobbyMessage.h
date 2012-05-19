//============================================================================
// Name        : MatchLobbyMessage.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Message class pertaining to the Match Lobby (not main lobby)
//============================================================================

#ifndef MATCHLOBBYMESSAGE_H_
#define MATCHLOBBYMESSAGE_H_

#include "Message.h"
#include "../Map.h"
#include "../Player.h"
#include "../Enums.h"

namespace RTT
{

enum MatchLobbyType: char
{
	MATCH_LEAVE_NOTIFICATION = 0,
	MATCH_LEAVE_ACKNOWLEDGE,
	//Change what team a player is on
	CHANGE_TEAM_REQUEST,
	CHANGE_TEAM_REPLY,
	//Start the match
	START_MATCH_REQUEST,
	START_MATCH_REPLY,
	//Change color of a player
	CHANGE_COLOR_REQUEST,
	CHANGE_COLOR_REPLY,
	//Change the map
	CHANGE_MAP_REQUEST,
	CHANGE_MAP_REPLY,
	//Change victory condition
	CHANGE_VICTORY_COND_REQUEST,
	CHANGE_VICTORY_COND_REPLY,
	//Change game speed
	CHANGE_GAME_SPEED_REQUEST,
	CHANGE_GAME_SPEED_REPLY,
	//Change match leader
	CHANGE_LEADER_REQUEST,
	CHANGE_LEADER_REPLY,
	//Kick player
	KICK_PLAYER_REQUEST,
	KICK_PLAYER_REPLY,
	//ConnectBack
	CALLBACK_REGISTER,

	//***********************
	// (Initiated by server)
	//************************
	//Someone's team changed
	TEAM_CHANGED_NOTIFICATION,
	TEAM_CHANGED_ACK,
	//Leader kicked you out of match
	KICKED_FROM_MATCH_NOTIFICATION,
	KICKED_FROM_MATCH_ACK,
	//Someone else left the match
	PLAYER_LEFT_MATCH_NOTIFICATION,
	PLAYER_LEFT_MATCH_ACK,
	//Someone joined the match
	PLAYER_JOINED_MATCH_NOTIFICATION,
	PLAYER_JOINED_MATCH_ACK,
	//Someone's color changed
	COLOR_CHANGED_NOTIFICATION,
	COLOR_CHANGED_ACK,
	//Leader changed the map
	MAP_CHANGED_NOTIFICATION,
	MAP_CHANGED_ACK,
	//Leader changed the game speed
	GAME_SPEED_CHANGED_NOTIFICATION,
	GAME_SPEED_CHANGED_ACK,
	//Victory condition changed
	VICTORY_COND_CHANGED_NOTIFICATION,
	VICTORY_COND_CHANGED_ACK,
	//Leader has changed
	CHANGE_LEADER_NOTIFICATION,
	CHANGE_LEADER_ACK,
	//The match has started!
	MATCH_START_NOTIFICATION,
	MATCH_START_ACK,
};

class MatchLobbyMessage: public Message
{
public:

	enum TeamNumber m_newTeam;
	enum TeamColor m_newColor;
	enum VictoryCondition m_newVictCond;
	enum GameSpeed m_newSpeed;
	bool m_changeAccepted;
	struct MapDescription m_mapDescription;
	uint32_t m_playerID;
	struct PlayerDescription m_playerDescription;
	uint32_t m_portNum;
	uint32_t m_newLeaderID;

	enum MatchLobbyType m_matchLobbyType;

	MatchLobbyMessage(enum MatchLobbyType type);
	~MatchLobbyMessage();
	MatchLobbyMessage(char *buffer, uint length);
	char *Serialize(uint *length);

};

}

#endif /* MATCHLOBBYMESSAGE_H_ */

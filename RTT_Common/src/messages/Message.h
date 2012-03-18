//============================================================================
// Name        : Message.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Message class which is passed to/from client/server
//============================================================================

#ifndef MESSAGE_H_
#define MESSAGE_H_

#define MESSAGE_MIN_SIZE	sizeof(enum MessageType)
#define USERNAME_MAX_LENGTH	20

#include <stdlib.h>
#include <vector>
#include <unistd.h>
#include <stdint.h>

using namespace std;

namespace RTT
{

enum MessageType: char
{
	//****************
	//	AuthMessage
	//****************
	//Messages for first contacting the server and authenticating
	CLIENT_HELLO = 0,
	SERVER_HELLO,
	CLIENT_AUTH,
	SERVER_AUTH_REPLY,

	//****************
	//	LobbyMessage
	//****************
	//Querying matches
	MATCH_LIST_REQUEST,
	MATCH_LIST_REPLY,
	//Creating a new match
	MATCH_CREATE_REQUEST,
	MATCH_CREATE_OPTIONS_AVAILABLE,
	MATCH_CREATE_OPTIONS_CHOSEN,
	MATCH_CREATE_REPLY,
	//Joining a match already created
	MATCH_JOIN_REQUEST,
	MATCH_JOIN_REPLY,
	//Ask server for stats
	SERVER_STATS_REQUEST,
	SERVER_STATS_REPLY,
	//Exit the server
	MATCH_EXIT_SERVER_NOTIFICATION,
	MATCH_EXIT_SERVER_ACKNOWLEDGE,

	//*********************
	//	MatchLobbyMessage
	// (Initiated by client)
	//*********************
	//leaving a match you're in
	MATCH_LEAVE_NOTIFICATION,
	MATCH_LEAVE_ACKNOWLEDGE,
	//Change what team a player is on
	CHANGE_TEAM_REQUEST,
	CHANGE_TEAM_REPLY,
	//Request match to begin
	START_MATCH_REQUEST,
	START_MATCH_REPLY,
	//Register for the match start (play the match)
	REGISTER_FOR_MATCH,
	REGISTER_REPLY,
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

	//************************
	//	MatchLobbyMessage
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

	//*******************
	//   Game Message
	//*******************
	MOVE_UNIT_DIRECTION_REQUEST,
	MOVE_UNIT_DIRECTION_REPLY,
	UNIT_MOVED_DIRECTION_NOTICE,
	UNIT_MOVED_DIRECTION_ACK,

	//*******************
	//   Error Message
	//*******************
	MESSAGE_ERROR,

	//TODO: Fill this out as they become needed / invented

};

struct VersionNumber
{
	uint32_t major;
	uint32_t minor;
	uint32_t rev;
};

class Message
{
public:

	//Not ever sent. Just used to return errors
	bool serializeError;

	enum MessageType type;

	//Plain old constructor
	Message();

	virtual char *Serialize(uint *length);
	static Message *Deserialize(char *buffer, uint length);

	static Message *ReadMessage(int connectFD);
	static bool WriteMessage(Message *message, int connectFD);

};

}


#endif /* MESSAGE_H_ */

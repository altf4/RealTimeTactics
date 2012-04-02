//============================================================================
// Name        : RTT_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : RTT Game Server
//============================================================================

#ifndef LOF_SERVER_H_
#define LOF_SERVER_H_

#include <vector>
#include "Action.h"
#include "Match.h"
#include "messages/LobbyMessage.h"
#include "messages/AuthMessage.h"
#include "messages/MatchLobbyMessage.h"
#include <google/dense_hash_map>

#define CHARGE_MAX 100
//TODO: Read this from config file
#define MATCHES_PER_PAGE 10
//TODO: Read this from config file
#define MAX_PLAYERS_IN_MATCH 8

#define SERVER_VERSION_MAJOR	0
#define SERVER_VERSION_MINOR	0
#define SERVER_VERSION_REV		1

#define DEFAULT_SERVER_PORT 23000

namespace RTT
{

struct eqint
{
  bool operator()(int s1, int s2) const
  {
    return (s1 == s2);
  }
};

//Define types, so it's easier to refer to later
//Key : Player ID
typedef google::dense_hash_map<int, Player*, std::tr1::hash<int>, eqint> PlayerList;
//Key : Match unique ID
typedef google::dense_hash_map<int, Match*, std::tr1::hash<int>, eqint> MatchList;
//Key: Player's unique ID
typedef google::dense_hash_map<int, int, std::tr1::hash<int>, eqint> ConnectBackWaitPool;

std::string Usage();
void ProcessRound(Match *match);

void *MainListen(void * param);
void *CallbackListen(void * param);

void *MainClientThread(void * parm);
void *CallbackClientThread(void * parm);


//Gets match descriptions from the matchlist
//	page: specifies which block of matches to get
//	descArray: output array where matches are written to
//	Returns: The number of matches written
uint GetMatchDescriptions(uint page, MatchDescription *descArray);

//Gets match descriptions from the playerlist
//	matchID: What match to get the players from
//	descArray: output array where matches are written to
//		(Length = MAX_PLAYERS_IN_MATCH)
//	Returns: The number of matches written
uint GetPlayerDescriptions(uint matchID, PlayerDescription *descArray);

//Creates a new match and places it into matchList
//	Returns: The unique ID of the new match
uint RegisterNewMatch(Player *player, struct MatchOptions options);

//Make player join specified match
//	Sets the variables within player and match properly
//	Returns an enum of the success or failure condition
enum LobbyResult JoinMatch(Player *player, uint matchID);

//Make player leave specified match
//	Sets the variables within player and match properly
//	Returns success or failure
bool LeaveMatch(Player *player);

//Player has quit the server, clean up any references to it
//	Deletes the player object
void QuitServer(Player *player);

}

#endif /* LOF_SERVER_H_ */

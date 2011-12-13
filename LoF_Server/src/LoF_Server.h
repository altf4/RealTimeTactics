//============================================================================
// Name        : LoF_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : LoF Game Server
//============================================================================

#ifndef LOF_SERVER_H_
#define LOF_SERVER_H_

#include <vector>
#include "Action.h"
#include "Match.h"
#include "messages/LobbyMessage.h"
#include "messages/AuthMessage.h"
#include <google/dense_hash_map>

using namespace std;
using namespace LoF;
using google::dense_hash_map;
using tr1::hash;

#define CHARGE_MAX 100
//TODO: Read this from config file
#define MATCHES_PER_PAGE 10
//TODO: Read this from config file
#define MAX_PLAYERS_IN_MATCH 8

#define SERVER_VERSION_MAJOR	0
#define SERVER_VERSION_MINOR	0
#define SERVER_VERSION_REV		1

struct eqstr
{
  bool operator()(const char* s1, const char* s2) const
  {
    return (s1 == s2) || (s1 && s2 && strcmp(s1, s2) == 0);
  }
};

struct eqint
{
  bool operator()(int s1, int s2) const
  {
    return (s1 == s2);
  }
};

//Define types, so it's easier to refer to later
//Key : Player name as a c string
typedef dense_hash_map<const char*, Player*, hash<const char*>, eqstr> PlayerList;
//Key : Match unique ID
typedef dense_hash_map<int, Match*, hash<int>, eqint> MatchList;

static PlayerList playerList;
static MatchList matchList;

//The mast match ID given out
static uint lastMatchID;

string Usage();
void ProcessRound(Match *match);
void *ClientThread(void * parm);

//Gets match descriptions from the matchlist
//	page: specifies which block of matches to get
//	descArray: output array where matches are written to
//	Returns: The number of matches written
uint GetMatchDescriptions(uint page, MatchDescription *descArray);

//Creates a new match and places it into matchList
//	Returns: The unique ID of the new match
uint RegisterNewMatch(Player *player);

//Make player join specified match
//	Sets the variables within player and match properly
//	Returns an enum of the success or failure condition
enum LobbyResult JoinMatch(Player *player, uint matchID);

//Make player leave specified match
//	Sets the variables within player and match properly
//	Returns success or failure
bool LeaveMatch(Player *player, uint matchID);

#endif /* LOF_SERVER_H_ */

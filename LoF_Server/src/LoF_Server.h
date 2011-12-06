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
#include "messages/AuthMessage.h"
#include <google/dense_hash_map>

using namespace std;
using namespace LoF;
using google::dense_hash_map;
using tr1::hash;

#define CHARGE_MAX 100

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

static dense_hash_map<const char*, Player*, hash<const char*>, eqstr> playerList;

string Usage();

void ProcessRound(Match *match);

void *ClientThread(void * parm);

#endif /* LOF_SERVER_H_ */

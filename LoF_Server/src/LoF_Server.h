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

using namespace std;
using namespace LoF;

#define CHARGE_MAX 100

#define SERVER_VERSION_MAJOR	0
#define SERVER_VERSION_MINOR	0
#define SERVER_VERSION_REV		1

string Usage();

void ProcessRound(Match *match);

void *ClientThread(void * parm);

#endif /* LOF_SERVER_H_ */

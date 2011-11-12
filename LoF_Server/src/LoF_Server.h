//============================================================================
// Name        : LoF_Server.cpp
// Author      : AltF4
// Copyright   : GNU GPLv3
// Description : LoF Game Server
//============================================================================

#ifndef LOF_SERVER_H_
#define LOF_SERVER_H_

#include <vector>
#include "Action.h"

using namespace std;
using namespace LoF;

#define CHARGE_MAX 100

// chargingActions is the global list of actions which have not yet been triggered
// chargedActions is the list of actions which are on queue to be triggered
vector <Action*> chargingActions, chargedActions;

string Usage();


#endif /* LOF_SERVER_H_ */

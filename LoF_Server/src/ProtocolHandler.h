//============================================================================
// Name        : LoF_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Code for handling socket IO on the server side
//============================================================================

#ifndef PROTOCOLHANDLER_H_
#define PROTOCOLHANDLER_H_

#include <vector>
#include "messages/AuthMessage.h"

using namespace std;

namespace LoF
{

bool AuthenticateNewClient(int connectFD);

Message *ReadMessage(int connectFD);

bool WriteMessage(Message *message, int connectFD);

}
#endif /* PROTOCOLHANDLER_H_ */

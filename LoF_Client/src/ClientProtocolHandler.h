//============================================================================
// Name        : LoF_Client.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Code for handling socket IO on the server side
//============================================================================

#ifndef PROTOCOLHANDLER_H_
#define PROTOCOLHANDLER_H_

namespace LoF
{

bool AuthToServer(int connectFD, string username, unsigned char *hashedPassword);

}

#endif /* PROTOCOLHANDLER_H_ */

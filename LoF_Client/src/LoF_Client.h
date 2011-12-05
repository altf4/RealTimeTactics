//============================================================================
// Name        : LoF_Client.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : LoF Game Client
//============================================================================

#ifndef LOF_CLIENT_H_
#define LOF_CLIENT_H_

#define CLIENT_VERSION_MAJOR	0
#define CLIENT_VERSION_MINOR	0
#define CLIENT_VERSION_REV		1

#include <string>
#include <openssl/sha.h>

using namespace std;

bool GetPasswordTerminal(unsigned char *hash);
string Usage();

#endif /* LOF_CLIENT_H_ */

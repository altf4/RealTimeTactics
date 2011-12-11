//============================================================================
// Name        : Unit.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Message class sent while player is in the server lobby
//============================================================================


#include "Message.h"
#include "../Match.h"

using namespace std;

namespace LoF
{

class LobbyMessage: public Message
{
	uint requestedPage;
	uint returnedMatchesCount;
	struct MatchDescription *matchDescription;

	LobbyMessage();
	~LobbyMessage();
	LobbyMessage(char *buffer, uint length);
	char *Serialize(uint *length);
};

}

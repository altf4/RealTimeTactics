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

enum LobbyResult: char
{
	//Success
	LOBBY_SUCCESS = 0,

	//Failure, Invalid options
	INVALID_MAX_PLAYERS,

	//Failure joining
	MATCH_IS_FULL,
	MATCH_DOESNT_EXIST,
	NOT_ALLOWED_IN,

	//Failure, General server stuff
	TOO_BUSY,
	SHUTTING_DOWN_SOON,
};

class LobbyMessage: public Message
{
	uint requestedPage;
	uint returnedMatchesCount;
	//A list of queried matches
	struct MatchDescription *matchDescriptions;
	//Newly created match
	struct MatchDescription matchDescription;
	uint maxPlayers;
	enum LobbyResult error;


	LobbyMessage();
	~LobbyMessage();
	LobbyMessage(char *buffer, uint length);
	char *Serialize(uint *length);
};

}

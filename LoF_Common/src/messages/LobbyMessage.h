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
	ALREADY_IN_MATCH,

	//Failure leaving
	NOT_IN_THAT_MATCH,

	//Failure, General server stuff
	TOO_BUSY,
	SHUTTING_DOWN_SOON,

	//Protocol error
	SPOKE_OUT_OF_TURN,
};

class LobbyMessage: public Message
{
public:
	uint requestedPage;
	uint returnedMatchesCount;
	//A list of queried matches
	struct MatchDescription *matchDescriptions;
	//Newly created match
	struct MatchDescription matchDescription;
	struct MatchOptions options;
	enum LobbyResult error;
	//ID of a match to join
	uint ID;


	LobbyMessage();
	~LobbyMessage();
	LobbyMessage(char *buffer, uint length);
	char *Serialize(uint *length);
};

}

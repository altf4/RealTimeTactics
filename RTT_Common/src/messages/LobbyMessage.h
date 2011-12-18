//============================================================================
// Name        : Unit.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Message class sent while player is in the server lobby
//============================================================================


#include "Message.h"
#include "../Match.h"

using namespace std;

namespace RTT
{

enum LobbyResult: char
{
	//Success
	LOBBY_SUCCESS = 0,

	//Failure joining
	LOBBY_MATCH_IS_FULL,
	LOBBY_MATCH_DOESNT_EXIST,
	LOBBY_NOT_ALLOWED_IN,
	LOBBY_ALREADY_IN_MATCH,
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
	//ID of a match to join
	uint ID;


	LobbyMessage();
	~LobbyMessage();
	LobbyMessage(char *buffer, uint length);
	char *Serialize(uint *length);
};

}

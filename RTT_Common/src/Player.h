//============================================================================
// Name        : RTT_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Collects all information specific to one player
//============================================================================

#ifndef PLAYER_H_
#define PLAYER_H_

#define PLAYER_NAME_SIZE 20
#define PLAYER_DESCR_SIZE sizeof(uint32_t) + PLAYER_NAME_SIZE + \
	sizeof(enum TeamNumber) + sizeof(enum TeamColor)

#include <vector>
#include "Unit.h"
#include "Match.h"
#include <string>
#include "Enums.h"
#include <stdint.h>
#include <pthread.h>

using namespace std;

namespace RTT
{

//Forward declaration to avoid self-reference
class Match;

//A fixed size representation of the Player suitable for providing
//	to other Players for display
struct PlayerDescription
{
	uint32_t ID;
	char name[PLAYER_NAME_SIZE];
	enum TeamNumber team;
	enum TeamColor color;
};

class Player
{
public:

	Player();

	//For when c strings are easier, or std::strings are
	Player(char *username, uint newID);
	Player(string username, uint newID);

	string GetName();
	uint GetID();
	enum TeamNumber GetTeam();
	enum TeamColor GetColor();
	int GetCallbackSocket();
	uint GetCurrentMatchID();
	struct PlayerDescription GetDescription();

	void SetName(string newName);
	void SetID(uint newID);
	void SetTeam(enum TeamNumber newTeam);
	void SetColor(enum TeamColor newColor);
	void SetCallbackSocket(int socket);
	void SetCurrentMatchID(int matchID);

private:
	pthread_rwlock_t lock;
	string name;
	uint ID;
	enum TeamNumber team;
	enum TeamColor color;
	//Socket to receive MatchLobby messages from server on
	int callbackSocket;
	struct PlayerDescription description;
	vector <Unit*> units;
	//The match that this player is currently in
	uint currentMatchID;
};

}
#endif /* PLAYER_H_ */

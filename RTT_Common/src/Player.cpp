//============================================================================
// Name        : RTT_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Collects all information specific to one player
//============================================================================

#include "Player.h"

using namespace RTT;

Player::Player()
{
	currentMatchID = 0;
	callbackSocket = -1;
	pthread_rwlock_init(&lock, NULL);
}

Player::Player(char *username, uint newID)
{
	name = string(username);
	strncpy(description.name, username, sizeof(description.name));
	currentMatchID = 0;
	ID = newID;
	description.ID = newID;
	callbackSocket = -1;
	pthread_rwlock_init(&lock, NULL);
}

Player::Player(string username, uint newID)
{
	name = username;
	strncpy(description.name, username.c_str(), sizeof(description.name));
	currentMatchID = 0;
	ID = newID;
	description.ID = newID;
	callbackSocket = -1;
	pthread_rwlock_init(&lock, NULL);
}

string Player::GetName()
{
	pthread_rwlock_rdlock(&lock);
	string nameCopy = name;
	pthread_rwlock_unlock(&lock);
	return nameCopy;
}

uint Player::GetID()
{
	pthread_rwlock_rdlock(&lock);
	uint ID_Copy = ID;
	pthread_rwlock_unlock(&lock);
	return ID_Copy;
}

enum TeamNumber Player::GetTeam()
{
	pthread_rwlock_rdlock(&lock);
	enum TeamNumber teamCopy = team;
	pthread_rwlock_unlock(&lock);
	return teamCopy;
}

enum TeamColor Player::GetColor()
{
	pthread_rwlock_rdlock(&lock);
	enum TeamColor colorCopy = color;
	pthread_rwlock_unlock(&lock);
	return colorCopy;
}

int Player::GetCallbackSocket()
{
	pthread_rwlock_rdlock(&lock);
	int socketCopy = callbackSocket;
	pthread_rwlock_unlock(&lock);
	return socketCopy;
}

uint Player::GetCurrentMatchID()
{
	pthread_rwlock_rdlock(&lock);
	uint matchIDCopy = currentMatchID;
	pthread_rwlock_unlock(&lock);
	return matchIDCopy;
}

struct PlayerDescription Player::GetDescription()
{
	pthread_rwlock_rdlock(&lock);
	struct PlayerDescription desrcCopy = description;
	pthread_rwlock_unlock(&lock);
	return desrcCopy;
}

void Player::SetName(string newName)
{
	pthread_rwlock_wrlock(&lock);
	name = newName;
	strncpy(description.name, newName.c_str(), sizeof(description.name));
	pthread_rwlock_unlock(&lock);
}

void Player::SetID(uint newID)
{
	pthread_rwlock_wrlock(&lock);
	ID = newID;
	description.ID = newID;
	pthread_rwlock_unlock(&lock);
}

void Player::SetTeam(enum TeamNumber newTeam)
{
	pthread_rwlock_wrlock(&lock);
	team = newTeam;
	description.team = newTeam;
	pthread_rwlock_unlock(&lock);
}

void Player::SetColor(enum TeamColor newColor)
{
	pthread_rwlock_wrlock(&lock);
	color = newColor;
	description.color = newColor;
	pthread_rwlock_unlock(&lock);
}

void Player::SetCallbackSocket(int socket)
{
	pthread_rwlock_wrlock(&lock);
	callbackSocket = socket;
	pthread_rwlock_unlock(&lock);
}

void Player::SetCurrentMatchID(int matchID)
{
	pthread_rwlock_wrlock(&lock);
	currentMatchID = matchID;
	pthread_rwlock_unlock(&lock);
}

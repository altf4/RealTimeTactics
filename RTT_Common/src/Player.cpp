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
	m_currentMatchID = 0;
	m_callbackSocket = -1;
	pthread_rwlock_init(&m_lock, NULL);
}

Player::Player(char *username, uint newID)
{
	m_name = string(username);
	strncpy(m_description.m_name, username, sizeof(m_description.m_name));
	m_currentMatchID = 0;
	m_ID = newID;
	m_description.m_ID = newID;
	m_callbackSocket = -1;
	pthread_rwlock_init(&m_lock, NULL);
}

Player::Player(string username, uint newID)
{
	m_name = username;
	strncpy(m_description.m_name, username.c_str(), sizeof(m_description.m_name));
	m_currentMatchID = 0;
	m_ID = newID;
	m_description.m_ID = newID;
	m_callbackSocket = -1;
	pthread_rwlock_init(&m_lock, NULL);
}

string Player::GetName()
{
	pthread_rwlock_rdlock(&m_lock);
	string nameCopy = m_name;
	pthread_rwlock_unlock(&m_lock);
	return nameCopy;
}

uint Player::GetID()
{
	pthread_rwlock_rdlock(&m_lock);
	uint ID_Copy = m_ID;
	pthread_rwlock_unlock(&m_lock);
	return ID_Copy;
}

enum TeamNumber Player::GetTeam()
{
	pthread_rwlock_rdlock(&m_lock);
	enum TeamNumber teamCopy = m_team;
	pthread_rwlock_unlock(&m_lock);
	return teamCopy;
}

enum TeamColor Player::GetColor()
{
	pthread_rwlock_rdlock(&m_lock);
	enum TeamColor colorCopy = m_color;
	pthread_rwlock_unlock(&m_lock);
	return colorCopy;
}

int Player::GetCallbackSocket()
{
	pthread_rwlock_rdlock(&m_lock);
	int socketCopy = m_callbackSocket;
	pthread_rwlock_unlock(&m_lock);
	return socketCopy;
}

uint Player::GetCurrentMatchID()
{
	pthread_rwlock_rdlock(&m_lock);
	uint matchIDCopy = m_currentMatchID;
	pthread_rwlock_unlock(&m_lock);
	return matchIDCopy;
}

struct PlayerDescription Player::GetDescription()
{
	pthread_rwlock_rdlock(&m_lock);
	struct PlayerDescription desrcCopy = m_description;
	pthread_rwlock_unlock(&m_lock);
	return desrcCopy;
}

void Player::SetName(string newName)
{
	pthread_rwlock_wrlock(&m_lock);
	m_name = newName;
	strncpy(m_description.m_name, newName.c_str(), sizeof(m_description.m_name));
	pthread_rwlock_unlock(&m_lock);
}

void Player::SetID(uint newID)
{
	pthread_rwlock_wrlock(&m_lock);
	m_ID = newID;
	m_description.m_ID = newID;
	pthread_rwlock_unlock(&m_lock);
}

void Player::SetTeam(enum TeamNumber newTeam)
{
	pthread_rwlock_wrlock(&m_lock);
	m_team = newTeam;
	m_description.m_team = newTeam;
	pthread_rwlock_unlock(&m_lock);
}

void Player::SetColor(enum TeamColor newColor)
{
	pthread_rwlock_wrlock(&m_lock);
	m_color = newColor;
	m_description.m_color = newColor;
	pthread_rwlock_unlock(&m_lock);
}

void Player::SetCallbackSocket(int socket)
{
	pthread_rwlock_wrlock(&m_lock);
	m_callbackSocket = socket;
	pthread_rwlock_unlock(&m_lock);
}

void Player::SetCurrentMatchID(int matchID)
{
	pthread_rwlock_wrlock(&m_lock);
	m_currentMatchID = matchID;
	pthread_rwlock_unlock(&m_lock);
}

//============================================================================
// Name        : RTT_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Collects all information specific to one player
//============================================================================

#include "Player.h"

#include "string.h"

using namespace RTT;
using namespace std;

Player::Player()
{
	m_currentMatchID = 0;
	m_socketFD = -1;
	pthread_rwlock_init(&m_lock, NULL);
}

Player::Player(char *username, uint newID)
{
	m_name = string(username);
	strncpy(m_description.m_name, username, sizeof(m_description.m_name));
	m_currentMatchID = 0;
	m_ID = newID;
	m_description.m_ID = newID;
	m_socketFD = -1;
	pthread_rwlock_init(&m_lock, NULL);
}

Player::Player(string username, uint newID)
{
	m_name = username;
	strncpy(m_description.m_name, username.c_str(), sizeof(m_description.m_name));
	m_currentMatchID = 0;
	m_ID = newID;
	m_description.m_ID = newID;
	m_socketFD = -1;
	pthread_rwlock_init(&m_lock, NULL);
}

string Player::GetName()
{
	Lock lock(&m_lock, READ_LOCK);
	return m_name;
}

uint Player::GetID()
{
	Lock lock(&m_lock, READ_LOCK);
	return m_ID;
}

enum TeamNumber Player::GetTeam()
{
	Lock lock(&m_lock, READ_LOCK);
	return m_team;
}

enum TeamColor Player::GetColor()
{
	Lock lock(&m_lock, READ_LOCK);
	return m_color;
}

int Player::GetSocket()
{
	Lock lock(&m_lock, READ_LOCK);
	return m_socketFD;
}

uint Player::GetCurrentMatchID()
{
	Lock lock(&m_lock, READ_LOCK);
	return m_currentMatchID;
}

struct PlayerDescription Player::GetDescription()
{
	Lock lock(&m_lock, READ_LOCK);
	return m_description;
}

void Player::SetName(string newName)
{
	Lock lock(&m_lock, WRITE_LOCK);
	m_name = newName;
	strncpy(m_description.m_name, newName.c_str(), sizeof(m_description.m_name));
}

void Player::SetID(uint newID)
{
	Lock lock(&m_lock, WRITE_LOCK);
	m_ID = newID;
	m_description.m_ID = newID;
}

void Player::SetTeam(enum TeamNumber newTeam)
{
	Lock lock(&m_lock, WRITE_LOCK);
	m_team = newTeam;
	m_description.m_team = newTeam;
}

void Player::SetColor(enum TeamColor newColor)
{
	Lock lock(&m_lock, WRITE_LOCK);
	m_color = newColor;
	m_description.m_color = newColor;
}

void Player::SetSocket(int socket)
{
	Lock lock(&m_lock, WRITE_LOCK);
	m_socketFD = socket;
}

void Player::SetCurrentMatchID(int matchID)
{
	Lock lock(&m_lock, WRITE_LOCK);
	m_currentMatchID = matchID;
}

//============================================================================
// Name        : RTT_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : RTT Game Server
//============================================================================

#include <iostream>
#include "Unit.h"
#include "RTT_Server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <algorithm>
#include <pthread.h>
#include "ServerProtocolHandler.h"
#include <iterator>

using namespace std;
using namespace RTT;

PlayerList playerList;
MatchList matchList;

//The mast match ID given out
uint lastMatchID;
uint lastPlayerID;

pthread_rwlock_t playerListLock;
pthread_rwlock_t matchListLock;
pthread_rwlock_t matchIDLock;
pthread_rwlock_t playerIDLock;

int main(int argc, char **argv)
{
	matchList.set_empty_key(-1);
	matchList.set_deleted_key(-2);
	playerList.set_empty_key(-1);
	playerList.set_deleted_key(-2);

	pthread_rwlock_init(&playerListLock, NULL);
	pthread_rwlock_init(&matchListLock, NULL);
	pthread_rwlock_init(&matchIDLock, NULL);
	pthread_rwlock_init(&playerIDLock, NULL);

	pthread_t threadID;
	int c;
	uint serverPortNumber;

	bool portEntered = false;

	while ((c = getopt(argc, argv, ":p:")) != -1)
	{
		switch (c)
		{
			case 'p':
			{
				char *errString;
				serverPortNumber = strtol(optarg, &errString, 10);
				if( *errString != '\0' || optarg == '\0')
				{
					//Error occurred
					cerr << "You entered an invalid port number\n";
					cerr << Usage();
					exit(-1);
				}
				portEntered = true;
				break;
			}
			case '?':
			{
				cerr << Usage();
				exit(-1);
				break;
			}
		}
	}

	//Check that all parameters were entered
	if( !portEntered )
	{
		cerr << "You did not enter a port number\n";
		cerr << Usage();
		exit(-1);
	}

	//Set up the TCP socket
	struct sockaddr_in stSockAddr;
	int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(-1 == SocketFD)
	{
		perror("can not create socket");
		exit(EXIT_FAILURE);
	}
	int optval = 1;
	setsockopt(SocketFD, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

	memset(&stSockAddr, 0, sizeof(stSockAddr));

	stSockAddr.sin_family = AF_INET;
	stSockAddr.sin_port = htons(serverPortNumber);
	stSockAddr.sin_addr.s_addr = INADDR_ANY;

	if(-1 == bind(SocketFD,(struct sockaddr *)&stSockAddr, sizeof(stSockAddr)))
	{
		perror("error bind failed");
		close(SocketFD);
		exit(EXIT_FAILURE);
	}

	if(-1 == listen(SocketFD, 10))
	{
		perror("error listen failed");
		close(SocketFD);
		exit(EXIT_FAILURE);
	}

	//Main loop, just listens for new TCP connections and sends them off to ClientThread
	for(;;)
	{
		int ConnectFD = accept(SocketFD, NULL, NULL);

		if(0 > ConnectFD)
		{
			perror("error accept failed");
			close(SocketFD);
			exit(EXIT_FAILURE);
		}

		//Send the new connection off to another thread for handling
		pthread_create(&threadID, NULL, ClientThread, (void *) ConnectFD );
	}

	return 0;
}

void *ClientThread(void * parm)
{
	int ConnectFD = (int)parm;

	//First, authenticate the client
	Player *player = GetNewClient(ConnectFD);
	if( player == NULL )
	{
		cout << "ERROR: Authentication Failure\n";
		shutdown(ConnectFD, SHUT_RDWR);
		close(ConnectFD);
		return NULL;
	}

	cout << "Client: " << player->name << " Authenticated!\n";

	//*************************************
	// In the main lobby
	// Main Server Loop
	//*************************************
	while(true)
	{
		enum LobbyReturn lobbyReturn;
		lobbyReturn = ProcessLobbyCommand(ConnectFD, player);

		if(lobbyReturn == EXITING_SERVER)
		{
			cout << "Player: " << player->name << " has left.\n";
			QuitServer(player);
			return NULL;
		}

		if(lobbyReturn == STARTING_MATCH)
		{
			//TODO: START A MATCH!!
		}
	}

	return NULL;
}

//Processes one round of combat. (Can consist of many actions triggered)
void ProcessRound(Match *match)
{

	//Step 1: Increment all the charges on the charging actions
	for(uint i = 0; i < match->chargingActions.size(); i++)
	{
		match->chargingActions[i]->currentCharge += match->chargingActions[i]->speed;
	}

	//Step 2: Move any finished actions over to chargedActions
	for(uint i = 0; i < match->chargingActions.size(); i++)
	{
		if( match->chargingActions[i]->currentCharge >= CHARGE_MAX )
		{
			//Move the Action over
			match->chargedActions.push_back(match->chargingActions[i]);
			//Delete it from this list
			match->chargingActions[i] = NULL;
			match->chargingActions.erase( match->chargingActions.begin()+i );
			//Move the index back, since we just erased one elements
			i--;
		}
	}

	//Step 3: Sort the new charged list according to execution order
	sort(match->chargedActions.front(),
			match->chargedActions.back(), Action::CompareActions);

	//Step 4: Execute each of the charged actions, one by one
	while(match->chargedActions.size() > 0)
	{
		match->chargedActions[0]->Execute();
		match->chargingActions.erase( match->chargingActions.begin() );

		//Re-sort the actions, since new ones might have been added
		sort(match->chargedActions.front(),
				match->chargedActions.back(), Action::CompareActions);
		//TODO: This is probably inefficient. Find a better way than re-sorting every time
	}

}

//Gets match descriptions from the matchlist
//	page: specifies which block of matches to get
//	descArray: output array where matches are written to
//	Returns: The number of matches written
uint GetMatchDescriptions(uint page, MatchDescription *descArray)
{
	pthread_rwlock_rdlock(&matchListLock);
	MatchList::iterator it = matchList.begin();
	uint count = 0;

	if(matchList.empty())
	{
		pthread_rwlock_unlock(&matchListLock);
		return 0;
	}

	//Skip forward to the beginning of this page
	while(count < ( (page-1) * MATCHES_PER_PAGE))
	{
		it++;
		count++;

		if(it == matchList.end())
		{
			break;
		}
	}

	//Copy the matches in, one by one
	for(uint i = 0; i < MATCHES_PER_PAGE; i++)
	{
		if(it == matchList.end())
		{
			pthread_rwlock_unlock(&matchListLock);
			return i;
		}
		descArray[i] = it.pos->second->description;
		it++;
	}

	pthread_rwlock_unlock(&matchListLock);
	return MATCHES_PER_PAGE;
}

//Creates a new match and places it into matchList
//	Returns: The unique ID of the new match
//		returns 0 on error
uint RegisterNewMatch(Player *player, struct MatchOptions options)
{
	//The player's current match must be empty to join a new one
	if( player->currentMatch != NULL )
	{
		return 0;
	}
	pthread_rwlock_wrlock(&matchIDLock);
	uint matchID = ++lastMatchID;
	pthread_rwlock_unlock(&matchIDLock);

	Match *match = new Match();
	match->SetID(matchID);
	match->SetStatus(WAITING_FOR_PLAYERS);
	match->SetMaxPlayers(options.maxPlayers);
	match->SetCurrentPlayerCount(1);

	//Put the match in the global match list
	pthread_rwlock_wrlock(&matchListLock);
	matchList[matchID] = match;
	pthread_rwlock_unlock(&matchListLock);

	//Put the match in this player's current match
	player->currentMatch = match;
	//Put the player in this match's player list
	match->players.push_back(player);

	return match->GetID();
}

//Make player join specified match
//	Sets the variables within player and match properly
//	Returns an enum of the success or failure condition
enum LobbyResult JoinMatch(Player *player, uint matchID)
{
	pthread_rwlock_rdlock(&matchListLock);
	//The player's current match must be empty to join a new one
	if( player->currentMatch != NULL )
	{
		pthread_rwlock_unlock(&matchListLock);
		return LOBBY_ALREADY_IN_MATCH;
	}
	if( matchList[matchID] == NULL)
	{
		pthread_rwlock_unlock(&matchListLock);
		return LOBBY_MATCH_DOESNT_EXIST;
	}
	if( matchList[matchID]->players.size() == matchList[matchID]->GetMaxPlayers())
	{
		pthread_rwlock_unlock(&matchListLock);
		return LOBBY_MATCH_IS_FULL;
	}
	pthread_rwlock_unlock(&matchListLock);

	//TODO: Check for permission to enter
//	if(permission is not granted)
//	{
//		return NOT_ALLOWED_IN;
//	}

	pthread_rwlock_wrlock(&matchListLock);
	matchList[matchID]->players.push_back(player);
	//Increment match's player count
	matchList[matchID]->SetCurrentPlayerCount(
			matchList[matchID]->GetCurrentPlayerCount() +1 );
	player->currentMatch = matchList[matchID];
	pthread_rwlock_unlock(&matchListLock);

	return LOBBY_SUCCESS;
}

//Make player leave specified match
//	Sets the variables within player and match properly
//	If no players remain in the match afterward, then the match is deleted
//	Returns success or failure
bool LeaveMatch(Player *player, uint matchID)
{
	bool foundOne = false;
	pthread_rwlock_wrlock(&matchListLock);
	if( matchList[matchID] == NULL )
	{
		pthread_rwlock_unlock(&matchListLock);
		return false;
	}
	for(uint i = 0; i < matchList[matchID]->players.size(); i++)
	{
		//Find our player in the match' player list
		if( matchList[matchID]->players[i]->name == player->name)
		{
			matchList[matchID]->players.erase(matchList[matchID]->players.begin()+i);
			//Increment match's player count
			matchList[matchID]->SetCurrentPlayerCount(
					matchList[matchID]->GetCurrentPlayerCount() +1 );
			foundOne = true;
			break;
		}
	}
	if( !foundOne )
	{
		pthread_rwlock_unlock(&matchListLock);
		return false;
	}
	player->currentMatch = NULL;

	//If this was the last player in the match
	if( matchList[matchID]->players.empty() )
	{
		delete matchList[matchID];
		matchList.erase(matchID);
	}

	pthread_rwlock_unlock(&matchListLock);
	return true;
}

//Player has quit the server, clean up any references to it
//	Deletes the player object
void QuitServer(Player *player)
{
	if( player == NULL )
	{
		return;
	}
	if( player->currentMatch != NULL)
	{
		//Leave any matches currently in
		LeaveMatch(player, player->currentMatch->GetID());
	}

	int ID = player->ID;
	//Remove from the list of current players
	pthread_rwlock_wrlock(&playerListLock);
	playerList[ID] = NULL;
	playerList.erase(ID);
	pthread_rwlock_unlock(&playerListLock);

	delete player;
}

//Prints usage tips
string Usage()
{
	string out;

	out += "Line of Fire Server Usage:\n";
	out += "\t RTT_Server -p PORT\n\n";
	out += "\t -p PORT == TCP Port number to listen for connections on.\n";
	return out;
}


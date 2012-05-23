//============================================================================
// Name        : RTT_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : RTT Game Server
//============================================================================

#include <iostream>
#include "Unit.h"
#include "RTT_Server.h"
#include "messaging/MessageManager.h"
#include "ServerProtocolHandler.h"
#include "Player.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <algorithm>
#include <pthread.h>
#include <iterator>
#include <signal.h>

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

uint serverPortNumber;

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

	MessageManager::Initialize(DIRECTION_TO_CLIENT);

	// We expect write failures to occur but we want to handle them where
	// the error occurs rather than in a SIGPIPE handler.
	signal(SIGPIPE, SIG_IGN);

	int c;

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
		serverPortNumber = DEFAULT_SERVER_PORT;
	}

	//Set up the TCP sockets
	struct sockaddr_in stSockAddr;
	int mainSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(-1 == mainSocket)
	{
		perror("can not create socket");
		exit(EXIT_FAILURE);
	}
	int optval = 1;
	setsockopt(mainSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

	memset(&stSockAddr, 0, sizeof(stSockAddr));
	stSockAddr.sin_family = AF_INET;
	stSockAddr.sin_port = htons(serverPortNumber);
	stSockAddr.sin_addr.s_addr = INADDR_ANY;

	if(-1 == bind(mainSocket,(struct sockaddr *)&stSockAddr, sizeof(stSockAddr)))
	{
		perror("error bind failed");
		close(mainSocket);
		exit(EXIT_FAILURE);
	}

	if(-1 == listen(mainSocket, 10))
	{
		perror("error listen failed");
		close(mainSocket);
		exit(EXIT_FAILURE);
	}

	pthread_t mainThreadID;

	//Send the new connection off to another thread for handling
	pthread_create(&mainThreadID, NULL, MainListen, (void *) mainSocket );

	//TODO: stupid hack to keep the threads alive. replace later
	while(true)
	{
		sleep(1000);
	}
}

void *RTT::MainListen(void * param)
{
	intptr_t mainSocket = (intptr_t)param;
	//Main loop, just listens for new TCP connections and sends them off to MainClientThread
	for(;;)
	{
		intptr_t ConnectFD = accept(mainSocket, NULL, NULL);

		if(0 > ConnectFD)
		{
			perror("error accept failed");
			close(mainSocket);
			exit(EXIT_FAILURE);
		}
		pthread_t threadID;
		//Send the new connection off to another thread for handling
		pthread_create(&threadID, NULL, MainClientThread, (void *) ConnectFD );
	}

	return 0;
}

void *RTT::MainClientThread(void * parm)
{
	intptr_t ConnectFD = (intptr_t)parm;

	MessageManager::Instance().StartSocket(ConnectFD);
	cout << "xxxDEBUGxxx " << "Socket: " << ConnectFD << "started" << endl;

	//First, authenticate the client
	Player *player = GetNewClient(ConnectFD);
	if( player == NULL )
	{
		cout << "ERROR: Authentication Failure\n";
		shutdown(ConnectFD, SHUT_RDWR);
		close(ConnectFD);
		return NULL;
	}

	cout << "Client: " << player->GetName() << " Authenticated!\n";

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
			cout << "Player: " << player->GetName() << " has left.\n";
			QuitServer(player);
			return NULL;
		}

		//In the a Match Lobby
		if(lobbyReturn == IN_MATCH_LOBBY)
		{
			while( lobbyReturn == IN_MATCH_LOBBY)
			{
				lobbyReturn = ProcessMatchLobbyCommand(ConnectFD, player);
			}
			if( lobbyReturn == EXITING_SERVER )
			{
				cout << "Player: " << player->GetName() << " has left.\n";
				QuitServer(player);
				return NULL;
			}
			if( lobbyReturn == IN_GAME )
			{
				//TODO: Start the match!!!
			}
		}
	}

	return NULL;
}

//Processes one round of combat. (Can consist of many actions triggered)
void RTT::ProcessRound(Match *match)
{

	//Step 1: Increment all the charges on the charging actions
	for(uint i = 0; i < match->m_chargingActions.size(); i++)
	{
		match->m_chargingActions[i]->m_currentCharge += match->m_chargingActions[i]->m_speed;
	}

	//Step 2: Move any finished actions over to chargedActions
	for(uint i = 0; i < match->m_chargingActions.size(); i++)
	{
		if( match->m_chargingActions[i]->m_currentCharge >= CHARGE_MAX )
		{
			//Move the Action over
			match->m_chargedActions.push_back(match->m_chargingActions[i]);
			//Delete it from this list
			match->m_chargingActions[i] = NULL;
			match->m_chargingActions.erase( match->m_chargingActions.begin()+i );
			//Move the index back, since we just erased one elements
			i--;
		}
	}

	//Step 3: Sort the new charged list according to execution order
	sort(match->m_chargedActions.front(),
			match->m_chargedActions.back(), Action::CompareActions);

	//Step 4: Execute each of the charged actions, one by one
	while(match->m_chargedActions.size() > 0)
	{
		match->m_chargedActions[0]->Execute();
		match->m_chargingActions.erase( match->m_chargingActions.begin() );

		//Re-sort the actions, since new ones might have been added
		sort(match->m_chargedActions.front(),
				match->m_chargedActions.back(), Action::CompareActions);
		//TODO: This is probably inefficient. Find a better way than re-sorting every time
	}

}

//Gets match descriptions from the matchlist
//	page: specifies which block of matches to get
//	descArray: output array where matches are written to
//	Returns: The number of matches written
uint RTT::GetMatchDescriptions(uint page, MatchDescription *descArray)
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
		descArray[i] = it.pos->second->GetDescription();
		it++;
	}

	pthread_rwlock_unlock(&matchListLock);
	return MATCHES_PER_PAGE;
}

//Gets match descriptions from the playerlist
//	matchID: What match to get the players from
//	descArray: output array where matches are written to
//		(Length = MAX_PLAYERS_IN_MATCH)
//	Returns: The number of matches written
uint RTT::GetPlayerDescriptions(uint matchID, PlayerDescription *descArray)
{
	pthread_rwlock_rdlock(&matchListLock);

	if( matchList.count(matchID) == 0 )
	{
		pthread_rwlock_unlock(&matchListLock);
		return 0;
	}

	Match *joinedMatch = matchList[matchID];
	pthread_rwlock_unlock(&matchListLock);

	uint count = 0;
	for(uint i = 0; i < MAX_TEAMS; i++)
	{
		vector<struct PlayerDescription> descriptions =
				joinedMatch->m_teams[i]->GetPlayerDescriptions();
		for(uint j = 0; j < descriptions.size(); j++)
		{
			descArray[count] = descriptions[j];
			count++;
		}
	}
	return count;
}
//Creates a new match and places it into matchList
//	Returns: The unique ID of the new match
//		returns 0 on error
uint RTT::RegisterNewMatch(Player *player, struct MatchOptions options)
{
	//The player's current match must be empty to join a new one
	if( player->GetCurrentMatchID() != 0 )
	{
		return 0;
	}
	pthread_rwlock_wrlock(&matchIDLock);
	uint matchID = ++lastMatchID;
	pthread_rwlock_unlock(&matchIDLock);

	Match *match = new Match(player);
	match->SetID(matchID);
	match->SetStatus(WAITING_FOR_PLAYERS);
	match->SetMaxPlayers(options.m_maxPlayers);
	match->SetName(options.m_name);

	//Put the match in the global match list
	pthread_rwlock_wrlock(&matchListLock);
	matchList[matchID] = match;
	pthread_rwlock_unlock(&matchListLock);

	//Put the match in this player's current match
	player->SetCurrentMatchID(match->GetID());
	//Put the player in this match's player list
	match->AddPlayer(player, TEAM_1);

	return match->GetID();
}

//Make player join specified match
//	Sets the variables within player and match properly
//	Returns an enum of the success or failure condition
enum LobbyResult RTT::JoinMatch(Player *player, uint matchID)
{
	if( player == NULL )
	{
		return LOBBY_PLAYER_NULL;
	}

	//The player's current match must be empty to join a new one
	if( player->GetCurrentMatchID() != 0 )
	{
		return LOBBY_ALREADY_IN_MATCH;
	}

	pthread_rwlock_rdlock(&matchListLock);
	if( matchList.count(matchID) == 0)
	{
		pthread_rwlock_unlock(&matchListLock);
		return LOBBY_MATCH_DOESNT_EXIST;
	}
	Match *foundMatch = matchList[matchID];
	pthread_rwlock_unlock(&matchListLock);

	if( foundMatch->GetCurrentPlayerCount() == foundMatch->GetMaxPlayers())
	{
		return LOBBY_MATCH_IS_FULL;
	}

	//TODO: Check for permission to enter
//	if(permission is not granted)
//	{
//		return NOT_ALLOWED_IN;
//	}

	foundMatch->AddPlayer(player, TEAM_1);
	player->SetCurrentMatchID(foundMatch->GetID());

	return LOBBY_SUCCESS;
}

//Make player leave specified match
//	Sets the variables within player and match properly
//	If no players remain in the match afterward, then the match is deleted
//	Returns success or failure
bool RTT::LeaveMatch(Player *player)
{
	bool foundOne = false;
	if( player->GetCurrentMatchID() == 0)
	{
		return false;
	}
	uint matchID = player->GetCurrentMatchID();

	pthread_rwlock_rdlock(&matchListLock);
	if( matchList.count(matchID) == 0 )
	{
		pthread_rwlock_unlock(&matchListLock);
		return false;
	}
	Match *foundMatch = matchList[matchID];
	pthread_rwlock_unlock(&matchListLock);

	foundOne = foundMatch->RemovePlayer( player->GetID() );
	if( !foundOne )
	{
		return false;
	}
	player->SetCurrentMatchID(0);

	//If this was the last player in the match
	if( foundMatch->GetCurrentPlayerCount() == 0 )
	{
		delete foundMatch;
		pthread_rwlock_wrlock(&matchListLock);
		matchList.erase(matchID);
		pthread_rwlock_unlock(&matchListLock);
		return true;
	}

	//*******************************
	// Send Client Notifications
	//*******************************
	MatchLobbyMessage notification(PLAYER_LEFT_MATCH_NOTIFICATION, DIRECTION_TO_CLIENT);
	notification.m_playerID = player->GetID();
	notification.m_newLeaderID = foundMatch->GetLeaderID();
	NotifyClients(foundMatch, &notification);
	return true;
}

//Player has quit the server, clean up any references to it
//	Deletes the player object
void RTT::QuitServer(Player *player)
{
	if( player == NULL )
	{
		return;
	}
	if( player->GetCurrentMatchID() != 0)
	{
		//Leave any matches currently in
		LeaveMatch(player);
	}

	int ID = player->GetID();
	//Remove from the list of current players
	pthread_rwlock_wrlock(&playerListLock);
	playerList[ID] = NULL;
	playerList.erase(ID);
	pthread_rwlock_unlock(&playerListLock);

	delete player;
}

//Prints usage tips
string RTT::Usage()
{
	string out;

	out += "Line of Fire Server Usage:\n";
	out += "\t RTT_Server [-p PORT]\n\n";
	out += "\t -p PORT == TCP Port number to listen for connections on.\n";
	return out;
}


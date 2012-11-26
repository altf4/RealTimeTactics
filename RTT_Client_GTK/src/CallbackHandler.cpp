//============================================================================
// Name        : CallbackHandler.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Handles incoming callback messages from game server
//============================================================================

#include "CallbackHandler.h"
#include "messaging/MessageManager.h"

#include <iostream>

using namespace std;
using namespace Gtk;
using namespace RTT;

CallbackHandler::CallbackHandler()
{
	m_thread = NULL;
}

CallbackHandler::~CallbackHandler()
{
	if(m_thread != NULL)
	{
		m_thread->join();
	}
}


bool CallbackHandler::Start()
{
	if(m_thread == NULL)
	{
		m_thread = Glib::Thread::create(sigc::mem_fun(*this, &CallbackHandler::CallbackThread), true);
		return true;
	}
	else
	{
		return false;
	}
}

void CallbackHandler::Stop()
{
	if(m_thread != NULL)
	{
		m_thread->join();
	}
	m_thread = NULL;
}

CallbackChange *CallbackHandler::PopCallbackChange()
{
	Glib::Mutex::Lock lock (m_queueMutex);
	if(m_changeQueue.empty())
	{
		cerr << "ERROR: Callback change queue underflow!" << endl;
		return new CallbackChange(CALLBACK_ERROR);
	}
	else
	{
		MainLobbyCallbackChange *temp = m_changeQueue.front();
		m_changeQueue.pop();
		return temp;
	}
}

void CallbackHandler::PushCallbackChange(MainLobbyCallbackChange *change)
{
	Glib::Mutex::Lock lock (m_queueMutex);
	m_changeQueue.push(change);
}

void CallbackHandler::CallbackThread()
{
	while(true)
	{
		CallbackChange *change = ProcessCallbackCommand();

		//Check to see that it's not a game callback, since the gtk client can't run in game state
		if(change->m_type == CHANGE_GAME)
		{
			cerr << "ERROR: Received game callback in Gtk client. Cannot run in game state" << endl;
			delete change;
			continue;
		}
		if(change->m_type == CALLBACK_CLOSED)
		{
			m_sig_callback_closed();
			delete change;
			return;
		}
		if(change->m_type == CALLBACK_ERROR)
		{
			m_sig_callback_error();
			delete change;
			continue;
		}

		MainLobbyCallbackChange *lobbyChange = (MainLobbyCallbackChange*)change;

		PushCallbackChange(lobbyChange);


		switch( lobbyChange->m_mainLobbyType )
		{
			case TEAM_CHANGE:
			{
				m_sig_team_change();
				break;
			}
			case COLOR_CHANGE:
			{
				m_sig_color_change();
				break;
			}
			case MAP_CHANGE:
			{
				m_sig_map_change();
				break;
			}
			case SPEED_CHANGE:
			{
				m_sig_speed_change();
				break;
			}
			case VICTORY_CHANGE:
			{
				m_sig_victory_cond_change();
				break;
			}
			case PLAYER_LEFT:
			{
				m_sig_player_left();
				break;
			}
			case KICKED:
			{
				m_sig_kicked();
				return;
			}
			case PLAYER_JOINED:
			{
				m_sig_player_joined();
				break;
			}
			case LEADER_CHANGE:
			{
				m_sig_leader_change();
				break;
			}
			case MATCH_STARTED:
			{
				m_sig_match_started();

				//The actual game!!!!
				if(system("RTT_Ogre_3D") < 0)
				{
					//Failure
				}

				break;
			}
			default:
			{
				m_sig_callback_error();
				cerr << "ERROR: Got a bad type from callback. Shouldn't get here\n";
				return;
			}
		}
	}
}

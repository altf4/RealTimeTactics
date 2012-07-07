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
//using namespace Gtk;
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

struct CallbackChange CallbackHandler::PopCallbackChange()
{
	Glib::Mutex::Lock lock (m_queueMutex);
	if(m_changeQueue.empty())
	{
		cerr << "ERROR: Callback change queue underflow!" << endl;
		struct CallbackChange temp;
		temp.m_type = CALLBACK_ERROR;
		return temp;
	}
	else
	{
		struct CallbackChange temp = m_changeQueue.front();
		m_changeQueue.pop();
		return temp;
	}
}

void CallbackHandler::PushCallbackChange(struct CallbackChange change)
{
	Glib::Mutex::Lock lock (m_queueMutex);
	m_changeQueue.push(change);
}

void CallbackHandler::CallbackThread()
{
	while(true)
	{
		struct CallbackChange change = ProcessCallbackCommand();
		PushCallbackChange(change);
		switch( change.m_type )
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
			case CALLBACK_CLOSED:
			{
				m_sig_callback_closed();
				return;
			}
			case CALLBACK_ERROR:
			{
				m_sig_callback_error();
				return;
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

//============================================================================
// Name        : AppStateManager.cpp
// Author      : Mark Petro
// Copyright   : 2011, GNU GPLv3
// Description : Built from the Advanced OGRE Framework tutorial found here:
//	http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Advanced+Ogre+Framework&structure=Tutorials
//============================================================================

#include "AppStateManager.h"
#include <OGRE/OgreWindowEventUtilities.h>
#include "EventQueue.h"

using namespace RTT;

AppStateManager::AppStateManager()
{
	m_isShutdown = false;
}

AppStateManager::~AppStateManager()
{
	state_info si;

	while(!m_activeStateStack.empty())
	{
		m_activeStateStack.back()->Exit();
		m_activeStateStack.pop_back();
	}

	while(!m_states.empty())
	{
		si = m_states.back();
		si.m_state->Destroy();
		m_states.pop_back();
	}
}

void AppStateManager::ManageAppState(Ogre::String stateName, AppState *state)
{
	try
	{
		state_info new_state_info;
		new_state_info.m_name = stateName;
		new_state_info.m_state = state;
		m_states.push_back(new_state_info);
	}
	catch(std::exception& e)
	{
		delete state;
		throw Ogre::Exception(Ogre::Exception::ERR_INTERNAL_ERROR,
				"Error while trying to manage a new AppState\n" + Ogre::String(e.what()),
				"AppStateManager.cpp (39)");
	}
}

AppState* AppStateManager::FindByName(Ogre::String stateName)
{
	std::vector<state_info>::iterator itr;

	for(itr=m_states.begin(); itr!=m_states.end(); itr++)
	{
		if(itr->m_name == stateName)
			return itr->m_state;
	}

	return 0;
}

void AppStateManager::Start(AppState *state)
{
	ChangeAppState(state);

	int timeSinceLastFrame = 1;
	int startTime = 0;

	while(!m_isShutdown)
	{
		if(OgreFramework::getSingletonPtr()->m_renderWnd->isClosed())
		{
			m_isShutdown = true;
		}

		Ogre::WindowEventUtilities::messagePump();

		if(OgreFramework::getSingletonPtr()->m_renderWnd->isActive())
		{
			startTime = OgreFramework::getSingletonPtr()->m_timer->getMillisecondsCPU();

			OgreFramework::getSingletonPtr()->m_keyboard->capture();
			OgreFramework::getSingletonPtr()->m_mouse->capture();

			m_activeStateStack.back()->Update(timeSinceLastFrame);

			OgreFramework::getSingletonPtr()->UpdateOgre(timeSinceLastFrame);
			OgreFramework::getSingletonPtr()->m_root->renderOneFrame();

			timeSinceLastFrame = OgreFramework::getSingletonPtr()->
					m_timer->getMillisecondsCPU() - startTime;

			//Process all events we've seen this frame
			while(!EventQueue::Instance().Empty())
			{
				struct ServerEvent event = EventQueue::Instance().Dequeue();
				m_activeStateStack.back()->ProcessCallback(event);
			}
		}
		else
		{
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
            Sleep(1000);
#else
            sleep(1);
#endif
		}
	}

	OgreFramework::getSingletonPtr()->m_log->logMessage("Main loop quit");
}

void AppStateManager::ChangeAppState(AppState* state)
{
	if(!m_activeStateStack.empty())
	{
		m_activeStateStack.back()->Exit();
		m_activeStateStack.pop_back();
	}

	m_activeStateStack.push_back(state);
	init(state);
	m_activeStateStack.back()->Enter();
}

bool AppStateManager::PushAppState(AppState* state)
{
	if(!m_activeStateStack.empty())
	{
		if(!m_activeStateStack.back()->Pause())
			return false;
	}

	m_activeStateStack.push_back(state);
	init(state);
	m_activeStateStack.back()->Enter();

	return true;
}

void AppStateManager::PopAppState()
{
	if(!m_activeStateStack.empty())
	{
		m_activeStateStack.back()->Exit();
		m_activeStateStack.pop_back();
	}

	if(!m_activeStateStack.empty())
	{
		init(m_activeStateStack.back());
		m_activeStateStack.back()->Resume();
	}
    else
		Shutdown();
}

void AppStateManager::PopAllAndPushAppState(AppState* state)
{
    while(!m_activeStateStack.empty())
    {
        m_activeStateStack.back()->Exit();
        m_activeStateStack.pop_back();
    }

    PushAppState(state);
}

void AppStateManager::PauseAppState()
{
	if(!m_activeStateStack.empty())
	{
		m_activeStateStack.back()->Pause();
	}

	if(m_activeStateStack.size() > 2)
	{
		init(m_activeStateStack.at(m_activeStateStack.size() - 2));
		m_activeStateStack.at(m_activeStateStack.size() - 2)->Resume();
	}
}

void AppStateManager::Shutdown()
{
	m_isShutdown = true;
}

void AppStateManager::init(AppState* state)
{
	OgreFramework::getSingletonPtr()->m_keyboard->setEventCallback(state);
	OgreFramework::getSingletonPtr()->m_mouse->setEventCallback(state);
	//OgreFramework::getSingletonPtr()->m_pTrayMgr->setListener(state);

	OgreFramework::getSingletonPtr()->m_renderWnd->resetStatistics();
}

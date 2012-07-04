//============================================================================
// Name        : AppStateManager.cpp
// Author      : Mark Petro
// Copyright   : 2011, GNU GPLv3
// Description : Built from the Advanced OGRE Framework tutorial found here:
//	http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Advanced+Ogre+Framework&structure=Tutorials
//============================================================================

#include "AppStateManager.h"

#include <OgreWindowEventUtilities.h>

AppStateManager::AppStateManager()
{
	m_bShutdown = false;
}

AppStateManager::~AppStateManager()
{
	state_info si;

	while(!m_ActiveStateStack.empty())
	{
		m_ActiveStateStack.back()->exit();
		m_ActiveStateStack.pop_back();
	}

	while(!m_States.empty())
	{
		si = m_States.back();
		si.m_state->destroy();
		m_States.pop_back();
	}
}

void AppStateManager::manageAppState(Ogre::String stateName, AppState *state)
{
	try
	{
		state_info new_state_info;
		new_state_info.m_name = stateName;
		new_state_info.m_state = state;
		m_States.push_back(new_state_info);
	}
	catch(std::exception& e)
	{
		delete state;
		throw Ogre::Exception(Ogre::Exception::ERR_INTERNAL_ERROR,
				"Error while trying to manage a new AppState\n" + Ogre::String(e.what()),
				"AppStateManager.cpp (39)");
	}
}

AppState* AppStateManager::findByName(Ogre::String stateName)
{
	std::vector<state_info>::iterator itr;

	for(itr=m_States.begin(); itr!=m_States.end(); itr++)
	{
		if(itr->m_name == stateName)
			return itr->m_state;
	}

	return 0;
}

void AppStateManager::start(AppState *state)
{
	changeAppState(state);

	int timeSinceLastFrame = 1;
	int startTime = 0;

	while(!m_bShutdown)
	{
		if(OgreFramework::getSingletonPtr()->m_pRenderWnd->isClosed())m_bShutdown = true;

		Ogre::WindowEventUtilities::messagePump();

		if(OgreFramework::getSingletonPtr()->m_pRenderWnd->isActive())
		{
			startTime = OgreFramework::getSingletonPtr()->m_pTimer->getMillisecondsCPU();

			OgreFramework::getSingletonPtr()->m_pKeyboard->capture();
			OgreFramework::getSingletonPtr()->m_pMouse->capture();

			m_ActiveStateStack.back()->update(timeSinceLastFrame);

			OgreFramework::getSingletonPtr()->updateOgre(timeSinceLastFrame);
			OgreFramework::getSingletonPtr()->m_pRoot->renderOneFrame();

			timeSinceLastFrame = OgreFramework::getSingletonPtr()->m_pTimer->getMillisecondsCPU() - startTime;
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

	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Main loop quit");
}

void AppStateManager::changeAppState(AppState* state)
{
	if(!m_ActiveStateStack.empty())
	{
		m_ActiveStateStack.back()->exit();
		m_ActiveStateStack.pop_back();
	}

	m_ActiveStateStack.push_back(state);
	init(state);
	m_ActiveStateStack.back()->enter();
}

bool AppStateManager::pushAppState(AppState* state)
{
	if(!m_ActiveStateStack.empty())
	{
		if(!m_ActiveStateStack.back()->pause())
			return false;
	}

	m_ActiveStateStack.push_back(state);
	init(state);
	m_ActiveStateStack.back()->enter();

	return true;
}

void AppStateManager::popAppState()
{
	if(!m_ActiveStateStack.empty())
	{
		m_ActiveStateStack.back()->exit();
		m_ActiveStateStack.pop_back();
	}

	if(!m_ActiveStateStack.empty())
	{
		init(m_ActiveStateStack.back());
		m_ActiveStateStack.back()->resume();
	}
    else
		shutdown();
}

void AppStateManager::popAllAndPushAppState(AppState* state)
{
    while(!m_ActiveStateStack.empty())
    {
        m_ActiveStateStack.back()->exit();
        m_ActiveStateStack.pop_back();
    }

    pushAppState(state);
}

void AppStateManager::pauseAppState()
{
	if(!m_ActiveStateStack.empty())
	{
		m_ActiveStateStack.back()->pause();
	}

	if(m_ActiveStateStack.size() > 2)
	{
		init(m_ActiveStateStack.at(m_ActiveStateStack.size() - 2));
		m_ActiveStateStack.at(m_ActiveStateStack.size() - 2)->resume();
	}
}

void AppStateManager::shutdown()
{
	m_bShutdown = true;
}

void AppStateManager::init(AppState* state)
{
	OgreFramework::getSingletonPtr()->m_pKeyboard->setEventCallback(state);
	OgreFramework::getSingletonPtr()->m_pMouse->setEventCallback(state);
	//OgreFramework::getSingletonPtr()->m_pTrayMgr->setListener(state);

	OgreFramework::getSingletonPtr()->m_pRenderWnd->resetStatistics();
}

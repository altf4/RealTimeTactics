//============================================================================
// Name        : DemoApp.cpp
// Author      : Mark Petro
// Copyright   : 2011, GNU GPLv3
// Description : Built from the Advanced OGRE Framework tutorial found here:
//	http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Advanced+Ogre+Framework&structure=Tutorials
//============================================================================

#include "DemoApp.h"
#include "MenuState.h"
#include "GameState.h"
//#include "PauseState.h"

DemoApp::DemoApp()
{
	m_pAppStateManager = NULL;
}

DemoApp::~DemoApp()
{
	delete m_pAppStateManager;
	delete OgreFramework::getSingletonPtr();
}

void DemoApp::startDemo()
{
	new OgreFramework();
	if(!OgreFramework::getSingletonPtr()->initOgre("Real Time Tactics Render Window!", NULL, NULL))
	{
		return;
	}

	OgreFramework::getSingletonPtr()->m_pLog->logMessage("RTT Client OGRE initialized!");

	m_pAppStateManager = new AppStateManager();

	MenuState::create(m_pAppStateManager, "MenuState");
	GameState::create(m_pAppStateManager, "GameState");
	//PauseState::create(m_pAppStateManager, "PauseState");

	m_pAppStateManager->start(m_pAppStateManager->findByName("MenuState"));
}

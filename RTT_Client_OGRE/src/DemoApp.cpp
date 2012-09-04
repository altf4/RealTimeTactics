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

DemoApp::DemoApp()
{
	m_appStateManager = NULL;
}

DemoApp::~DemoApp()
{
	delete m_appStateManager;
	delete OgreFramework::getSingletonPtr();
}

void DemoApp::StartDemo()
{
	new OgreFramework();
	if(!OgreFramework::getSingletonPtr()->InitOgre("Real Time Tactics Render Window!", NULL, NULL))
	{
		return;
	}

	OgreFramework::getSingletonPtr()->m_log->logMessage("RTT Client OGRE initialized!");

	m_appStateManager = new AppStateManager();

	MenuState::create(m_appStateManager, "MenuState");
	GameState::create(m_appStateManager, "GameState");

	m_appStateManager->Start(m_appStateManager->FindByName("MenuState"));
}

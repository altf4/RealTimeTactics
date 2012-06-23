/*
 * DemoApp.cpp
 *
 *  Created on: Jun 22, 2012
 *      Author: mark
 */




//|||||||||||||||||||||||||||||||||||||||||||||||

#include "DemoApp.hpp"

#include "MenuState.hpp"
#include "GameState.hpp"
#include "PauseState.hpp"

//|||||||||||||||||||||||||||||||||||||||||||||||

DemoApp::DemoApp()
{
	m_pAppStateManager = 0;
}

//|||||||||||||||||||||||||||||||||||||||||||||||

DemoApp::~DemoApp()
{
	delete m_pAppStateManager;
        delete OgreFramework::getSingletonPtr();
}

//|||||||||||||||||||||||||||||||||||||||||||||||

void DemoApp::startDemo()
{
	new OgreFramework();
	if(!OgreFramework::getSingletonPtr()->initOgre("Real Time Tactics Render Window!", 0, 0))
		return;

	OgreFramework::getSingletonPtr()->m_pLog->logMessage("RTT Client OGRE initialized!");

	m_pAppStateManager = new AppStateManager();

	MenuState::create(m_pAppStateManager, "MenuState");
	GameState::create(m_pAppStateManager, "GameState");
        PauseState::create(m_pAppStateManager, "PauseState");

	m_pAppStateManager->start(m_pAppStateManager->findByName("MenuState"));
}

//|||||||||||||||||||||||||||||||||||||||||||||||

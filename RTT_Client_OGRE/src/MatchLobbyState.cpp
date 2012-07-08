//============================================================================
// Name        : MatchLobbyState.cpp
// Author      : Mark Petro
// Copyright   : 2011, GNU GPLv3
// Description : An OGRE based 3D frontend to the RTT project.
//	Built from the Advanced OGRE Framework tutorial found here:
//	http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Advanced+Ogre+Framework&structure=Tutorials
//============================================================================

#include "MatchLobbyState.h"

using namespace Ogre;

MatchLobbyState::MatchLobbyState()
{
    m_bQuit = false;
    m_FrameEvent = Ogre::FrameEvent();
}

void MatchLobbyState::enter()
{
	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Entering MatchLobbyState...");

	m_pSceneMgr = OgreFramework::getSingletonPtr()->m_pRoot->createSceneManager(
			ST_GENERIC, "MenuSceneMgr");
	m_pSceneMgr->setAmbientLight(Ogre::ColourValue(0.7f, 0.7f, 0.7f));

	m_pCamera = m_pSceneMgr->createCamera("MenuCam");
	m_pCamera->setPosition(Vector3(0, 25, -50));
	m_pCamera->lookAt(Vector3(0, 0, 0));
	m_pCamera->setNearClipDistance(1);

	m_pCamera->setAspectRatio(
		Real(OgreFramework::getSingletonPtr()->m_pViewport->getActualWidth()) /
		Real(OgreFramework::getSingletonPtr()->m_pViewport->getActualHeight()));

	OgreFramework::getSingletonPtr()->m_pViewport->setCamera(m_pCamera);

	OgreFramework::getSingletonPtr()->m_pKeyboard->setEventCallback(this);
	OgreFramework::getSingletonPtr()->m_pMouse->setEventCallback(this);

	CEGUI::Window *pMainWnd = CEGUI::WindowManager::getSingleton().getWindow("RTT_MatchLobby_GUI");
	OgreFramework::getSingletonPtr()->m_pGUISystem->setGUISheet(pMainWnd);

	CEGUI::PushButton *button = (CEGUI::PushButton*)pMainWnd->getChild("MatchExitButton");
	button->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MatchLobbyState::MatchExitButton, this));
	button = (CEGUI::PushButton*)pMainWnd->getChild("MatchBackButton");
	button->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&MatchLobbyState::MatchBackButton, this));


	m_bQuit = false;

	createScene();
}

void MatchLobbyState::createScene()
{
}

bool MatchLobbyState::pause()
{
	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Pausing MatchLobbyState...");

	OgreFramework::getSingletonPtr()->m_pGUISystem->setGUISheet(0);

	return true;
}

void MatchLobbyState::resume()
{
	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Resuming MatchLobbyState...");

	OgreFramework::getSingletonPtr()->m_pViewport->setCamera(m_pCamera);

	OgreFramework::getSingletonPtr()->m_pGUISystem->setGUISheet(CEGUI::WindowManager::getSingleton().getWindow("AOF_GUI"));

	m_bQuit = false;
}

void MatchLobbyState::exit()
{
	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Leaving MatchLobbyState...");

	OgreFramework::getSingletonPtr()->m_pGUISystem->setGUISheet(0);
	m_pSceneMgr->destroyCamera(m_pCamera);
	if(m_pSceneMgr)
	{
		OgreFramework::getSingletonPtr()->m_pRoot->destroySceneManager(m_pSceneMgr);
	}
}

bool MatchLobbyState::keyPressed(const OIS::KeyEvent &keyEventRef)
{
	if(OgreFramework::getSingletonPtr()->m_pKeyboard->isKeyDown(OIS::KC_ESCAPE))
	{
		m_bQuit = true;
		return true;
	}

	OgreFramework::getSingletonPtr()->keyPressed(keyEventRef);
	return true;
}

bool MatchLobbyState::keyReleased(const OIS::KeyEvent &keyEventRef)
{
	OgreFramework::getSingletonPtr()->keyReleased(keyEventRef);
	return true;
}

bool MatchLobbyState::mouseMoved(const OIS::MouseEvent &evt)
{
	OgreFramework::getSingletonPtr()->m_pGUISystem->injectMouseWheelChange(evt.state.Z.rel);
	OgreFramework::getSingletonPtr()->m_pGUISystem->injectMouseMove(evt.state.X.rel, evt.state.Y.rel);
	return true;
}

bool MatchLobbyState::mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{
	if(id == OIS::MB_Left)
	{
		OgreFramework::getSingletonPtr()->m_pGUISystem->injectMouseButtonDown(CEGUI::LeftButton);
	}
	return true;
}

bool MatchLobbyState::mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{
	if(id == OIS::MB_Left)
		OgreFramework::getSingletonPtr()->m_pGUISystem->injectMouseButtonUp(CEGUI::LeftButton);
	return true;
}

void MatchLobbyState::update(double timeSinceLastFrame)
{
	m_FrameEvent.timeSinceLastFrame = timeSinceLastFrame;

	if(m_bQuit == true)
	{
		shutdown();
		return;
	}
}

bool MatchLobbyState::MatchExitButton(const CEGUI::EventArgs &args)
{
	m_bQuit = true;
	return true;
}

bool MatchLobbyState::MatchBackButton(const CEGUI::EventArgs &args)
{
	changeAppState(findByName("LobbyState"));
	return true;
}

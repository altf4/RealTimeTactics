//============================================================================
// Name        : LobbyState.cpp
// Author      : Mark Petro
// Copyright   : 2011, GNU GPLv3
// Description : An OGRE based 3D frontend to the RTT project.
//	Built from the Advanced OGRE Framework tutorial found here:
//	http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Advanced+Ogre+Framework&structure=Tutorials
//============================================================================

#include "LobbyState.h"

using namespace Ogre;

LobbyState::LobbyState()
{
    m_bQuit = false;
    m_FrameEvent = Ogre::FrameEvent();
}

void LobbyState::enter()
{
	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Entering LobbyState...");

	m_pSceneMgr = OgreFramework::getSingletonPtr()->m_pRoot->createSceneManager(
			ST_GENERIC, "LobbySceneMgr");
	m_pSceneMgr->setAmbientLight(Ogre::ColourValue(0.7f, 0.7f, 0.7f));

	m_pCamera = m_pSceneMgr->createCamera("LobbyCam");
	m_pCamera->setPosition(Vector3(0, 25, -50));
	m_pCamera->lookAt(Vector3(0, 0, 0));
	m_pCamera->setNearClipDistance(1);

	m_pCamera->setAspectRatio(
		Real(OgreFramework::getSingletonPtr()->m_pViewport->getActualWidth()) /
		Real(OgreFramework::getSingletonPtr()->m_pViewport->getActualHeight()));

	OgreFramework::getSingletonPtr()->m_pViewport->setCamera(m_pCamera);

	OgreFramework::getSingletonPtr()->m_pKeyboard->setEventCallback(this);
	OgreFramework::getSingletonPtr()->m_pMouse->setEventCallback(this);

	OgreFramework::getSingletonPtr()->m_pGUISystem->setDefaultMouseCursor((CEGUI::utf8*)"OgreTrayImages", (CEGUI::utf8*)"MouseArrow");
	CEGUI::MouseCursor::getSingleton().setImage("OgreTrayImages", "MouseArrow");
	const OIS::MouseState state = OgreFramework::getSingletonPtr()->m_pMouse->getMouseState();
	CEGUI::Point mousePos = CEGUI::MouseCursor::getSingleton().getPosition();
	CEGUI::System::getSingleton().injectMouseMove(state.X.abs-mousePos.d_x,state.Y.abs-mousePos.d_y);

	CEGUI::Window *pMainWnd = CEGUI::WindowManager::getSingleton().getWindow("RTT_ServerLobby_GUI");
	OgreFramework::getSingletonPtr()->m_pGUISystem->setGUISheet(pMainWnd);

	CEGUI::PushButton *button = (CEGUI::PushButton*)pMainWnd->getChild("ServerExitButton");
	button->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&LobbyState::onExitButton, this));
	button = (CEGUI::PushButton*)pMainWnd->getChild("ServerBackButton");
	button->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&LobbyState::onBackButton, this));

	m_bQuit = false;

	createScene();
}

void LobbyState::createScene()
{
}

bool LobbyState::pause()
{
	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Pausing LobbyState...");
	OgreFramework::getSingletonPtr()->m_pGUISystem->setGUISheet(0);
	return true;
}

void LobbyState::resume()
{
	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Resuming LobbyState...");
	OgreFramework::getSingletonPtr()->m_pViewport->setCamera(m_pCamera);
	OgreFramework::getSingletonPtr()->m_pGUISystem->setGUISheet(CEGUI::WindowManager::getSingleton().getWindow("AOF_GUI"));
	m_bQuit = false;
}

void LobbyState::exit()
{
	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Leaving LobbyState...");

	OgreFramework::getSingletonPtr()->m_pGUISystem->setGUISheet(0);
	m_pSceneMgr->destroyCamera(m_pCamera);
	if(m_pSceneMgr)
	{
		OgreFramework::getSingletonPtr()->m_pRoot->destroySceneManager(m_pSceneMgr);
	}
}

bool LobbyState::keyPressed(const OIS::KeyEvent &keyEventRef)
{
	if(OgreFramework::getSingletonPtr()->m_pKeyboard->isKeyDown(OIS::KC_ESCAPE))
	{
		m_bQuit = true;
		return true;
	}

	OgreFramework::getSingletonPtr()->keyPressed(keyEventRef);
	return true;
}

bool LobbyState::keyReleased(const OIS::KeyEvent &keyEventRef)
{
	OgreFramework::getSingletonPtr()->keyReleased(keyEventRef);
	return true;
}

bool LobbyState::mouseMoved(const OIS::MouseEvent &evt)
{
	OgreFramework::getSingletonPtr()->m_pGUISystem->injectMouseWheelChange(evt.state.Z.rel);
	OgreFramework::getSingletonPtr()->m_pGUISystem->injectMouseMove(evt.state.X.rel, evt.state.Y.rel);

	return true;
}

bool LobbyState::mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{
	if(id == OIS::MB_Left)
	{
		OgreFramework::getSingletonPtr()->m_pGUISystem->injectMouseButtonDown(CEGUI::LeftButton);
	}
	return true;
}

bool LobbyState::mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{
	if(id == OIS::MB_Left)
	{
		OgreFramework::getSingletonPtr()->m_pGUISystem->injectMouseButtonUp(CEGUI::LeftButton);
	}
	return true;
}

void LobbyState::update(double timeSinceLastFrame)
{
	m_FrameEvent.timeSinceLastFrame = timeSinceLastFrame;

	if(m_bQuit == true)
	{
		shutdown();
		return;
	}
}


bool LobbyState::onExitButton(const CEGUI::EventArgs &args)
{
	m_bQuit = true;
	return true;
}

bool LobbyState::onBackButton(const CEGUI::EventArgs &args)
{
	changeAppState(findByName("JoinCustomServerState"));
	return true;
}


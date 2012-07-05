//============================================================================
// Name        : JoinCustomServerStateState.cpp
// Author      : Mark Petro
// Copyright   : 2011, GNU GPLv3
// Description : An OGRE based 3D frontend to the RTT project.
//	Built from the Advanced OGRE Framework tutorial found here:
//	http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Advanced+Ogre+Framework&structure=Tutorials
//============================================================================

#include "JoinCustomServerState.h"

using namespace Ogre;

JoinCustomServerState::JoinCustomServerState()
{
    m_bQuit = false;
    m_FrameEvent = Ogre::FrameEvent();
}

void JoinCustomServerState::enter()
{
	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Entering JoinCustomServerState...");

	m_pSceneMgr = OgreFramework::getSingletonPtr()->m_pRoot->createSceneManager(
			ST_GENERIC, "JCSSceneMgr");
	m_pSceneMgr->setAmbientLight(Ogre::ColourValue(0.7f, 0.7f, 0.7f));

	m_pCamera = m_pSceneMgr->createCamera("JCSCam");
	m_pCamera->setPosition(Vector3(0, 25, -50));
	m_pCamera->lookAt(Vector3(0, 0, 0));
	m_pCamera->setNearClipDistance(1);

	m_pCamera->setAspectRatio(
		Real(OgreFramework::getSingletonPtr()->m_pViewport->getActualWidth()) /
		Real(OgreFramework::getSingletonPtr()->m_pViewport->getActualHeight()));

	OgreFramework::getSingletonPtr()->m_pViewport->setCamera(m_pCamera);

/*
	OgreFramework::getSingletonPtr()->m_pTrayMgr->destroyAllWidgets();
	OgreFramework::getSingletonPtr()->m_pTrayMgr->showFrameStats(OgreBites::TL_BOTTOMLEFT);
	OgreFramework::getSingletonPtr()->m_pTrayMgr->showLogo(OgreBites::TL_BOTTOMRIGHT);
	OgreFramework::getSingletonPtr()->m_pTrayMgr->showCursor();
	OgreFramework::getSingletonPtr()->m_pTrayMgr->createButton(
			OgreBites::TL_CENTER, "EnterBtn", "Enter Match", 250);
	OgreFramework::getSingletonPtr()->m_pTrayMgr->createButton(
			OgreBites::TL_CENTER, "ExitBtn", "Exit Real Time Tactics", 250);
	OgreFramework::getSingletonPtr()->m_pTrayMgr->createLabel(
			OgreBites::TL_TOP, "MenuLbl", "Menu mode", 250);
*/

	OgreFramework::getSingletonPtr()->m_pKeyboard->setEventCallback(this);
	OgreFramework::getSingletonPtr()->m_pMouse->setEventCallback(this);

	//OgreFramework::getSingletonPtr()->m_pGUIRenderer->setTargetSceneManager(m_pSceneMgr);

	OgreFramework::getSingletonPtr()->m_pGUISystem->setDefaultMouseCursor((CEGUI::utf8*)"TaharezLook", (CEGUI::utf8*)"MouseArrow");
	CEGUI::MouseCursor::getSingleton().setImage("TaharezLook", "MouseArrow");
	const OIS::MouseState state = OgreFramework::getSingletonPtr()->m_pMouse->getMouseState();
	CEGUI::Point mousePos = CEGUI::MouseCursor::getSingleton().getPosition();
	CEGUI::System::getSingleton().injectMouseMove(state.X.abs-mousePos.d_x,state.Y.abs-mousePos.d_y);

	CEGUI::Window *pMainWnd = CEGUI::WindowManager::getSingleton().getWindow("RTT_JoinCustomServer_GUI");
	OgreFramework::getSingletonPtr()->m_pGUISystem->setGUISheet(pMainWnd);

	CEGUI::PushButton *button = (CEGUI::PushButton*)pMainWnd->getChild("ExitButton_JCS");
	button->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&JoinCustomServerState::onExitButton, this));
	button = (CEGUI::PushButton*)pMainWnd->getChild("BackButton");
	button->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&JoinCustomServerState::onBackButton, this));
	button = (CEGUI::PushButton*)pMainWnd->getChild("JoinServerButton");
	button->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&JoinCustomServerState::onJoinServerButton, this));

	m_bQuit = false;

	createScene();
}

void JoinCustomServerState::createScene()
{
}

bool JoinCustomServerState::pause()
{
	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Pausing JoinCustomServerState...");

	OgreFramework::getSingletonPtr()->m_pGUISystem->setGUISheet(0);
	//OgreFramework::getSingletonPtr()->m_pGUIRenderer->setTargetSceneManager(0);

	return true;
}

void JoinCustomServerState::resume()
{
	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Resuming JoinCustomServerState...");

	OgreFramework::getSingletonPtr()->m_pViewport->setCamera(m_pCamera);
	//OgreFramework::getSingletonPtr()->m_pGUIRenderer->setTargetSceneManager(m_pSceneMgr);

	OgreFramework::getSingletonPtr()->m_pGUISystem->setGUISheet(CEGUI::WindowManager::getSingleton().getWindow("AOF_GUI"));

	m_bQuit = false;
}

void JoinCustomServerState::exit()
{
	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Leaving JoinCustomServerState...");

	OgreFramework::getSingletonPtr()->m_pGUISystem->setGUISheet(0);
	m_pSceneMgr->destroyCamera(m_pCamera);
	if(m_pSceneMgr)
	{
		OgreFramework::getSingletonPtr()->m_pRoot->destroySceneManager(m_pSceneMgr);
	}

	//OgreFramework::getSingletonPtr()->m_pTrayMgr->clearAllTrays();
	//OgreFramework::getSingletonPtr()->m_pTrayMgr->destroyAllWidgets();
	//OgreFramework::getSingletonPtr()->m_pTrayMgr->setListener(0);
}

bool JoinCustomServerState::keyPressed(const OIS::KeyEvent &keyEventRef)
{
	if(OgreFramework::getSingletonPtr()->m_pKeyboard->isKeyDown(OIS::KC_ESCAPE))
	{
		m_bQuit = true;
		return true;
	}

	OgreFramework::getSingletonPtr()->keyPressed(keyEventRef);
	return true;
}

bool JoinCustomServerState::keyReleased(const OIS::KeyEvent &keyEventRef)
{
	OgreFramework::getSingletonPtr()->keyReleased(keyEventRef);
	return true;
}

bool JoinCustomServerState::mouseMoved(const OIS::MouseEvent &evt)
{
	OgreFramework::getSingletonPtr()->m_pGUISystem->injectMouseWheelChange(evt.state.Z.rel);
	OgreFramework::getSingletonPtr()->m_pGUISystem->injectMouseMove(evt.state.X.rel, evt.state.Y.rel);

	//if(OgreFramework::getSingletonPtr()->m_pTrayMgr->injectMouseMove(evt))
	//{
	//	return true;
	//}
	return true;
}

bool JoinCustomServerState::mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{
	if(id == OIS::MB_Left)
		OgreFramework::getSingletonPtr()->m_pGUISystem->injectMouseButtonDown(CEGUI::LeftButton);

	//if(OgreFramework::getSingletonPtr()->m_pTrayMgr->injectMouseDown(evt, id))
	//{
	//	return true;
	//}
	return true;
}

bool JoinCustomServerState::mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{
	if(id == OIS::MB_Left)
		OgreFramework::getSingletonPtr()->m_pGUISystem->injectMouseButtonUp(CEGUI::LeftButton);

	//if(OgreFramework::getSingletonPtr()->m_pTrayMgr->injectMouseUp(evt, id))
	//{
	//	return true;
	//}
	return true;
}

void JoinCustomServerState::update(double timeSinceLastFrame)
{
	m_FrameEvent.timeSinceLastFrame = timeSinceLastFrame;
	//OgreFramework::getSingletonPtr()->m_pTrayMgr->frameRenderingQueued(m_FrameEvent);

	if(m_bQuit == true)
	{
		shutdown();
		return;
	}
}
/*
void JoinCustomServerState::buttonHit(OgreBites::Button *button)
{
	if(button->getName() == "ExitBtn")
	{
		m_bQuit = true;
	}
	else if(button->getName() == "EnterBtn")
	{
		changeAppState(findByName("GameState"));
	}
}
*/

bool JoinCustomServerState::onExitButton(const CEGUI::EventArgs &args)
{
	m_bQuit = true;
	return true;
}

bool JoinCustomServerState::onBackButton(const CEGUI::EventArgs &args)
{
	changeAppState(findByName("MenuState"));
	return true;
}

bool JoinCustomServerState::onJoinServerButton(const CEGUI::EventArgs &args)
{
	//changeAppState(findByName("JoinCustomServerStateState"));
	return true;
}

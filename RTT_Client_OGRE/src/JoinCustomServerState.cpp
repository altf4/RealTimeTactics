//============================================================================
// Name        : JoinCustomServerStateState.cpp
// Author      : Mark Petro
// Copyright   : 2011, GNU GPLv3
// Description : An OGRE based 3D frontend to the RTT project.
//	Built from the Advanced OGRE Framework tutorial found here:
//	http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Advanced+Ogre+Framework&structure=Tutorials
//============================================================================

#include "JoinCustomServerState.h"

#include "messaging/MessageManager.h"

using boost::posix_time::ptime;
using boost::posix_time::time_duration;
using boost::gregorian::date;

using namespace Ogre;

JoinCustomServerState::JoinCustomServerState()
{
    m_bQuit = false;
    m_FrameEvent = Ogre::FrameEvent();
    //m_pJCSWnd = NULL;
}

void JoinCustomServerState::enter()
{
    RTT::MessageManager::Initialize(RTT::DIRECTION_TO_SERVER);

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

	OgreFramework::getSingletonPtr()->m_pGUISystem->setDefaultMouseCursor((CEGUI::utf8*)"OgreTrayImages", (CEGUI::utf8*)"MouseArrow");
	CEGUI::MouseCursor::getSingleton().setImage("OgreTrayImages", "MouseArrow");
	const OIS::MouseState state = OgreFramework::getSingletonPtr()->m_pMouse->getMouseState();
	CEGUI::Point mousePos = CEGUI::MouseCursor::getSingleton().getPosition();
	CEGUI::System::getSingleton().injectMouseMove(state.X.abs-mousePos.d_x,state.Y.abs-mousePos.d_y);

	CEGUI::Window *m_pJCSWnd = CEGUI::WindowManager::getSingleton().getWindow("RTT_JoinCustomServer_GUI");
	OgreFramework::getSingletonPtr()->m_pGUISystem->setGUISheet(m_pJCSWnd);

	CEGUI::PushButton *button = (CEGUI::PushButton*)m_pJCSWnd->getChild("ExitButton_JCS");
	button->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&JoinCustomServerState::onExitButton, this));
	button = (CEGUI::PushButton*)m_pJCSWnd->getChild("BackButton");
	button->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&JoinCustomServerState::onBackButton, this));
	button = (CEGUI::PushButton*)m_pJCSWnd->getChild("JoinServerButton");
	button->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&JoinCustomServerState::onJoinServerButton, this));

	CEGUI::Editbox *pInputBox = (CEGUI::Editbox*)m_pJCSWnd->getChild("ServerAddressBox");
	pInputBox->subscribeEvent(CEGUI::Editbox::EventActivated, CEGUI::Event::Subscriber(&JoinCustomServerState::onAddressActivate, this));
	pInputBox->subscribeEvent(CEGUI::Editbox::EventDeactivated, CEGUI::Event::Subscriber(&JoinCustomServerState::onAddressDeactivate, this));
	pInputBox = (CEGUI::Editbox*)m_pJCSWnd->getChild("ServerPortBox");
	pInputBox->subscribeEvent(CEGUI::Editbox::EventActivated, CEGUI::Event::Subscriber(&JoinCustomServerState::onPortActivate, this));
	pInputBox->subscribeEvent(CEGUI::Editbox::EventDeactivated, CEGUI::Event::Subscriber(&JoinCustomServerState::onPortDeactivate, this));
	pInputBox = (CEGUI::Editbox*)m_pJCSWnd->getChild("UsernameBox");
	pInputBox->subscribeEvent(CEGUI::Editbox::EventActivated, CEGUI::Event::Subscriber(&JoinCustomServerState::onUsernameActivate, this));
	pInputBox->subscribeEvent(CEGUI::Editbox::EventDeactivated, CEGUI::Event::Subscriber(&JoinCustomServerState::onUsernameDeactivate, this));
	pInputBox = (CEGUI::Editbox*)m_pJCSWnd->getChild("PasswordBox");
	pInputBox->subscribeEvent(CEGUI::Editbox::EventActivated, CEGUI::Event::Subscriber(&JoinCustomServerState::onPasswordActivate, this));
	pInputBox->subscribeEvent(CEGUI::Editbox::EventDeactivated, CEGUI::Event::Subscriber(&JoinCustomServerState::onPasswordDeactivate, this));

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
	OgreFramework::getSingletonPtr()->m_pGUISystem->injectKeyDown(keyEventRef.key);
	OgreFramework::getSingletonPtr()->m_pGUISystem->injectChar(keyEventRef.text);



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
	CEGUI::Window *m_pJCSWnd = CEGUI::WindowManager::getSingleton().getWindow("RTT_JoinCustomServer_GUI");
	CEGUI::Editbox *StatusBox = (CEGUI::Editbox*)m_pJCSWnd->getChild("StatusBox");
	CEGUI::Editbox *AddressBox = (CEGUI::Editbox*)m_pJCSWnd->getChild("ServerAddressBox");
	CEGUI::Editbox *PortBox = (CEGUI::Editbox*)m_pJCSWnd->getChild("ServerPortBox");
	CEGUI::Editbox *UsernameBox = (CEGUI::Editbox*)m_pJCSWnd->getChild("UsernameBox");
	CEGUI::Editbox *PasswordBox = (CEGUI::Editbox*)m_pJCSWnd->getChild("PasswordBox");

	StatusBox->setText("Trying to connect...");
	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Trying to connect...");

	//A little bit of input validation here
	std::string serverIP = AddressBox->getText().c_str();

	struct sockaddr_in stSockAddr;
	int Res = inet_pton(AF_INET, serverIP.c_str(), &stSockAddr.sin_addr);
	if (Res == 0)
	{
		StatusBox->setText("Invalid IP address");
		OgreFramework::getSingletonPtr()->m_pLog->logMessage("Invalid IP Address");
		return true;
	}

	char *errString;
	uint serverPort = strtoul(PortBox->getText().c_str(), &errString, 10);
	if( *errString != '\0' || PortBox->getText().c_str() == '\0')
	{
		//Error occurred
		StatusBox->setText("Invalid port number");
		OgreFramework::getSingletonPtr()->m_pLog->logMessage("Invalid port number");
		return true;
	}

	std::string givenName = UsernameBox->getText().c_str();
	if(givenName == "Enter Username...")
	{
		StatusBox->setText("Invalid Username");
		OgreFramework::getSingletonPtr()->m_pLog->logMessage("Invalid Username");
		return true;
	}

	std::string hashedPassword = PasswordBox->getText().c_str();

	int SocketFD = AuthToServer(serverIP, serverPort,
			givenName, (unsigned char*)hashedPassword.c_str(), &m_playerDescription);

	if( SocketFD > 0 )
	{
		StatusBox->setText("Connection Successful!");
		OgreFramework::getSingletonPtr()->m_pLog->logMessage("Connection Successful!");
		//LaunchMainLobbyPane();
		changeAppState(findByName("GameState"));

		//Launch the Callback Thread
//		if(m_callbackHandler != NULL)
//		{
//			m_callbackHandler->Start();
//		}
	}
	else
	{
		StatusBox->setText("Failed to connect to server");
		OgreFramework::getSingletonPtr()->m_pLog->logMessage("Failed to connect to server");
	}
	//changeAppState(findByName("JoinCustomServerStateState"));
	return true;
}

bool JoinCustomServerState::onAddressActivate(const CEGUI::EventArgs &args)
{
	CEGUI::Window *m_pJCSWnd = CEGUI::WindowManager::getSingleton().getWindow("RTT_JoinCustomServer_GUI");
	CEGUI::Editbox *tServerAddressBox = (CEGUI::Editbox*)m_pJCSWnd->getChild("ServerAddressBox");
	if(tServerAddressBox->getText() == "127.0.0.1")
	{
		tServerAddressBox->setText("");
	}
	return true;
}
bool JoinCustomServerState::onAddressDeactivate(const CEGUI::EventArgs &args)
{
	CEGUI::Window *m_pJCSWnd = CEGUI::WindowManager::getSingleton().getWindow("RTT_JoinCustomServer_GUI");
	CEGUI::Editbox *tServerAddressBox = (CEGUI::Editbox*)m_pJCSWnd->getChild("ServerAddressBox");
	if(tServerAddressBox->getText() == "")
	{
		tServerAddressBox->setText("127.0.0.1");
	}
	return true;
}
bool JoinCustomServerState::onPortActivate(const CEGUI::EventArgs &args)
{
	CEGUI::Window *m_pJCSWnd = CEGUI::WindowManager::getSingleton().getWindow("RTT_JoinCustomServer_GUI");
	CEGUI::Editbox *pInputBox = (CEGUI::Editbox*)m_pJCSWnd->getChild("ServerPortBox");
	if(pInputBox->getText() == "23000")
	{
		pInputBox->setText("");
	}
	return true;
}
bool JoinCustomServerState::onPortDeactivate(const CEGUI::EventArgs &args)
{
	CEGUI::Window *m_pJCSWnd = CEGUI::WindowManager::getSingleton().getWindow("RTT_JoinCustomServer_GUI");
	CEGUI::Editbox *pInputBox = (CEGUI::Editbox*)m_pJCSWnd->getChild("ServerPortBox");
	if(pInputBox->getText() == "")
	{
		pInputBox->setText("23000");
	}
	return true;
}
bool JoinCustomServerState::onUsernameActivate(const CEGUI::EventArgs &args)
{
	CEGUI::Window *m_pJCSWnd = CEGUI::WindowManager::getSingleton().getWindow("RTT_JoinCustomServer_GUI");
	CEGUI::Editbox *pInputBox = (CEGUI::Editbox*)m_pJCSWnd->getChild("UsernameBox");
	if(pInputBox->getText() == "Enter Username...")
	{
		pInputBox->setText("");
	}
	return true;
}
bool JoinCustomServerState::onUsernameDeactivate(const CEGUI::EventArgs &args)
{
	CEGUI::Window *m_pJCSWnd = CEGUI::WindowManager::getSingleton().getWindow("RTT_JoinCustomServer_GUI");
	CEGUI::Editbox *pInputBox = (CEGUI::Editbox*)m_pJCSWnd->getChild("UsernameBox");
	if(pInputBox->getText() == "")
	{
		pInputBox->setText("Enter Username...");
	}
	return true;
}
bool JoinCustomServerState::onPasswordActivate(const CEGUI::EventArgs &args)
{
	CEGUI::Window *m_pJCSWnd = CEGUI::WindowManager::getSingleton().getWindow("RTT_JoinCustomServer_GUI");
	CEGUI::Editbox *pInputBox = (CEGUI::Editbox*)m_pJCSWnd->getChild("PasswordBox");
	if(pInputBox->getText() == "Enter Password...")
	{
		pInputBox->setText("");
	}
	return true;
}
bool JoinCustomServerState::onPasswordDeactivate(const CEGUI::EventArgs &args)
{
	CEGUI::Window *m_pJCSWnd = CEGUI::WindowManager::getSingleton().getWindow("RTT_JoinCustomServer_GUI");
	CEGUI::Editbox *pInputBox = (CEGUI::Editbox*)m_pJCSWnd->getChild("PasswordBox");
	if(pInputBox->getText() == "")
	{
		pInputBox->setText("Enter Password...");
	}
	return true;
}

//============================================================================
// Name        : MenuState.cpp
// Author      : Mark Petro
// Copyright   : 2011, GNU GPLv3
// Description : An OGRE based 3D frontend to the RTT project.
//	Built from the Advanced OGRE Framework tutorial found here:
//	http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Advanced+Ogre+Framework&structure=Tutorials
//============================================================================

#include "MenuState.h"

using namespace Ogre;
using namespace RTT;

MenuState::MenuState()
{
    m_bQuit = false;
    m_FrameEvent = Ogre::FrameEvent();
    RTT::MessageManager::Initialize(RTT::DIRECTION_TO_SERVER);
    OgreFramework::getSingletonPtr()->m_pLog->logMessage("Is there a callback thread?");
    if(OgreFramework::getSingletonPtr()->m_callbackHandler == NULL)
    {
    	OgreFramework::getSingletonPtr()->m_pLog->logMessage("No, make one");
    	OgreFramework::getSingletonPtr()->m_callbackHandler = new RTT::CallbackHandler();
    }
    else
    {
    	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Yes, use it");
    }

    RTT::MessageManager::Initialize(RTT::DIRECTION_TO_SERVER);
}

void MenuState::enter()
{
	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Entering MenuState...");

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



	m_bQuit = false;

	createScene();
}

void MenuState::createScene()
{
	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Initalizing GUI:");

	//Initialize:
	//Main Menu
	CEGUI::Window *pMainWnd =
			CEGUI::WindowManager::getSingleton().getWindow("RTT_MainMenu");
	OgreFramework::getSingletonPtr()->m_pGUISystem->setGUISheet(pMainWnd);
	CEGUI::PushButton *button = (CEGUI::PushButton*)pMainWnd->getChild("ExitButton");
	button->subscribeEvent(CEGUI::PushButton::EventClicked,
			CEGUI::Event::Subscriber(&MenuState::onExitButton, this));
	button = (CEGUI::PushButton*)pMainWnd->getChild("EnterButton");
	button->subscribeEvent(CEGUI::PushButton::EventClicked,
			CEGUI::Event::Subscriber(&MenuState::onEnterButton, this));
	button = (CEGUI::PushButton*)pMainWnd->getChild("JoinCustomServerButton");
	button->subscribeEvent(CEGUI::PushButton::EventClicked,
			CEGUI::Event::Subscriber(&MenuState::onCustomServerButton, this));
	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Main Menu");

	//Join Custom Server
	//pMainWnd = CEGUI::WindowManager::getSingleton().getWindow("RTT_JoinCustomServer");
	CEGUI::FrameWindow *pCustomServerWnd =
			(CEGUI::FrameWindow*)pMainWnd->getChild("RTT_CustomServerWindow");
	pCustomServerWnd->setVisible(false);
	button = (CEGUI::PushButton*)pCustomServerWnd->getChild("CustomServerCancelButton");
	button->subscribeEvent(CEGUI::PushButton::EventClicked,
			CEGUI::Event::Subscriber(&MenuState::onBackButton, this));
	button = (CEGUI::PushButton*)pCustomServerWnd->getChild("JoinServerButton");
	button->subscribeEvent(CEGUI::PushButton::EventClicked,
			CEGUI::Event::Subscriber(&MenuState::onJoinServerButton, this));
	CEGUI::Editbox *pInputBox =
			(CEGUI::Editbox*)pCustomServerWnd->getChild("ServerAddressBox");
	pInputBox->subscribeEvent(CEGUI::Editbox::EventActivated,
			CEGUI::Event::Subscriber(&MenuState::onAddressActivate, this));
	pInputBox->subscribeEvent(CEGUI::Editbox::EventDeactivated,
			CEGUI::Event::Subscriber(&MenuState::onAddressDeactivate, this));
	pInputBox = (CEGUI::Editbox*)pCustomServerWnd->getChild("ServerPortBox");
	pInputBox->subscribeEvent(CEGUI::Editbox::EventActivated,
			CEGUI::Event::Subscriber(&MenuState::onPortActivate, this));
	pInputBox->subscribeEvent(CEGUI::Editbox::EventDeactivated,
			CEGUI::Event::Subscriber(&MenuState::onPortDeactivate, this));
	pInputBox = (CEGUI::Editbox*)pCustomServerWnd->getChild("UsernameBox");
	pInputBox->subscribeEvent(CEGUI::Editbox::EventActivated,
			CEGUI::Event::Subscriber(&MenuState::onUsernameActivate, this));
	pInputBox->subscribeEvent(CEGUI::Editbox::EventDeactivated,
			CEGUI::Event::Subscriber(&MenuState::onUsernameDeactivate, this));
	pInputBox = (CEGUI::Editbox*)pCustomServerWnd->getChild("PasswordBox");
	pInputBox->subscribeEvent(CEGUI::Editbox::EventActivated,
			CEGUI::Event::Subscriber(&MenuState::onPasswordActivate, this));
	pInputBox->subscribeEvent(CEGUI::Editbox::EventDeactivated,
			CEGUI::Event::Subscriber(&MenuState::onPasswordDeactivate, this));
	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Custom Server Menu");

	//Match Lobby Menu
	pMainWnd = CEGUI::WindowManager::getSingleton().getWindow("RTT_MatchLobby");
	button = (CEGUI::PushButton*)pMainWnd->getChild("MatchExitButton");
	button->subscribeEvent(CEGUI::PushButton::EventClicked,
			CEGUI::Event::Subscriber(&MenuState::onExitButton, this));
	button = (CEGUI::PushButton*)pMainWnd->getChild("MatchBackButton");
	button->subscribeEvent(CEGUI::PushButton::EventClicked,
			CEGUI::Event::Subscriber(&MenuState::onBackButton, this));



	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Match Lobby");

	//Server Lobby Menu
	pMainWnd = CEGUI::WindowManager::getSingleton().getWindow("RTT_ServerLobby");
	button = (CEGUI::PushButton*)pMainWnd->getChild("ServerExitButton");
	button->subscribeEvent(CEGUI::PushButton::EventClicked,
			CEGUI::Event::Subscriber(&MenuState::onExitButton, this));
	button = (CEGUI::PushButton*)pMainWnd->getChild("ServerBackButton");
	button->subscribeEvent(CEGUI::PushButton::EventClicked,
			CEGUI::Event::Subscriber(&MenuState::onBackButton, this));
	button = (CEGUI::PushButton*)pMainWnd->getChild("RefreshListButton");
	button->subscribeEvent(CEGUI::PushButton::EventClicked,
			CEGUI::Event::Subscriber(&MenuState::listMatchesButton, this));
	button = (CEGUI::PushButton*)pMainWnd->getChild("JoinMatchButton");
	button->subscribeEvent(CEGUI::PushButton::EventClicked,
			CEGUI::Event::Subscriber(&MenuState::JoinMatchButton, this));
	button = (CEGUI::PushButton*)pMainWnd->getChild("CreateMatchButton");
	button->subscribeEvent(CEGUI::PushButton::EventClicked,
			CEGUI::Event::Subscriber(&MenuState::createMatchButton, this));
	multiColumnListMatch = (CEGUI::MultiColumnList*)pMainWnd->getChild("MatchesMCL");
	multiColumnListMatch->addColumn("ID", 0, CEGUI::UDim(0.1f, 0));
	multiColumnListMatch->addColumn("Players", 1, CEGUI::UDim(0.20f, 0));
	multiColumnListMatch->addColumn("Name", 2, CEGUI::UDim(0.20f, 0));
	multiColumnListMatch->addColumn("Map", 3, CEGUI::UDim(0.20f, 0));
	multiColumnListMatch->addColumn("Time Created", 4, CEGUI::UDim(0.27f, 0));
	multiColumnListMatch->setSelectionMode(CEGUI::MultiColumnList::RowSingle);
	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Server Lobby");

	//Create Match Menu
	CEGUI::FrameWindow *pCreateMatchWnd =
			(CEGUI::FrameWindow*)pMainWnd->getChild("RTT_CreateMatchWindow");
	pCreateMatchWnd->setVisible(false);
	button = (CEGUI::PushButton*)pCreateMatchWnd->getChild("CreateMatchCancelButton");
	button->subscribeEvent(CEGUI::PushButton::EventClicked,
			CEGUI::Event::Subscriber(&MenuState::onBackButton, this));
	button = (CEGUI::PushButton*)pCreateMatchWnd->getChild("CreateMatchSubmitButton");
	button->subscribeEvent(CEGUI::PushButton::EventClicked,
			CEGUI::Event::Subscriber(&MenuState::createMatchSubmitButton, this));
	pInputBox = (CEGUI::Editbox*)pCreateMatchWnd->getChild("MatchNameBox");
	pInputBox->subscribeEvent(CEGUI::Editbox::EventActivated,
			CEGUI::Event::Subscriber(&MenuState::onMatchNameActivate, this));
	pInputBox->subscribeEvent(CEGUI::Editbox::EventDeactivated,
			CEGUI::Event::Subscriber(&MenuState::onMatchNameDeactivate, this));
	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Create Match Menu");

	//Set GUI to Main Menu
	pMainWnd = CEGUI::WindowManager::getSingleton().getWindow("RTT_MainMenu");
	OgreFramework::getSingletonPtr()->m_pGUISystem->setGUISheet(pMainWnd);

	mLocation = MAINMENU;
	OgreFramework::getSingletonPtr()->m_pLog->logMessage(
			"GUI Initialized, entering Main Menu");

	return;
}

bool MenuState::pause()
{
	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Pausing MenuState...");

	OgreFramework::getSingletonPtr()->m_pGUISystem->setGUISheet(0);

	return true;
}

void MenuState::resume()
{
	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Resuming MenuState...");

	OgreFramework::getSingletonPtr()->m_pViewport->setCamera(m_pCamera);

	OgreFramework::getSingletonPtr()->m_pGUISystem->setGUISheet(
			CEGUI::WindowManager::getSingleton().getWindow("RTT_MainMenu"));

	m_bQuit = false;
}

void MenuState::exit()
{
	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Leaving MenuState...");

	OgreFramework::getSingletonPtr()->m_pGUISystem->setGUISheet(0);
	m_pSceneMgr->destroyCamera(m_pCamera);
	if(m_pSceneMgr)
	{
		OgreFramework::getSingletonPtr()->m_pRoot->destroySceneManager(m_pSceneMgr);
	}

}

bool MenuState::keyPressed(const OIS::KeyEvent &keyEventRef)
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

bool MenuState::keyReleased(const OIS::KeyEvent &keyEventRef)
{
	OgreFramework::getSingletonPtr()->keyReleased(keyEventRef);
	return true;
}

bool MenuState::mouseMoved(const OIS::MouseEvent &evt)
{
	OgreFramework::getSingletonPtr()->m_pGUISystem->injectMouseWheelChange(
			evt.state.Z.rel);
	OgreFramework::getSingletonPtr()->m_pGUISystem->injectMouseMove(
			evt.state.X.rel, evt.state.Y.rel);


	return true;
}

bool MenuState::mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{
	if(id == OIS::MB_Left)
	{
		OgreFramework::getSingletonPtr()->m_pGUISystem->injectMouseButtonDown(
				CEGUI::LeftButton);
	}
	return true;
}

bool MenuState::mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{
	if(id == OIS::MB_Left)
	{
		OgreFramework::getSingletonPtr()->m_pGUISystem->injectMouseButtonUp(
				CEGUI::LeftButton);
	}
	return true;
}

void MenuState::update(double timeSinceLastFrame)
{
	m_FrameEvent.timeSinceLastFrame = timeSinceLastFrame;

	if(m_bQuit == true)
	{
		shutdown();
		return;
	}
}

bool MenuState::onExitButton(const CEGUI::EventArgs &args)
{
	m_bQuit = true;
	return true;
}

bool MenuState::onEnterButton(const CEGUI::EventArgs &args)
{
	changeAppState(findByName("GameState"));
	return true;
}

bool MenuState::onBackButton(const CEGUI::EventArgs &args)
{
	switch(mLocation)
	{
		case MATCHLOBBY:
		{
			OgreFramework::getSingletonPtr()->m_pLog->logMessage("Leaving Match ID: "
					+ Ogre::StringConverter::toString(m_currentMatch.m_ID));
			if( RTT::LeaveMatch() )
			{
				m_currentMatch.m_ID = 0;
				serverLobby();
			}
			else
			{
				OgreFramework::getSingletonPtr()->m_pLog->logMessage(
						"Server error in leaving Match ID: " +
						Ogre::StringConverter::toString(m_currentMatch.m_ID));
			}
			break;
		}
		case SERVERLOBBY:
		{
			OgreFramework::getSingletonPtr()->m_pLog->logMessage("Leaving Server");
			RTT::ExitServer();
			CEGUI::Window *pMainWnd =
					CEGUI::WindowManager::getSingleton().getWindow("RTT_MainMenu");
			CEGUI::FrameWindow *pCustomServerWnd =
					(CEGUI::FrameWindow*)pMainWnd->getChild("RTT_CustomServerWindow");
			pCustomServerWnd->setVisible(false);
			mLocation = MAINMENU;
			OgreFramework::getSingletonPtr()->m_pGUISystem->setGUISheet(pMainWnd);
			break;
		}
		case JOINCUSTOMSERVER:
		{
			CEGUI::Window *pMainWnd =
					CEGUI::WindowManager::getSingleton().getWindow("RTT_MainMenu");
			CEGUI::FrameWindow *pCustomServerWnd =
					(CEGUI::FrameWindow*)pMainWnd->getChild("RTT_CustomServerWindow");
			pCustomServerWnd->setVisible(false);
			mLocation = MAINMENU;
			break;
		}
		case CREATEMATCH:
		{
			CEGUI::Window *pMainWnd =
					CEGUI::WindowManager::getSingleton().getWindow("RTT_ServerLobby");
			CEGUI::FrameWindow *pCreateMatchWnd =
					(CEGUI::FrameWindow*)pMainWnd->getChild("RTT_CreateMatchWindow");
			pCreateMatchWnd->setVisible(false);
			mLocation = SERVERLOBBY;
			break;
		}
		default:
		{
			break;
		}
	}

	return true;
}

bool MenuState::onCustomServerButton(const CEGUI::EventArgs &args)
{
	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Custom Server Menu");
	CEGUI::Window *pMainWnd =
			CEGUI::WindowManager::getSingleton().getWindow("RTT_MainMenu");
	CEGUI::FrameWindow *pCustomServerWnd =
			(CEGUI::FrameWindow*)pMainWnd->getChild("RTT_CustomServerWindow");
	pCustomServerWnd->setVisible(true);
	OgreFramework::getSingletonPtr()->m_pGUISystem->setGUISheet(pMainWnd);
	mLocation = JOINCUSTOMSERVER;
	return true;
}

bool MenuState::onJoinServerButton(const CEGUI::EventArgs &args)
{
	CEGUI::Window *pMainWnd =
			CEGUI::WindowManager::getSingleton().getWindow("RTT_MainMenu");
	CEGUI::FrameWindow *pCustomServerWnd =
			(CEGUI::FrameWindow*)pMainWnd->getChild("RTT_CustomServerWindow");
	pCustomServerWnd->setVisible(true);
	CEGUI::Editbox *AddressBox =
			(CEGUI::Editbox*)pCustomServerWnd->getChild("ServerAddressBox");
	CEGUI::Editbox *PortBox =
			(CEGUI::Editbox*)pCustomServerWnd->getChild("ServerPortBox");
	CEGUI::Editbox *UsernameBox =
			(CEGUI::Editbox*)pCustomServerWnd->getChild("UsernameBox");
	CEGUI::Editbox *PasswordBox =
			(CEGUI::Editbox*)pCustomServerWnd->getChild("PasswordBox");

	pCustomServerWnd->setText("Trying to connect...");
	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Trying to connect...");

	//A little bit of input validation here
	std::string serverIP = AddressBox->getText().c_str();

	struct sockaddr_in stSockAddr;
	int Res = inet_pton(AF_INET, serverIP.c_str(), &stSockAddr.sin_addr);
	if (Res == 0)
	{
		pCustomServerWnd->setText("Invalid IP address");
		OgreFramework::getSingletonPtr()->m_pLog->logMessage("Invalid IP Address");
		return true;
	}

	char *errString;
	uint serverPort = strtoul(PortBox->getText().c_str(), &errString, 10);
	if( *errString != '\0' || PortBox->getText().c_str() == '\0')
	{
		//Error occurred
		pCustomServerWnd->setText("Invalid port number");
		OgreFramework::getSingletonPtr()->m_pLog->logMessage("Invalid port number");
		return true;
	}

	std::string givenName = UsernameBox->getText().c_str();
	if(givenName == "Enter Username...")
	{
		pCustomServerWnd->setText("Invalid Username");
		OgreFramework::getSingletonPtr()->m_pLog->logMessage("Invalid Username");
		return true;
	}

	std::string hashedPassword = PasswordBox->getText().c_str();

	int SocketFD = AuthToServer(serverIP, serverPort,
			givenName, (unsigned char*)hashedPassword.c_str(), &m_playerDescription);

	if( SocketFD > 0 )
	{
		pCustomServerWnd->setText("Connection Successful!");
		OgreFramework::getSingletonPtr()->m_pLog->logMessage("Connection Successful!");

		//Launch the Callback Thread
		if(OgreFramework::getSingletonPtr()->m_callbackHandler != NULL)
		{
			OgreFramework::getSingletonPtr()->m_pLog->logMessage(
					"Starting Callback Thread");
			OgreFramework::getSingletonPtr()->m_callbackHandler->Start();
		}
		serverLobby();
	}
	else
	{
		pCustomServerWnd->setText("Failed to connect to server");
		OgreFramework::getSingletonPtr()->m_pLog->logMessage(
				"Failed to connect to server");
	}
	return true;
}

bool MenuState::onAddressActivate(const CEGUI::EventArgs &args)
{
	CEGUI::Window *pMainWnd =
			CEGUI::WindowManager::getSingleton().getWindow("RTT_MainMenu");
	CEGUI::FrameWindow *pCustomServerWnd =
			(CEGUI::FrameWindow*)pMainWnd->getChild("RTT_CustomServerWindow");
	CEGUI::Editbox *tServerAddressBox =
			(CEGUI::Editbox*)pCustomServerWnd->getChild("ServerAddressBox");
	if(tServerAddressBox->getText() == "127.0.0.1")
	{
		tServerAddressBox->setText("");
	}
	return true;
}
bool MenuState::onAddressDeactivate(const CEGUI::EventArgs &args)
{
	CEGUI::Window *pMainWnd =
			CEGUI::WindowManager::getSingleton().getWindow("RTT_MainMenu");
	CEGUI::FrameWindow *pCustomServerWnd =
			(CEGUI::FrameWindow*)pMainWnd->getChild("RTT_CustomServerWindow");
	CEGUI::Editbox *tServerAddressBox =
			(CEGUI::Editbox*)pCustomServerWnd->getChild("ServerAddressBox");
	if(tServerAddressBox->getText() == "")
	{
		tServerAddressBox->setText("127.0.0.1");
	}
	return true;
}
bool MenuState::onPortActivate(const CEGUI::EventArgs &args)
{
	CEGUI::Window *pMainWnd =
			CEGUI::WindowManager::getSingleton().getWindow("RTT_MainMenu");
	CEGUI::FrameWindow *pCustomServerWnd =
			(CEGUI::FrameWindow*)pMainWnd->getChild("RTT_CustomServerWindow");
	CEGUI::Editbox *pInputBox =
			(CEGUI::Editbox*)pCustomServerWnd->getChild("ServerPortBox");
	if(pInputBox->getText() == "23000")
	{
		pInputBox->setText("");
	}
	return true;
}
bool MenuState::onPortDeactivate(const CEGUI::EventArgs &args)
{
	CEGUI::Window *pMainWnd =
			CEGUI::WindowManager::getSingleton().getWindow("RTT_MainMenu");
	CEGUI::FrameWindow *pCustomServerWnd =
			(CEGUI::FrameWindow*)pMainWnd->getChild("RTT_CustomServerWindow");
	CEGUI::Editbox *pInputBox =
			(CEGUI::Editbox*)pCustomServerWnd->getChild("ServerPortBox");
	if(pInputBox->getText() == "")
	{
		pInputBox->setText("23000");
	}
	return true;
}
bool MenuState::onUsernameActivate(const CEGUI::EventArgs &args)
{
	CEGUI::Window *pMainWnd =
			CEGUI::WindowManager::getSingleton().getWindow("RTT_MainMenu");
	CEGUI::FrameWindow *pCustomServerWnd =
			(CEGUI::FrameWindow*)pMainWnd->getChild("RTT_CustomServerWindow");
	CEGUI::Editbox *pInputBox =
			(CEGUI::Editbox*)pCustomServerWnd->getChild("UsernameBox");
	if(pInputBox->getText() == "Enter Username...")
	{
		pInputBox->setText("");
	}
	return true;
}
bool MenuState::onUsernameDeactivate(const CEGUI::EventArgs &args)
{
	CEGUI::Window *pMainWnd =
			CEGUI::WindowManager::getSingleton().getWindow("RTT_MainMenu");
	CEGUI::FrameWindow *pCustomServerWnd =
			(CEGUI::FrameWindow*)pMainWnd->getChild("RTT_CustomServerWindow");
	CEGUI::Editbox *pInputBox =
			(CEGUI::Editbox*)pCustomServerWnd->getChild("UsernameBox");
	if(pInputBox->getText() == "")
	{
		pInputBox->setText("Enter Username...");
	}
	return true;
}
bool MenuState::onPasswordActivate(const CEGUI::EventArgs &args)
{
	CEGUI::Window *pMainWnd =
			CEGUI::WindowManager::getSingleton().getWindow("RTT_MainMenu");
	CEGUI::FrameWindow *pCustomServerWnd =
			(CEGUI::FrameWindow*)pMainWnd->getChild("RTT_CustomServerWindow");
	CEGUI::Editbox *pInputBox =
			(CEGUI::Editbox*)pCustomServerWnd->getChild("PasswordBox");
	if(pInputBox->getText() == "Enter Password...")
	{
		pInputBox->setText("");
	}
	return true;
}
bool MenuState::onPasswordDeactivate(const CEGUI::EventArgs &args)
{
	CEGUI::Window *pMainWnd =
			CEGUI::WindowManager::getSingleton().getWindow("RTT_MainMenu");
	CEGUI::FrameWindow *pCustomServerWnd =
			(CEGUI::FrameWindow*)pMainWnd->getChild("RTT_CustomServerWindow");
	CEGUI::Editbox *pInputBox =
			(CEGUI::Editbox*)pCustomServerWnd->getChild("PasswordBox");
	if(pInputBox->getText() == "")
	{
		pInputBox->setText("Enter Password...");
	}
	return true;
}
bool MenuState::JoinMatchButton(const CEGUI::EventArgs &args)
{
	//Get selected match
	CEGUI::ListboxItem *listboxItem = multiColumnListMatch->getFirstSelectedItem();

	//Test to see if a match was selected
	if(listboxItem == NULL)
	{
		OgreFramework::getSingletonPtr()->m_pLog->logMessage("No match selected");
		return true;
	}

	//Extract and cast matchID from selected row
	Ogre::String matchIDString = listboxItem->getText().c_str();
	int matchID = Ogre::StringConverter::parseInt(matchIDString);

	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Trying to join Match ID: " +
			Ogre::StringConverter::toString(matchID));

	//RTT::PlayerDescription playerDescriptions[MAX_PLAYERS_IN_MATCH];

	uint playerCount = JoinMatch(matchID,  m_otherPlayers, &m_currentMatch);

	if( playerCount > 0 )
	{
		m_currentMatch.m_ID = matchID;
		OgreFramework::getSingletonPtr()->m_pLog->logMessage("Joined match ID: " +
				Ogre::StringConverter::toString(matchID));
		matchLobby(playerCount);
	}
	else
	{
		m_currentMatch.m_ID = 0;
		OgreFramework::getSingletonPtr()->m_pLog->logMessage("Failed to join match");
	}

	return true;
}

void MenuState::matchLobby(uint playerCount)
{
	CEGUI::Window *pMainWnd =
			CEGUI::WindowManager::getSingleton().getWindow("RTT_MatchLobby");
	OgreFramework::getSingletonPtr()->m_pGUISystem->setGUISheet(pMainWnd);
	m_bInMatch = true;
	mLocation = MATCHLOBBY;
	listPlayers(playerCount);
	return;
}

void MenuState::listPlayers(uint playerCount)
{
	CEGUI::Window *pMainWnd =
			CEGUI::WindowManager::getSingleton().getWindow("RTT_MatchLobby");
	CEGUI::ScrollablePane *scrollpane =
			(CEGUI::ScrollablePane*)pMainWnd->getChild("PlayersPane");
	CEGUI::RadioButton *isLeader;
	CEGUI::DefaultWindow *playerName;
	CEGUI::UDim offSet;
	OgreFramework::getSingletonPtr()->m_pLog->logMessage(
			"Adding " + Ogre::StringConverter::toString((int)playerCount) + " Players");
	if(playerCount > 1)
	{
		for(uint i = 0; i < playerCount; i++)
		{
			OgreFramework::getSingletonPtr()->m_pLog->logMessage(
					"Player " + Ogre::StringConverter::toString(
					(int)m_otherPlayers[i].m_ID) + " added");

			//Check to see if window object names already exist, if so delete them
			if(CEGUI::WindowManager::getSingleton().isWindowPresent("IsLeader" +
					CEGUI::PropertyHelper::intToString((int)m_otherPlayers[i].m_ID)))
			{
				CEGUI::WindowManager::getSingleton().destroyWindow(
						"IsLeader" + CEGUI::PropertyHelper::intToString(
						(int)m_otherPlayers[i].m_ID));
				CEGUI::WindowManager::getSingleton().destroyWindow(
						m_otherPlayers[i].m_name);
			}

			isLeader = (CEGUI::RadioButton*)CEGUI::WindowManager::getSingleton().
					createWindow("OgreTray/RadioButton","IsLeader" +
					CEGUI::PropertyHelper::intToString((int)m_otherPlayers[i].m_ID));
			playerName = (CEGUI::DefaultWindow*)CEGUI::WindowManager::getSingleton().
					createWindow("OgreTray/StaticText",m_otherPlayers[i].m_name);

			isLeader->setGroupID(1);
			isLeader->setID((int)m_otherPlayers[i].m_ID);
			if(m_otherPlayers[i].m_ID == m_currentMatch.m_leaderID)
			{
				isLeader->setSelected(true);
			}
			if(m_playerDescription.m_ID == m_currentMatch.m_leaderID)
			{
				isLeader->setEnabled(true);
			}
			else
			{
				//isLeader->setEnabled(false);
			}

			playerName->setText(m_otherPlayers[i].m_name);

			offSet = offSet + CEGUI::UDim(0.1f, 0.0f);

			isLeader->setPosition(CEGUI::UVector2(
					CEGUI::UDim(0.0f, 0.0f),CEGUI::UDim( (i+1) * 0.1f , 0.0f)));
			isLeader->setSize(CEGUI::UVector2(
					CEGUI::UDim(0.075f, 0.0f), CEGUI::UDim(0.075f, 0.0f)));
			playerName->setPosition(CEGUI::UVector2(
					CEGUI::UDim(0.1f, 0.0f),CEGUI::UDim( (i+1) * 0.1f , 0.0f)));
			playerName->setSize(CEGUI::UVector2(
					CEGUI::UDim(0.2f, 0.0f), CEGUI::UDim(0.075f, 0.0f)));
			playerName->setProperty("FrameEnabled", "False");

			scrollpane->addChildWindow(isLeader);
			scrollpane->addChildWindow(playerName);

			isLeader->subscribeEvent(
					CEGUI::RadioButton::EventSelectStateChanged,
					CEGUI::Event::Subscriber(&MenuState::onLeaderClick, this));
		}
	}
	else
	{
		OgreFramework::getSingletonPtr()->m_pLog->logMessage("Player " +
				Ogre::StringConverter::toString(
				(int)m_playerDescription.m_ID) + " added");

		//Check to see if window object names already exist, if so delete them
		if(CEGUI::WindowManager::getSingleton().isWindowPresent("IsLeader" +
				CEGUI::PropertyHelper::intToString((int)m_playerDescription.m_ID)))
		{
			CEGUI::WindowManager::getSingleton().destroyWindow("IsLeader" +
					CEGUI::PropertyHelper::intToString((int)m_playerDescription.m_ID));
			CEGUI::WindowManager::getSingleton().destroyWindow(
					m_playerDescription.m_name);
		}

		isLeader = (CEGUI::RadioButton*)CEGUI::WindowManager::getSingleton().
				createWindow("OgreTray/RadioButton","IsLeader" +
				CEGUI::PropertyHelper::intToString((int)m_playerDescription.m_ID));
		playerName = (CEGUI::DefaultWindow*)CEGUI::WindowManager::getSingleton().
				createWindow("OgreTray/StaticText",m_playerDescription.m_name);

		isLeader->setGroupID(1);
		isLeader->setID((int)m_playerDescription.m_ID);
		if(m_playerDescription.m_ID == m_currentMatch.m_leaderID)
		{
			isLeader->setSelected(true);
		}
		if(m_playerDescription.m_ID == m_currentMatch.m_leaderID)
		{
			isLeader->setEnabled(true);
		}
		else
		{
			isLeader->setEnabled(false);
		}

		playerName->setText(m_playerDescription.m_name);

		offSet = offSet + CEGUI::UDim(0.1f, 0.0f);

		isLeader->setPosition(CEGUI::UVector2(
				CEGUI::UDim(0.0f, 0.0f),CEGUI::UDim(0.1f , 0.0f)));
		isLeader->setSize(CEGUI::UVector2(
				CEGUI::UDim(0.075f, 0.0f), CEGUI::UDim(0.075f, 0.0f)));
		playerName->setPosition(CEGUI::UVector2(
				CEGUI::UDim(0.1f, 0.0f),CEGUI::UDim(0.1f , 0.0f)));
		playerName->setSize(CEGUI::UVector2(
				CEGUI::UDim(0.2f, 0.0f), CEGUI::UDim(0.075f, 0.0f)));
		playerName->setProperty("FrameEnabled", "False");

		scrollpane->addChildWindow(isLeader);
		scrollpane->addChildWindow(playerName);

		isLeader->subscribeEvent(
				CEGUI::RadioButton::EventSelectStateChanged, CEGUI::Event::Subscriber(
				&MenuState::onLeaderClick, this));
	}
	return;
}

bool MenuState::onLeaderClick(const CEGUI::EventArgs &args)
{
	CEGUI::RadioButton *leaderRadio = (CEGUI::RadioButton*)
			CEGUI::WindowManager::getSingleton().getWindow("IsLeader" +
			CEGUI::PropertyHelper::intToString((int)m_playerDescription.m_ID));
	uint newLeaderID = leaderRadio->getSelectedButtonInGroup()->getID();
	if(newLeaderID != m_currentMatch.m_leaderID)
	{
		OgreFramework::getSingletonPtr()->m_pLog->logMessage(
				"Trying to changing Leader from " + Ogre::StringConverter::toString(
				(int)m_currentMatch.m_leaderID) + " to " +
				Ogre::StringConverter::toString((int)newLeaderID));

		if(RTT::ChangeLeader(newLeaderID) == false)
		{
			OgreFramework::getSingletonPtr()->m_pLog->logMessage(
					"WARNING: Change of leader on the server failed");
			m_currentMatch.m_leaderID = newLeaderID;
			return true;
		}

		OgreFramework::getSingletonPtr()->m_pLog->logMessage(
				"Successful leader change from " + Ogre::StringConverter::toString(
				(int)m_currentMatch.m_leaderID) +
				" to " + Ogre::StringConverter::toString((int)newLeaderID));

		m_currentMatch.m_leaderID = newLeaderID;
		for(int i = 0; i< MAX_PLAYERS_IN_MATCH; i++)
		{
			if(CEGUI::WindowManager::getSingleton().isWindowPresent(
					"IsLeader" + CEGUI::PropertyHelper::intToString(
					(int)m_otherPlayers[i].m_ID)))
			{
				leaderRadio = (CEGUI::RadioButton*)CEGUI::WindowManager::getSingleton().
						getWindow("IsLeader" + CEGUI::PropertyHelper::intToString(
						(int)m_otherPlayers[i].m_ID));
				leaderRadio->setEnabled(false);
			}
		}
	}
	return true;
}

void MenuState::serverLobby()
{
	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Server Lobby");
	CEGUI::Window *pMainWnd = CEGUI::WindowManager::getSingleton().getWindow(
			"RTT_ServerLobby");
	OgreFramework::getSingletonPtr()->m_pGUISystem->setGUISheet(pMainWnd);
	CEGUI::FrameWindow *pCreateMatchWnd = (CEGUI::FrameWindow*)pMainWnd->getChild(
			"RTT_CreateMatchWindow");
	pCreateMatchWnd->setVisible(false);
	m_bInMatch = false;
	m_currentMatch.m_ID = 0;
	mLocation = SERVERLOBBY;
	listMatches();
	return;
}

void MenuState::listMatches()
{
	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Listing Matches...");

	boost::posix_time::ptime epoch(boost::gregorian::date(1970,boost::gregorian::Jan,1));
	//TODO: Unused variable?
	//struct RTT::ServerStats stats = RTT::GetServerStats();
	struct RTT::MatchDescription descriptions[MATCHES_PER_PAGE];
	uint numMatchesThisPage = ListMatches(1, descriptions);
	CEGUI::ListboxTextItem *itemMultiColumnList;

	multiColumnListMatch->resetList();

	for(uint i = 0; i < numMatchesThisPage; i++)
	{
		multiColumnListMatch->addRow((int)descriptions[i].m_ID);
		itemMultiColumnList = new CEGUI::ListboxTextItem(
				CEGUI::PropertyHelper::intToString((int)descriptions[i].m_ID), i);
		itemMultiColumnList->setSelectionBrushImage("OgreTrayImages", "Select");
		multiColumnListMatch->setItem(itemMultiColumnList, 0, i);

		CEGUI::String playerCount = CEGUI::PropertyHelper::intToString(
				(int)descriptions[i].m_currentPlayerCount) + "/" +
				CEGUI::PropertyHelper::intToString((int)descriptions[i].m_maxPlayers);

		itemMultiColumnList = new CEGUI::ListboxTextItem(playerCount, i);
		itemMultiColumnList->setSelectionBrushImage("OgreTrayImages", "Select");
		multiColumnListMatch->setItem(itemMultiColumnList, 1, i);

		itemMultiColumnList = new CEGUI::ListboxTextItem(descriptions[i].m_name, i);
		itemMultiColumnList->setSelectionBrushImage("OgreTrayImages", "Select");
		multiColumnListMatch->setItem(itemMultiColumnList, 2, i);

		boost::posix_time::ptime time = epoch +
				boost::posix_time::seconds(descriptions[i].m_timeCreated);
		std::string timeString = boost::posix_time::to_simple_string(time);

		itemMultiColumnList = new CEGUI::ListboxTextItem(timeString.c_str(), i);
		itemMultiColumnList->setSelectionBrushImage("OgreTrayImages", "Select");
		multiColumnListMatch->setItem(itemMultiColumnList, 4, i);

	}
	return;
}

bool MenuState::listMatchesButton(const CEGUI::EventArgs &args)
{
	listMatches();
	return true;
}

bool MenuState::createMatchButton(const CEGUI::EventArgs &args)
{
	CEGUI::Window *pMainWnd =
			CEGUI::WindowManager::getSingleton().getWindow("RTT_ServerLobby");
	CEGUI::FrameWindow *pCreateMatchWnd =
			(CEGUI::FrameWindow*)pMainWnd->getChild("RTT_CreateMatchWindow");
	pCreateMatchWnd->setVisible(true);

	CEGUI::Combobox *mapCombobox =
			(CEGUI::Combobox*)pCreateMatchWnd->getChild("MapComboBox");
	mapCombobox->setReadOnly(true);
	mapCombobox->resetList();
	CEGUI::ListboxTextItem *itemCombobox = new CEGUI::ListboxTextItem("Cool Map", 1);
	itemCombobox->setSelectionBrushImage("OgreTrayImages", "Select");
	mapCombobox->addItem(itemCombobox);
	itemCombobox = new CEGUI::ListboxTextItem("Even Cooler Map", 2);
	itemCombobox->setSelectionBrushImage("OgreTrayImages", "Select");
	itemCombobox->setSelected(true);
	mapCombobox->addItem(itemCombobox);
	itemCombobox = new CEGUI::ListboxTextItem("ZOMFG", 3);
	itemCombobox->setSelectionBrushImage("OgreTrayImages", "Select");
	mapCombobox->addItem(itemCombobox);

	CEGUI::Combobox *maxPlayersCombobox =
			(CEGUI::Combobox*)pCreateMatchWnd->getChild("MaxPlayersComboBox");
	maxPlayersCombobox->setReadOnly(true);
	maxPlayersCombobox->resetList();

	//max number of players in a match is 8, minimum number of players is 2
	//TODO make these dynamically adjustable based on map
	int min = 2;
	int max = 8;

	for(int i = min; i <= max; i++)
	{
		//i-min+1 so the first value is placed in the first item
		itemCombobox = new CEGUI::ListboxTextItem(
				Ogre::StringConverter::toString(i), (i-min+1) );
		itemCombobox->setSelectionBrushImage("OgreTrayImages", "Select");
		maxPlayersCombobox->addItem(itemCombobox);
	}

	mLocation = CREATEMATCH;
	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Create Match Menu");
	return true;
}

bool MenuState::createMatchSubmitButton(const CEGUI::EventArgs &args)
{
	OgreFramework::getSingletonPtr()->m_pLog->logMessage(
			"Attempting to creating Match...");
	CEGUI::Window *pMainWnd =
			CEGUI::WindowManager::getSingleton().getWindow("RTT_ServerLobby");
	CEGUI::FrameWindow *pCreateMatchWnd =
			(CEGUI::FrameWindow*)pMainWnd->getChild("RTT_CreateMatchWindow");

	CEGUI::Editbox *matchNameBox =
			(CEGUI::Editbox*)pCreateMatchWnd->getChild("MatchNameBox");
	CEGUI::Combobox *mapCombobox =
			(CEGUI::Combobox*)pCreateMatchWnd->getChild("MapComboBox");
	CEGUI::Combobox *maxPlayersCombobox =
			(CEGUI::Combobox*)pCreateMatchWnd->getChild("MaxPlayersComboBox");
	//TODO: Unused variable?
	//CEGUI::Checkbox *privateCheckBox =
	//	(CEGUI::Checkbox*)pCreateMatchWnd->getChild("PrivateCheckBox");


	string matchName = matchNameBox->getText().c_str();
	if(matchName.size() < 1 && matchName.size() > 20)
	{
		pCreateMatchWnd->setText("Invalid match name length");
		return true;
	}

	CEGUI::ListboxItem *selectedItem = mapCombobox->getSelectedItem();
	if(selectedItem == NULL)
	{
		pCreateMatchWnd->setText("Please select a map");
		return true;
	}
	string mapName = selectedItem->getText().c_str();

	selectedItem = maxPlayersCombobox->getSelectedItem();
	if(selectedItem == NULL)
	{
		pCreateMatchWnd->setText("Please select a maximum number of players");
		return true;
	}
	int maxPlayers = Ogre::StringConverter::parseInt(selectedItem->getText().c_str());

	//TODO: Use this once private matches are implemented
	//bool privateMatch = privateCheckBox->isSelected();

	struct RTT::MatchOptions options;
	options.m_maxPlayers = maxPlayers;
	std::strncpy(options.m_name, matchNameBox->getText().c_str(),
			sizeof(options.m_name));

	if (CreateMatch(options, &m_currentMatch) )
	{
		matchLobby(1);
	}
	else
	{
		pCreateMatchWnd->setText("Server returned failure to create match");
		return true;
	}
	return true;
}
bool MenuState::onMatchNameActivate(const CEGUI::EventArgs &args)
{
	CEGUI::Window *pMainWnd =
			CEGUI::WindowManager::getSingleton().getWindow("RTT_ServerLobby");
	CEGUI::FrameWindow *pCreateMatchWnd =
			(CEGUI::FrameWindow*)pMainWnd->getChild("RTT_CreateMatchWindow");
	CEGUI::Editbox *tMatchNameBox =
			(CEGUI::Editbox*)pCreateMatchWnd->getChild("MatchNameBox");
	if(tMatchNameBox->getText() == "Enter Match Name...")
	{
		tMatchNameBox->setText("");
	}
	return true;
}

bool MenuState::onMatchNameDeactivate(const CEGUI::EventArgs &args)
{
	CEGUI::Window *pMainWnd =
			CEGUI::WindowManager::getSingleton().getWindow("RTT_ServerLobby");
	CEGUI::FrameWindow *pCreateMatchWnd =
			(CEGUI::FrameWindow*)pMainWnd->getChild("RTT_CreateMatchWindow");
	CEGUI::Editbox *tMatchNameBox =
			(CEGUI::Editbox*)pCreateMatchWnd->getChild("MatchNameBox");
	if(tMatchNameBox->getText() == "")
	{
		tMatchNameBox->setText("Enter Match Name...");
	}
	return true;
}

//Callback Events
void MenuState::LeaderChangedEvent(struct CallbackChange change)
{
	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Leader Change Event");
	if(change.m_type == RTT::CALLBACK_ERROR)
	{
		cerr << "ERROR: Got an error in callback processing" << endl;
		return;
	}
	//PlayerListColumns playerColumns;
	CEGUI::Window *pMainWnd =
			CEGUI::WindowManager::getSingleton().getWindow("RTT_MatchLobby");
	CEGUI::RadioButton *oldLeader =
			(CEGUI::RadioButton*)pMainWnd->getChild("IsLeader" +
			CEGUI::PropertyHelper::intToString((int)m_currentMatch.m_leaderID));
	CEGUI::RadioButton *newLeader =
			(CEGUI::RadioButton*)pMainWnd->getChild("IsLeader" +
			CEGUI::PropertyHelper::intToString((int)change.m_playerID));

	oldLeader->setSelected(false);
	newLeader->setSelected(true);

	m_currentMatch.m_leaderID = change.m_playerID;

	if(m_playerDescription.m_ID == m_currentMatch.m_leaderID)
	{
		newLeader->setEnabled(true);
	}
	else
	{
		newLeader->setEnabled(false);
	}

	return;
}

void MenuState::ProcessCallback(struct RTT::CallbackChange change)
{
	switch(change.m_type)
	{
		case LEADER_CHANGE:
		{
			LeaderChangedEvent(change);
			break;
		}
		default:
		{
			break;
		}
	}
}

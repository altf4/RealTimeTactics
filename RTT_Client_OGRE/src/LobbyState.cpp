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
    m_bInMatch = false;
    m_FrameEvent = Ogre::FrameEvent();
    m_bInit = false;
}

void LobbyState::enter()
{
	if(m_bInit == false)
	{
		m_bInit = true;
		OgreFramework::getSingletonPtr()->m_pLog->logMessage("Joining Server Lobby...");
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

		m_bQuit = false;


		CEGUI::Window *pMainWnd = CEGUI::WindowManager::getSingleton().getWindow("RTT_MatchLobby_GUI");

		CEGUI::PushButton *button = (CEGUI::PushButton*)pMainWnd->getChild("MatchExitButton");
		button->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&LobbyState::onExitButton, this));
		button = (CEGUI::PushButton*)pMainWnd->getChild("MatchBackButton");
		button->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&LobbyState::onBackButton, this));

		multiColumnListPlayer = (CEGUI::MultiColumnList*)pMainWnd->getChild("PlayersMCL");

		multiColumnListPlayer->addColumn("Leader", 0, CEGUI::UDim(0.17f, 0));
		multiColumnListPlayer->addColumn("Name", 1, CEGUI::UDim(0.25f, 0));
		multiColumnListPlayer->addColumn("Team", 2, CEGUI::UDim(0.40f, 0));

		pMainWnd = CEGUI::WindowManager::getSingleton().getWindow("RTT_ServerLobby_GUI");

		button = (CEGUI::PushButton*)pMainWnd->getChild("ServerExitButton");
		button->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&LobbyState::onExitButton, this));
		button = (CEGUI::PushButton*)pMainWnd->getChild("ServerBackButton");
		button->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&LobbyState::onBackButton, this));
		button = (CEGUI::PushButton*)pMainWnd->getChild("RefreshListButton");
		button->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&LobbyState::listMatchesButton, this));
		button = (CEGUI::PushButton*)pMainWnd->getChild("JoinMatchButton");
		button->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&LobbyState::JoinMatchButton, this));

		multiColumnListMatch = (CEGUI::MultiColumnList*)pMainWnd->getChild("MatchesMCL");

		multiColumnListMatch->addColumn("ID", 0, CEGUI::UDim(0.1f, 0));
		multiColumnListMatch->addColumn("Players", 1, CEGUI::UDim(0.20f, 0));
		multiColumnListMatch->addColumn("Name", 2, CEGUI::UDim(0.20f, 0));
		multiColumnListMatch->addColumn("Map", 3, CEGUI::UDim(0.20f, 0));
		multiColumnListMatch->addColumn("Time Created", 4, CEGUI::UDim(0.27f, 0));

		multiColumnListMatch->setSelectionMode(CEGUI::MultiColumnList::RowSingle);

		OgreFramework::getSingletonPtr()->m_pLog->logMessage("Initialized state");
		serverLobby();
	}
}

void LobbyState::serverLobby()
{

	CEGUI::Window *pMainWnd = CEGUI::WindowManager::getSingleton().getWindow("RTT_ServerLobby_GUI");
	OgreFramework::getSingletonPtr()->m_pGUISystem->setGUISheet(pMainWnd);
	m_bInMatch = false;
	listMatches();
}

void LobbyState::listMatches()
{
	boost::posix_time::ptime epoch(boost::gregorian::date(1970,boost::gregorian::Jan,1));
	struct RTT::ServerStats stats = RTT::GetServerStats();
	struct RTT::MatchDescription descriptions[MATCHES_PER_PAGE];
	uint numMatchesThisPage = ListMatches(1, descriptions);
	CEGUI::ListboxTextItem *itemMultiColumnList;

	multiColumnListMatch->resetList();

	for(uint i = 0; i < numMatchesThisPage; i++)
	{
		multiColumnListMatch->addRow((int)descriptions[i].m_ID);
		itemMultiColumnList = new CEGUI::ListboxTextItem(CEGUI::PropertyHelper::intToString((int)descriptions[i].m_ID), i);
		itemMultiColumnList->setSelectionBrushImage("OgreTrayImages", "Select");
		multiColumnListMatch->setItem(itemMultiColumnList, 0, i);

		CEGUI::String playerCount = CEGUI::PropertyHelper::intToString(
				(int)descriptions[i].m_currentPlayerCount) + "/" + CEGUI::PropertyHelper::intToString((int)descriptions[i].m_maxPlayers);

		itemMultiColumnList = new CEGUI::ListboxTextItem(playerCount, i);
		itemMultiColumnList->setSelectionBrushImage("OgreTrayImages", "Select");
		multiColumnListMatch->setItem(itemMultiColumnList, 1, i);

		itemMultiColumnList = new CEGUI::ListboxTextItem(descriptions[i].m_name, i);
		itemMultiColumnList->setSelectionBrushImage("OgreTrayImages", "Select");
		multiColumnListMatch->setItem(itemMultiColumnList, 2, i);

		boost::posix_time::ptime time = epoch + boost::posix_time::seconds(descriptions[i].m_timeCreated);
		std::string timeString = boost::posix_time::to_simple_string(time);

		itemMultiColumnList = new CEGUI::ListboxTextItem(timeString.c_str(), i);
		itemMultiColumnList->setSelectionBrushImage("OgreTrayImages", "Select");
		multiColumnListMatch->setItem(itemMultiColumnList, 4, i);

	}
}

bool LobbyState::listMatchesButton(const CEGUI::EventArgs &args)
{
	listMatches();
	return true;
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
	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Leaving Server Lobby...");

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
	if(m_bInMatch == true)
	{
		if( RTT::LeaveMatch() )
		{
			OgreFramework::getSingletonPtr()->m_pLog->logMessage("Leaving Match ID: " + Ogre::StringConverter::toString(m_currentMatch.m_ID));
			m_currentMatch.m_ID = 0;
			serverLobby();
		}
	}
	else
	{
		RTT::ExitServer();

		changeAppState(findByName("JoinCustomServerState"));
	}
	return true;
}

bool LobbyState::JoinMatchButton(const CEGUI::EventArgs &args)
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

	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Trying to join Match ID: " + Ogre::StringConverter::toString(matchID));

	RTT::PlayerDescription playerDescriptions[MAX_PLAYERS_IN_MATCH];

	uint playerCount = JoinMatch(matchID, playerDescriptions, &m_currentMatch);

	if( playerCount > 0 )
	{
		m_currentMatch.m_ID = matchID;
		OgreFramework::getSingletonPtr()->m_pLog->logMessage("Joined match ID: " + Ogre::StringConverter::toString(matchID));
		matchLobby(playerDescriptions, playerCount);
	}
	else
	{
		m_currentMatch.m_ID = 0;
		OgreFramework::getSingletonPtr()->m_pLog->logMessage("Failed to join match");
	}

	return true;
}

void LobbyState::matchLobby(RTT::PlayerDescription *playerDescriptions,
		uint playerCount)
{
	CEGUI::Window *pMainWnd = CEGUI::WindowManager::getSingleton().getWindow("RTT_MatchLobby_GUI");
	OgreFramework::getSingletonPtr()->m_pGUISystem->setGUISheet(pMainWnd);
	m_bInMatch = true;


	listPlayers();
}

void LobbyState::listPlayers()
{




}

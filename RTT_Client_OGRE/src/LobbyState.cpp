//============================================================================
// Name        : LobbyState.cpp
// Author      : Mark Petro
// Copyright   : 2011, GNU GPLv3
// Description : An OGRE based 3D frontend to the RTT project.
//	Built from the Advanced OGRE Framework tutorial found here:
//	http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Advanced+Ogre+Framework&structure=Tutorials
//============================================================================

#include "LobbyState.h"
#include "messaging/MessageManager.h"
#include "ClientProtocolHandler.h"

using namespace Ogre;

LobbyState::LobbyState()
{
    m_bQuit = false;
    m_FrameEvent = Ogre::FrameEvent();
}

void LobbyState::enter()
{
	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Entering LobbyState...");
	//RTT::MessageManager::Initialize(RTT::DIRECTION_TO_SERVER);
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

	button = (CEGUI::PushButton*)pMainWnd->getChild("RefreshListButton");
	button->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&LobbyState::listMatchesButton, this));

	multiColumnList = (CEGUI::MultiColumnList*)pMainWnd->getChild("MCL");

	multiColumnList->addColumn("ID", 0, CEGUI::UDim(0.1f, 0));
	multiColumnList->addColumn("Players", 1, CEGUI::UDim(0.20f, 0));
	multiColumnList->addColumn("Name", 2, CEGUI::UDim(0.20f, 0));
	multiColumnList->addColumn("Map", 3, CEGUI::UDim(0.20f, 0));
	multiColumnList->addColumn("Time Created", 4, CEGUI::UDim(0.27f, 0));

	multiColumnList->setSelectionMode(CEGUI::MultiColumnList::RowSingle); // MultiColumnList::RowMultiple


/*
	multiColumnList->addRow();
		itemMultiColumnList = new CEGUI::ListboxTextItem("1", 101);
			itemMultiColumnList->setSelectionBrushImage("OgreTrayImages", "Select");
			multiColumnList->setItem(itemMultiColumnList, 0, 0); // ColumnID, RowID
		itemMultiColumnList = new CEGUI::ListboxTextItem("0/4", 102);
			itemMultiColumnList->setSelectionBrushImage("OgreTrayImages", "Select");
			multiColumnList->setItem(itemMultiColumnList, 1, 0); // ColumnID, RowID
		itemMultiColumnList = new CEGUI::ListboxTextItem("CTF", 103);
			itemMultiColumnList->setSelectionBrushImage("OgreTrayImages", "Select");
			multiColumnList->setItem(itemMultiColumnList, 2, 0); // ColumnID, RowID
		itemMultiColumnList = new CEGUI::ListboxTextItem("Forest", 104);
			itemMultiColumnList->setSelectionBrushImage("OgreTrayImages", "Select");
			multiColumnList->setItem(itemMultiColumnList, 3, 0); // ColumnID, RowID
		itemMultiColumnList = new CEGUI::ListboxTextItem("Now", 105);
			itemMultiColumnList->setSelectionBrushImage("OgreTrayImages", "Select");
			multiColumnList->setItem(itemMultiColumnList, 4, 0); // ColumnID, RowID

	multiColumnList->addRow();
		itemMultiColumnList = new CEGUI::ListboxTextItem("2", 201);
			itemMultiColumnList->setSelectionBrushImage("OgreTrayImages", "Select");
			multiColumnList->setItem(itemMultiColumnList, 0, 1); // ColumnID, RowID
		itemMultiColumnList = new CEGUI::ListboxTextItem("0/2", 202);
			itemMultiColumnList->setSelectionBrushImage("OgreTrayImages", "Select");
			multiColumnList->setItem(itemMultiColumnList, 1, 1); // ColumnID, RowID
		itemMultiColumnList = new CEGUI::ListboxTextItem("DM", 203);
			itemMultiColumnList->setSelectionBrushImage("OgreTrayImages", "Select");
			multiColumnList->setItem(itemMultiColumnList, 2, 1); // ColumnID, RowID
		itemMultiColumnList = new CEGUI::ListboxTextItem("City", 204);
			itemMultiColumnList->setSelectionBrushImage("OgreTrayImages", "Select");
			multiColumnList->setItem(itemMultiColumnList, 3, 1); // ColumnID, RowID
		itemMultiColumnList = new CEGUI::ListboxTextItem("Now", 205);
			itemMultiColumnList->setSelectionBrushImage("OgreTrayImages", "Select");
			multiColumnList->setItem(itemMultiColumnList, 4, 1); // ColumnID, RowID

	multiColumnList->addRow();
		itemMultiColumnList = new CEGUI::ListboxTextItem("3", 301);
			itemMultiColumnList->setSelectionBrushImage("OgreTrayImages", "Select");
			multiColumnList->setItem(itemMultiColumnList, 0, 2); // ColumnID, RowID
		itemMultiColumnList = new CEGUI::ListboxTextItem("1/3", 302);
			itemMultiColumnList->setSelectionBrushImage("OgreTrayImages", "Select");
			multiColumnList->setItem(itemMultiColumnList, 1, 2); // ColumnID, RowID
		itemMultiColumnList = new CEGUI::ListboxTextItem("DM", 303);
			itemMultiColumnList->setSelectionBrushImage("OgreTrayImages", "Select");
			multiColumnList->setItem(itemMultiColumnList, 2, 2); // ColumnID, RowID
		itemMultiColumnList = new CEGUI::ListboxTextItem("City", 304);
			itemMultiColumnList->setSelectionBrushImage("OgreTrayImages", "Select");
			multiColumnList->setItem(itemMultiColumnList, 3, 2); // ColumnID, RowID
		itemMultiColumnList = new CEGUI::ListboxTextItem("Now", 305);
			itemMultiColumnList->setSelectionBrushImage("OgreTrayImages", "Select");
			multiColumnList->setItem(itemMultiColumnList, 4, 2); // ColumnID, RowID
			OgreFramework::getSingletonPtr()->m_pLog->logMessage("Got here 3");

*/



	//CEGUI::MCLGridRef grid_ref(0, 0); // Select according to a grid reference; second row
	//multiColumnList->setItemSelectState(grid_ref, true);


	m_bQuit = false;

	createScene();
}

void LobbyState::createScene()
{
	listMatches();
}
void LobbyState::listMatches()
{
	boost::posix_time::ptime epoch(boost::gregorian::date(1970,boost::gregorian::Jan,1));
		struct RTT::ServerStats stats = RTT::GetServerStats();
		struct RTT::MatchDescription descriptions[MATCHES_PER_PAGE];
		uint numMatchesThisPage = ListMatches(1, descriptions);
		CEGUI::ListboxTextItem* itemMultiColumnList;
		//m_match_lists->set_current_page(page);

		//MatchListColumns *columns = new MatchListColumns();

		//Glib::RefPtr<ListStore> refListStore = ListStore::create(*columns);
		//view->set_model(refListStore);
		multiColumnList->resetList();

		for(uint i = 0; i < numMatchesThisPage; i++)
		{
			//TreeModel::Row row = *(refListStore->append());

			multiColumnList->addRow();
			//row[columns->m_matchID] = (int)descriptions[i].m_ID;
			itemMultiColumnList = new CEGUI::ListboxTextItem(CEGUI::PropertyHelper::intToString((int)descriptions[i].m_ID), 301);
						itemMultiColumnList->setSelectionBrushImage("OgreTrayImages", "Select");
						multiColumnList->setItem(itemMultiColumnList, 0, i); // ColumnID, RowID
			//row[columns->m_maxPlayers] = (int)descriptions[i].m_maxPlayers;
			//row[columns->m_currentPlayers] = (int)descriptions[i].m_currentPlayerCount;
			CEGUI::String playerCount = CEGUI::PropertyHelper::intToString(
					(int)descriptions[i].m_currentPlayerCount) + "/" + CEGUI::PropertyHelper::intToString((int)descriptions[i].m_maxPlayers);
			itemMultiColumnList = new CEGUI::ListboxTextItem(playerCount, 302);
						itemMultiColumnList->setSelectionBrushImage("OgreTrayImages", "Select");
						multiColumnList->setItem(itemMultiColumnList, 1, i); // ColumnID, RowID
			//row[columns->m_name] = descriptions[i].m_name;
			itemMultiColumnList = new CEGUI::ListboxTextItem(descriptions[i].m_name, 303);
						itemMultiColumnList->setSelectionBrushImage("OgreTrayImages", "Select");
						multiColumnList->setItem(itemMultiColumnList, 2, i); // ColumnID, RowID

			boost::posix_time::ptime time = epoch + boost::posix_time::seconds(descriptions[i].m_timeCreated);
			std::string timeString = boost::posix_time::to_simple_string(time);

			//row[columns->m_timeCreated] = timeString;
			itemMultiColumnList = new CEGUI::ListboxTextItem(timeString.c_str(), 303);
						itemMultiColumnList->setSelectionBrushImage("OgreTrayImages", "Select");
						multiColumnList->setItem(itemMultiColumnList, 4, i); // ColumnID, RowID


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


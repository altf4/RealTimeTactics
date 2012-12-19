//============================================================================
// Name        : GameState.cpp
// Author      : Mark Petro
// Copyright   : 2011, GNU GPLv3
// Description : Built from the Advanced OGRE Framework tutorial found here:
//	http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Advanced+Ogre+Framework&structure=Tutorials
//============================================================================

#include "GameState.h"
#include "GameCommands.h"
#include "ClientGameState.h"

using namespace Ogre;
using namespace RTT;

GameState::GameState(void):
		m_mainPlayer(),
		m_playerCursor()
{
	m_moveSpeed = 0.1f;
	m_rotateSpeed = 0.3f;

	m_isLMouseDown = false;
	m_isRMouseDown = false;
	m_quit = false;
	m_isSettingsMode = false;
	m_isMoving = false;
}

void GameState::BuildUnits(void)
{
	std::vector<uint32_t> playerIDs = ClientGameState::Instance().GetPlayers();
	for(uint i = 0; i < playerIDs.size(); i++)
	{
		OgreUnit *unit = new OgreUnit();
		unit->m_unitEntity = m_sceneMgr->createEntity("ColorMarine.mesh");
		unit->m_unitEntity->setCastShadows(true);
		//unit->m_unitEntity->setMaterialName("BlueMarine");
		unit->m_unitNode = m_sceneMgr->getRootSceneNode()->createChildSceneNode("BlueMarine" + unit->m_ID);
		unit->m_unitNode->attachObject(unit->m_unitEntity);
		unit->m_unitNode->yaw(Degree(150));
		unit->m_x = unit->m_ID; //Start out at the location of your ID
		unit->m_y = 0;
		unit->m_maxMovement = 2;

		//TODO: This is bad and should be changed! We need a way for the server to assign unit numbers.
		unit->m_ID = playerIDs[i];

		if(!ClientGameState::Instance().AddUnit(unit))
		{
			cerr << "WARNING: Could not add unit number: " << unit->m_ID << endl;
		}
		cerr << "xxxDEBUGxxx: Added unit number: " << unit->m_ID << endl;
	}

	m_selectedUnit = ClientGameState::Instance().GetOurPlayerID();

	m_playerCursor.m_unitEntity = m_sceneMgr->createEntity("Cursor", "Marker.mesh");
	m_playerCursor.m_unitEntity->setMaterialName("Marker");
	m_playerCursor.m_unitNode = m_sceneMgr->getRootSceneNode()->createChildSceneNode("Cursor");
	m_playerCursor.m_unitNode->attachObject(m_playerCursor.m_unitEntity);
	m_playerCursor.m_x = 0;
	m_playerCursor.m_y = 0;
	m_playerCursor.m_unitNode->setVisible(false);
}

void GameState::Enter()
{
	OgreFramework::getSingletonPtr()->m_log->logMessage("Entering GameState...");

	m_sceneMgr = OgreFramework::getSingletonPtr()->m_root->createSceneManager(
			ST_GENERIC, "GameSceneMgr");
	m_sceneMgr->setAmbientLight(Ogre::ColourValue(0, 0, 0));
	//Shadowmaps	EXPERIMENTAL
	m_sceneMgr->setShadowTexturePixelFormat(Ogre::PF_FLOAT32_R);
	m_sceneMgr->setShadowTextureSelfShadow(true);
	//rttSceneManager->setShadowCasterRenderBackFaces(false);
	m_sceneMgr->setShadowTextureCasterMaterial("Ogre/DepthShadowmap/Caster/Float");
	//rttSceneManager->setShadowTextureReceiverMaterial("Ogre/DepthShadowmap/BasicTemplateMaterial");
	m_sceneMgr->setShadowTextureSize(1024);
	m_isMoving = false;

	BuildUnits();

	m_RSQ = m_sceneMgr->createRayQuery(Ray());
	m_RSQ->setQueryMask(OGRE_HEAD_MASK);

	m_camera = m_sceneMgr->createCamera("GameCamera");
	m_camera->setPosition(Ogre::Vector3(5.5,10.5,6.5));
	m_camera->lookAt(Ogre::Vector3(5.5,0,-4));
	m_camera->setNearClipDistance(5);

	m_camera->setAspectRatio(
		Real(OgreFramework::getSingletonPtr()->m_viewport->getActualWidth()) /
		Real(OgreFramework::getSingletonPtr()->m_viewport->getActualHeight()));

	OgreFramework::getSingletonPtr()->m_viewport->setCamera(m_camera);
	m_currentObject = NULL;

	BuildGUI();

	SetUnbufferedMode();

	CreateScene();
}

bool GameState::Pause()
{
	OgreFramework::getSingletonPtr()->m_log->logMessage("Pausing GameState...");
	OgreFramework::getSingletonPtr()->m_GUISystem->setGUISheet(0);
	return true;
}

void GameState::Resume()
{
	OgreFramework::getSingletonPtr()->m_log->logMessage("Resuming GameState...");

	//buildGUI();

	//OgreFramework::getSingletonPtr()->m_pViewport->setCamera(m_pCamera);

	OgreFramework::getSingletonPtr()->m_viewport->setCamera(m_camera);
//	OgreFramework::getSingletonPtr()->m_pGUIRenderer->setTargetSceneManager(m_pSceneMgr);

	OgreFramework::getSingletonPtr()->m_GUISystem->setGUISheet(CEGUI::WindowManager::getSingleton().getWindow("RTT_Game"));

	m_quit = false;
}

void GameState::Exit()
{
	OgreFramework::getSingletonPtr()->m_log->logMessage("Leaving GameState...");
	OgreFramework::getSingletonPtr()->m_GUISystem->setGUISheet(0);

	if(m_sceneMgr != NULL)
	{
		m_sceneMgr->destroyCamera(m_camera);
		m_sceneMgr->destroyQuery(m_RSQ);
		OgreFramework::getSingletonPtr()->m_root->destroySceneManager(m_sceneMgr);
	}
}

void GameState::CreateScene()
{
	int tileSize = 1;
	float posX = 0;
	float posY = 0;
	Entity* tileVector[8][8];
	string tileType = "DirtTile";
	SceneNode* nodeVector[8][8];

	Entity* rangeMarker[8][8];
	string rangeType = "Range";

	for(int x = 0; x < 8 * tileSize; x += tileSize) //build our columns
	{
		for(int y = 0; y < 8 * tileSize; y += tileSize) //build our rows
		{
			posX = x * 1.732;
			if(y % 2 != 0)//test for odd
			{
				posX -= .866;  //steps over
			}
			tileVector[x][y] = m_sceneMgr->createEntity(tileType +
					Ogre::StringConverter::toString(x) + Ogre::StringConverter::toString(y),
					"DirtTile.mesh", "RTT");
			tileVector[x][y]->setCastShadows(true);
			nodeVector[x][y] = m_sceneMgr->getRootSceneNode()->createChildSceneNode(
					tileType + Ogre::StringConverter::toString(x) +
					Ogre::StringConverter::toString(y), Ogre::Vector3(posX, posY, -y * 1.5));
			nodeVector[x][y]->attachObject(tileVector[x][y]);

			rangeMarker[x][y] = m_sceneMgr->createEntity(rangeType +
					Ogre::StringConverter::toString(x) + Ogre::StringConverter::toString(y),
					"Range.mesh", "RTT");
			rangeMarker[x][y]->setMaterialName("Range");
			m_rangeNode[x][y] = m_sceneMgr->getRootSceneNode()->createChildSceneNode(
					rangeType + Ogre::StringConverter::toString(x) + Ogre::StringConverter::toString(y),
					Ogre::Vector3(posX, posY, -y * 1.5));
			m_rangeNode[x][y]->attachObject(rangeMarker[x][y]);
			m_rangeNode[x][y]->yaw(Ogre::Degree(0));
			m_rangeNode[x][y]->setVisible(false);
		}
	}
	Entity *groundPlane = m_sceneMgr->createEntity("Ground", "Plane.mesh");
	groundPlane->setMaterialName("Claygreen");
	groundPlane->setCastShadows(false);
	SceneNode *groundPlaneNode = m_sceneMgr->getRootSceneNode()->createChildSceneNode("Ground", Ogre::Vector3(2.25, 0, 0));
	groundPlaneNode->attachObject(groundPlane);

	// Create a light
	Light *mainLight = m_sceneMgr->createLight("MainLight");
	mainLight->setType(Light::LT_POINT);
	//mainLight->mCastShadows=true;
	mainLight->setPosition(20, 30, 15);
	mainLight->setCastShadows(true);
}

bool GameState::keyPressed(const OIS::KeyEvent& keyEventRef)
{
	if(m_isSettingsMode == true)
	{

	}

	if(m_isChatMode == true)
	{
		OgreFramework::getSingletonPtr()->m_GUISystem->injectKeyDown(keyEventRef.key);
		OgreFramework::getSingletonPtr()->m_GUISystem->injectChar(keyEventRef.text);
	}

	if(OgreFramework::getSingletonPtr()->m_keyboard->isKeyDown(OIS::KC_ESCAPE))
	{
		//TODO Make this pause the game and allow menu options
		return true;
	}

	if(OgreFramework::getSingletonPtr()->m_keyboard->isKeyDown(OIS::KC_TAB))
	{
		m_isSettingsMode = !m_isSettingsMode;
		m_isChatMode = !m_isChatMode;

		if(m_isChatMode)
			SetBufferedMode();
		else
			SetUnbufferedMode();

		return true;
	}

	if(m_isSettingsMode && (OgreFramework::getSingletonPtr()->m_keyboard->isKeyDown(OIS::KC_RETURN) ||
		OgreFramework::getSingletonPtr()->m_keyboard->isKeyDown(OIS::KC_NUMPADENTER)))
	{
		CEGUI::Editbox *pChatInputBox = (CEGUI::Editbox*)m_chatWnd->getChild("ChatInputBox");
		CEGUI::MultiLineEditbox *pChatContentBox = (CEGUI::MultiLineEditbox*)m_chatWnd->getChild("ChatContentBox");
		pChatContentBox->setText(pChatContentBox->getText() + pChatInputBox->getText() + "\n");
		pChatInputBox->setText("");
		pChatContentBox->setCaratIndex(pChatContentBox->getText().size());
		pChatContentBox->ensureCaratIsVisible();
	}

	if(!m_isSettingsMode || (m_isSettingsMode &&
			!OgreFramework::getSingletonPtr()->m_keyboard->isKeyDown(OIS::KC_O)))
		OgreFramework::getSingletonPtr()->keyPressed(keyEventRef);

	switch (keyEventRef.key)
	{
		case OIS::KC_NUMPAD7: //Move North West
		{
			MoveCursor(RTT::NORTHWEST);
			break;
		}
		case OIS::KC_NUMPAD4: //Move North EDIT::::::::WEST!
		{
			MoveCursor(RTT::WEST);
			break;
		}
		case OIS::KC_NUMPAD9: //Move North East
		{
			MoveCursor(RTT::NORTHEAST);
			break;
		}
		case OIS::KC_NUMPAD1://Move South West
		{
			MoveCursor(RTT::SOUTHWEST);
			break;
		}
		case OIS::KC_NUMPAD6: //Move South  EDIT:::::::::EAST!
		{
			MoveCursor(RTT::EAST);
			break;
		}
		case OIS::KC_NUMPAD3: //Move South East
		{
			MoveCursor(RTT::SOUTHEAST);
			break;
		}
		case OIS::KC_M: //Move 'dialog'
		{
			cerr << "xxxDEBUGxxx: Trying to move unit: " << m_selectedUnit << endl;
			if(!m_isMoving)
			{
				MoveUnitOnScreen(m_selectedUnit);
			}
			else
			{
				MakeMove(m_selectedUnit);
			}
			break;
		}
		case OIS::KC_F: //Facing 'dialog'
		{
			m_isMoving = false;
			ShowRange(m_selectedUnit, m_isMoving);
			break;
		}
		default:
		{
			break;
		}
	}
	return true;
}

void GameState::MoveUnitOnScreen(uint32_t unitID)
{
	if(!m_isMoving)
	{
		Unit *unit = ClientGameState::Instance().GetUnit(unitID);
		if(unit == NULL)
		{
			return;
		}
		m_isMoving = true;
		m_playerCursor.m_unitNode->setPosition(((OgreUnit*)unit)->m_unitNode->getPosition());
		m_playerCursor.m_x = unit->m_x;
		m_playerCursor.m_y = unit->m_y;
		ShowRange(unitID, m_isMoving);
		m_playerCursor.m_unitNode->setVisible(m_isMoving);
	}
}

void GameState::MakeMove(uint32_t unitID)
{
	if(m_isMoving)
	{
		Unit *unit = ClientGameState::Instance().GetUnit(unitID);
		if(unit == NULL)
		{
			return;
		}
		struct MovementResult result = MoveUnit(unit->m_ID, unit->m_x, unit->m_y, m_playerCursor.m_x, m_playerCursor.m_y);
		if(result.m_result == MOVE_SUCCESS)
		{
			m_isMoving = false;
			ShowRange(unitID, m_isMoving);
			m_playerCursor.m_unitNode->setVisible(m_isMoving);
			((OgreUnit*)unit)->m_unitNode->setPosition(m_playerCursor.m_unitNode->getPosition());
			unit->m_x = m_playerCursor.m_x;
			unit->m_y = m_playerCursor.m_y;
		}
		else
		{
			LogManager::getSingletonPtr()->logMessage("Movement was rejected by the server");
		}
	}
}

void GameState::ShowRange(uint32_t unitID, bool& value)
{
	Unit *unit = ClientGameState::Instance().GetUnit(unitID);
	if(unit == NULL)
	{
		return;
	}
	//TODO: This just truncates. Eventually we'll want to make movement more involved
	int radius =unit->m_maxMovement;
	cout << "Currently at: " << unit->m_x << ", " << unit->m_y << endl;
	cout << "Radius: " << radius << endl;

	int pivotX = unit->m_x - radius;
	int pivotY = unit->m_y;

	for(int i = 0; i <= radius; i++ )
	{
		for(int j = 0; j < (radius*2) +1 -i; j++)
		{
			//cout << "j = " << j << endl;
			if((pivotX + j >= 0) && (pivotX + j <= 7) && (pivotY >= 0) && (pivotY <= 7))
			{
				m_rangeNode[pivotX + j][pivotY]->setVisible(value);
			}
		}

		if((pivotY % 2) == 0 )
		{
			pivotX++;
		}
		pivotY++;
	}

	//Reset the pivot point
	pivotX = unit->m_x - radius;
	pivotY = unit->m_y;

	for(int i = 0; i <= radius; i++ )
	{
		for(int j = 0; j < (radius*2)+1-i; j++)
		{
			//cout << "j = " << j << endl;
			if((pivotX + j >= 0) && (pivotX + j <= 7) &&
					(pivotY >= 0) && (pivotY <= 7))
			{
				m_rangeNode[pivotX + j][pivotY]->setVisible(value);
			}
		}

		if((pivotY % 2) == 0 )
		{
			pivotX++;
		}
		pivotY--;
	}

}

void GameState::MoveCursor(const RTT::Direction& moveDirection)
{
	if(m_isMoving)
	{
		//Ogre::Vector3 playerDirChange = m_playerCursor.m_unitNode->getPosition();
		int facingDirection = 0;
		switch(moveDirection)
		{
			case RTT::EAST:
			{
				LogManager::getSingletonPtr()->logMessage("Moving: East");
				m_playerCursor.m_x++;
				facingDirection = 90;
				break;
			}
			case RTT::NORTHWEST:
			{
				LogManager::getSingletonPtr()->logMessage("Moving: North West");
				if(m_playerCursor.m_y%2 != 0)
					m_playerCursor.m_x--;
				m_playerCursor.m_y++;
				facingDirection = -150;
				break;
			}
			case RTT::NORTHEAST:
			{
				LogManager::getSingletonPtr()->logMessage("Moving: North East");
				if(m_playerCursor.m_y%2 == 0)
					m_playerCursor.m_x++;
				m_playerCursor.m_y++;
				facingDirection = 150;
				break;
			}
			case RTT::WEST:
			{
				LogManager::getSingletonPtr()->logMessage("Moving: West");
				m_playerCursor.m_x--;
				facingDirection = -90;
				break;
			}
			case RTT::SOUTHWEST:
			{
				LogManager::getSingletonPtr()->logMessage("Moving: South West");
				if(m_playerCursor.m_y%2 != 0)
					m_playerCursor.m_x--;
				m_playerCursor.m_y--;
				facingDirection = -30;
				break;
			}
			case RTT::SOUTHEAST:
			{
				LogManager::getSingletonPtr()->logMessage("Moving: South East");
				if(m_playerCursor.m_y%2 == 0)
					m_playerCursor.m_x++;
				m_playerCursor.m_y--;
				facingDirection = 30;
				break;
			}
			default:
			{
				break;
			}
		}

		LogManager::getSingletonPtr()->logMessage("Location: " +
				Ogre::StringConverter::toString(m_playerCursor.m_x) +"," +
				Ogre::StringConverter::toString(m_playerCursor.m_y));
		if(m_playerCursor.m_y%2 != 0)
		{
			//LogManager::getSingletonPtr()->logMessage("Real Location: "
			//+ Ogre::StringConverter::toString(unitX*1.732-.866) +"," +
			//Ogre::StringConverter::toString(-unitY*1.5));
			m_playerCursor.m_unitNode->setPosition(Ogre::Vector3(
					m_playerCursor.m_x*1.732-.866,0,-m_playerCursor.m_y*1.5));
		}
		else
		{
			m_playerCursor.m_unitNode->setPosition(Ogre::Vector3(
					m_playerCursor.m_x*1.732,0,-m_playerCursor.m_y*1.5));
		}
		m_playerCursor.m_unitNode->resetOrientation();
		//Make sure we are facing the right way
		m_playerCursor.m_unitNode->yaw(Degree(facingDirection));
	}
}

bool GameState::keyReleased(const OIS::KeyEvent &keyEventRef)
{
	OgreFramework::getSingletonPtr()->keyPressed(keyEventRef);
	return true;
}

bool GameState::mouseMoved(const OIS::MouseEvent &evt)
{
//	if(OgreFramework::getSingletonPtr()->m_pTrayMgr->injectMouseMove(evt)) return true;

	//Mouse scroll wheel zoom
	static double scrollZoomTotal = 0;

	//If we're trying to zoom past the max.
	if((evt.state.Z.rel > 0) &&
			(evt.state.Z.rel + scrollZoomTotal < 720))
	{
		scrollZoomTotal += evt.state.Z.rel;
		m_camera->pitch(Degree(evt.state.Z.rel * 0.015f));
		m_translateVector.z = (evt.state.Z.rel * -0.04f);
		MoveCamera();
	}
	else if((evt.state.Z.rel < 0) &&
			(evt.state.Z.rel + scrollZoomTotal > 0))
	{
		scrollZoomTotal += evt.state.Z.rel;
		m_translateVector.z = (evt.state.Z.rel * -0.04f);
		MoveCamera();
		m_camera->pitch(Degree(evt.state.Z.rel * 0.015f));
	}

	OgreFramework::getSingletonPtr()->m_GUISystem->injectMouseWheelChange(evt.state.Z.rel);
	OgreFramework::getSingletonPtr()->m_GUISystem->injectMouseMove(evt.state.X.rel, evt.state.Y.rel);

	if(m_isRMouseDown)
	{
		m_camera->yaw(Degree(evt.state.X.rel * -0.1f));
		m_camera->pitch(Degree(evt.state.Y.rel * -0.1f));
	}

	return true;
}

bool GameState::mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{
	//if(OgreFramework::getSingletonPtr()->m_pTrayMgr->injectMouseDown(evt, id)) return true;

	if(id == OIS::MB_Left)
	{
		OnLeftPressed(evt);
		m_isLMouseDown = true;

		OgreFramework::getSingletonPtr()->m_GUISystem->injectMouseButtonDown(CEGUI::LeftButton);
	}
	else if(id == OIS::MB_Right)
	{
		m_isRMouseDown = true;
	}

	return true;
}

bool GameState::mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{
	//if(OgreFramework::getSingletonPtr()->m_pTrayMgr->injectMouseUp(evt, id)) return true;

	if(id == OIS::MB_Left)
	{
		m_isLMouseDown = false;
		OgreFramework::getSingletonPtr()->m_GUISystem->injectMouseButtonUp(CEGUI::LeftButton);
	}
	else if(id == OIS::MB_Right)
	{
		m_isRMouseDown = false;
	}

	return true;
}

void GameState::OnLeftPressed(const OIS::MouseEvent &evt)
{
	/*
	if(m_pCurrentObject)
	{
		m_pCurrentObject->showBoundingBox(false);
		m_pCurrentEntity->getSubEntity(1)->setMaterial(m_pOgreHeadMat);
	}

	Ogre::Ray mouseRay = m_pCamera->getCameraToViewportRay(
		OgreFramework::getSingletonPtr()->m_pMouse->getMouseState().X.abs / float(evt.state.width),
		OgreFramework::getSingletonPtr()->m_pMouse->getMouseState().Y.abs / float(evt.state.height));
	m_pRSQ->setRay(mouseRay);
	m_pRSQ->setSortByDistance(true);

	Ogre::RaySceneQueryResult &result = m_pRSQ->execute();
	Ogre::RaySceneQueryResult::iterator itr;

	for(itr = result.begin(); itr != result.end(); itr++)
	{
		if(itr->movable)
		{
			OgreFramework::getSingletonPtr()->m_pLog->logMessage(
					"MovableName: " + itr->movable->getName());
			m_pCurrentObject = m_pSceneMgr->getEntity(
					itr->movable->getName())->getParentSceneNode();
			OgreFramework::getSingletonPtr()->m_pLog->logMessage(
					"ObjName " + m_pCurrentObject->getName());
			m_pCurrentObject->showBoundingBox(true);
			m_pCurrentEntity = m_pSceneMgr->getEntity(itr->movable->getName());
			m_pCurrentEntity->getSubEntity(1)->setMaterial(m_pOgreHeadMatHigh);
			break;
		}
	}
	*/
}

bool GameState::OnExitButtonGame(const CEGUI::EventArgs &args)
{
	m_quit = true;
	return true;
}

void GameState::MoveCamera()
{
    if(OgreFramework::getSingletonPtr()->m_keyboard->isKeyDown(OIS::KC_LSHIFT))
    {
        m_camera->moveRelative(m_translateVector);
    }
    m_camera->moveRelative(m_translateVector / 10);
}

void GameState::GetInput()
{
    if(m_isSettingsMode == false)
    {
        if(OgreFramework::getSingletonPtr()->m_keyboard->isKeyDown(OIS::KC_A))
            m_translateVector.x = -m_moveScale;

        if(OgreFramework::getSingletonPtr()->m_keyboard->isKeyDown(OIS::KC_D))
            m_translateVector.x = m_moveScale;

        if(OgreFramework::getSingletonPtr()->m_keyboard->isKeyDown(OIS::KC_W))
            m_translateVector.z = -m_moveScale;

        if(OgreFramework::getSingletonPtr()->m_keyboard->isKeyDown(OIS::KC_S))
            m_translateVector.z = m_moveScale;

        if(OgreFramework::getSingletonPtr()->m_keyboard->isKeyDown(OIS::KC_Q))
            m_translateVector.y = -m_moveScale;

        if(OgreFramework::getSingletonPtr()->m_keyboard->isKeyDown(OIS::KC_E))
            m_translateVector.y = m_moveScale;

        //camera roll
        if(OgreFramework::getSingletonPtr()->m_keyboard->isKeyDown(OIS::KC_Z))
            m_camera->roll(Angle(-m_moveScale));

        if(OgreFramework::getSingletonPtr()->m_keyboard->isKeyDown(OIS::KC_X))
            m_camera->roll(Angle(m_moveScale));

        //reset roll
        if(OgreFramework::getSingletonPtr()->m_keyboard->isKeyDown(OIS::KC_C))
            m_camera->roll(-(m_camera->getRealOrientation().getRoll()));
    }
}

void GameState::Update(double timeSinceLastFrame)
{
	m_frameEvent.timeSinceLastFrame = timeSinceLastFrame;
	//OgreFramework::getSingletonPtr()->m_pTrayMgr->frameRenderingQueued(m_FrameEvent);

	if(m_quit == true)
	{
		PopAppState();
		return;
	}

//	if(!OgreFramework::getSingletonPtr()->m_pTrayMgr->isDialogVisible())
	//{
		/*
		if(m_pDetailsPanel->isVisible())
		{
			m_pDetailsPanel->setParamValue(0, Ogre::StringConverter::toString(m_pCamera->getDerivedPosition().x));
			m_pDetailsPanel->setParamValue(1, Ogre::StringConverter::toString(m_pCamera->getDerivedPosition().y));
			m_pDetailsPanel->setParamValue(2, Ogre::StringConverter::toString(m_pCamera->getDerivedPosition().z));
			m_pDetailsPanel->setParamValue(3, Ogre::StringConverter::toString(m_pCamera->getDerivedOrientation().w));
			m_pDetailsPanel->setParamValue(4, Ogre::StringConverter::toString(m_pCamera->getDerivedOrientation().x));
			m_pDetailsPanel->setParamValue(5, Ogre::StringConverter::toString(m_pCamera->getDerivedOrientation().y));
			m_pDetailsPanel->setParamValue(6, Ogre::StringConverter::toString(m_pCamera->getDerivedOrientation().z));
			if(m_bSettingsMode)
				m_pDetailsPanel->setParamValue(7, "Buffered Input");
			else
				m_pDetailsPanel->setParamValue(7, "Un-Buffered Input");
		}
		*/
	//}

	m_moveScale = m_moveSpeed   * timeSinceLastFrame;
	m_rotScale  = m_rotateSpeed * timeSinceLastFrame;

	m_translateVector = Vector3::ZERO;

	GetInput();
	MoveCamera();
}

void GameState::BuildGUI()
{

//	OgreFramework::getSingletonPtr()->m_pGUIRenderer->setTargetSceneManager(m_pSceneMgr);

	OgreFramework::getSingletonPtr()->m_GUISystem->setDefaultMouseCursor((CEGUI::utf8*)"OgreTrayImages", (CEGUI::utf8*)"MouseArrow");
	CEGUI::MouseCursor::getSingleton().setImage("OgreTrayImages", "MouseArrow");
	const OIS::MouseState state = OgreFramework::getSingletonPtr()->m_mouse->getMouseState();
	CEGUI::Point mousePos = CEGUI::MouseCursor::getSingleton().getPosition();
	CEGUI::System::getSingleton().injectMouseMove(state.X.abs-mousePos.d_x,state.Y.abs-mousePos.d_y);

	m_mainWnd = CEGUI::WindowManager::getSingleton().getWindow("RTT_Game");
	m_chatWnd = CEGUI::WindowManager::getSingleton().getWindow("ChatWnd");

	OgreFramework::getSingletonPtr()->m_GUISystem->setGUISheet(m_mainWnd);

	CEGUI::PushButton* pExitButton = (CEGUI::PushButton*)m_mainWnd->getChild("ExitButton_Game");
	pExitButton->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&GameState::OnExitButtonGame, this));

	m_isLMouseDown = m_isRMouseDown = false;
	m_quit = false;
	m_isChatMode = false;

	/*
	OgreFramework::getSingletonPtr()->m_pTrayMgr->showFrameStats(OgreBites::TL_BOTTOMLEFT);
	//OgreFramework::getSingletonPtr()->m_pTrayMgr->showLogo(OgreBites::TL_BOTTOMRIGHT);
	//OgreFramework::getSingletonPtr()->m_pTrayMgr->createLabel(OgreBites::TL_TOP, "GameLbl",
	//	"Game mode", 250);
	OgreFramework::getSingletonPtr()->m_pTrayMgr->showCursor();

	Ogre::StringVector items;
	items.push_back("cam.pX");
	items.push_back("cam.pY");
	items.push_back("cam.pZ");
	items.push_back("cam.oW");
	items.push_back("cam.oX");
	items.push_back("cam.oY");
	items.push_back("cam.oZ");
	items.push_back("Mode");

	//m_pDetailsPanel = OgreFramework::getSingletonPtr()->m_pTrayMgr->createParamsPanel(
	//	OgreBites::TL_TOPLEFT, "DetailsPanel", 200, items);
	//m_pDetailsPanel->hide();

	Ogre::String infoText = "[TAB] - Switch input mode\n\n[W] - Forward / Mode up\n[S] - "
			"Backwards/ Mode down\n[A] - Left\n";
	infoText.append("[D] - Right\n\nPress [SHIFT] to move faster\n\n[O] - Toggle FPS / logo\n");
	infoText.append("[Print] - Take screenshot\n\n[ESC] - Exit");
	//OgreFramework::getSingletonPtr()->m_pTrayMgr->createTextBox(
	//	OgreBites::TL_RIGHT, "InfoPanel", infoText, 300, 220);

	Ogre::StringVector chatModes;
	chatModes.push_back("Solid mode");
	chatModes.push_back("Wireframe mode");
	chatModes.push_back("Point mode");
	//OgreFramework::getSingletonPtr()->m_pTrayMgr->createLongSelectMenu(
	//	OgreBites::TL_TOPRIGHT, "ChatModeSelMenu", "ChatMode", 200, 3, chatModes);
	 */
}

void GameState::SetBufferedMode()
{
	CEGUI::Editbox* pModeCaption = (CEGUI::Editbox*)m_mainWnd->getChild("ModeCaption");
	pModeCaption->setText("Buffered Input Mode");

	CEGUI::Editbox* pChatInputBox = (CEGUI::Editbox*)m_chatWnd->getChild("ChatInputBox");
	pChatInputBox->setText("");
	pChatInputBox->activate();
	pChatInputBox->captureInput();

	CEGUI::MultiLineEditbox* pControlsPanel = (CEGUI::MultiLineEditbox*)m_mainWnd->getChild("ControlsPanel");
	pControlsPanel->setText("[Tab] - To switch between input modes\n\nAll keys to write in the chat box.\n\nPress [Enter] or [Return] to send message.\n\n[Print] - Take screenshot\n\n[Esc] - Quit to main menu");
}

void GameState::SetUnbufferedMode()
{
	CEGUI::Editbox* pModeCaption = (CEGUI::Editbox*)m_mainWnd->getChild("ModeCaption");
	pModeCaption->setText("Unuffered Input Mode");

	CEGUI::MultiLineEditbox* pControlsPanel = (CEGUI::MultiLineEditbox*)m_mainWnd->getChild("ControlsPanel");
	pControlsPanel->setText("[Tab] - To switch between input modes\n\n[W] - Forward\n[S] - Backwards\n[A] - Left\n[D] - Right\n\nPress [Shift] to move faster\n\n[O] - Toggle Overlays\n[Print] - Take screenshot\n\n[Esc] - Quit to main menu");
}

void GameState::ProcessCallback(struct RTT::ServerEvent event)
{
	if(event.m_appState != IN_GAME)
	{
		return;
	}

	switch(event.m_type)
	{
		case EVENT_UNIT_MOVED_DISTANT:
		{
			cout << "xxxDEBUGxxx ENEMY UNIT:" << event.m_unitID << " moved to " << event.m_dest.m_x << ", " << event.m_dest.m_y << endl;
			break;
		}
		default:
		{
			break;
		}
	}
}

//void GameState::itemSelected(OgreBites::SelectMenu *menu)
//{
	//switch(menu->getSelectionIndex())
	//{
		//case 0:
		//{
		//	m_pCamera->setPolygonMode(Ogre::PM_SOLID);
		//	break;
		//}
		//case 1:
//		{
//			m_pCamera->setPolygonMode(Ogre::PM_WIREFRAME);
//			break;
//		}
//		case 2:
//		{
//			m_pCamera->setPolygonMode(Ogre::PM_POINTS);
//			break;
//		}
//	}
//}

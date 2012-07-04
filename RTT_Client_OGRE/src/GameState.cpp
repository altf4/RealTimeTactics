//============================================================================
// Name        : GameState.cpp
// Author      : Mark Petro
// Copyright   : 2011, GNU GPLv3
// Description : Built from the Advanced OGRE Framework tutorial found here:
//	http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Advanced+Ogre+Framework&structure=Tutorials
//============================================================================

#include "GameState.h"

using namespace Ogre;

GameState::GameState(void):
		m_mainPlayer(),
		m_playerCursor()
{
	m_MoveSpeed		= 0.1f;
	m_RotateSpeed	= 0.3f;

	m_bLMouseDown       = false;
	m_bRMouseDown       = false;
	m_bQuit             = false;
	m_bSettingsMode     = false;
	m_isMoving 			= false;

	//m_pDetailsPanel	= NULL;
}

void GameState::buildUnits(void)
{
	//mainPlayer.ogreUnits[0] = new RTT_Ogre_Unit();

	m_mainPlayer.ogreUnits.m_unitEntity = m_pSceneMgr->createEntity(
			"BlueMarine", "ColorMarine.mesh");

	m_mainPlayer.ogreUnits.m_unitEntity->setCastShadows(true);
	m_mainPlayer.ogreUnits.m_unitEntity->setMaterialName("BlueMarine");
	m_mainPlayer.ogreUnits.m_unitNode =
			m_pSceneMgr->getRootSceneNode()->createChildSceneNode("BlueMarine");
	m_mainPlayer.ogreUnits.m_unitNode->attachObject(m_mainPlayer.ogreUnits.m_unitEntity);
	m_mainPlayer.ogreUnits.m_unitNode->yaw(Degree(150));
	m_mainPlayer.ogreUnits.m_locationX = 0;
	m_mainPlayer.ogreUnits.m_locationY = 0;
	m_mainPlayer.ogreUnits.m_moveRange = 3;

	m_playerCursor.m_unitEntity = m_pSceneMgr->createEntity("Cursor", "Marker.mesh");
	m_playerCursor.m_unitEntity->setMaterialName("Marker");
	m_playerCursor.m_unitNode = m_pSceneMgr->getRootSceneNode()->createChildSceneNode("Cursor");
	m_playerCursor.m_unitNode->attachObject(m_playerCursor.m_unitEntity);
	m_playerCursor.m_locationX = 0;
	m_playerCursor.m_locationY = 0;
	m_playerCursor.m_unitNode->setVisible(false);
}

void GameState::enter()
{
	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Entering GameState...");

	m_pSceneMgr = OgreFramework::getSingletonPtr()->m_pRoot->createSceneManager(
			ST_GENERIC, "GameSceneMgr");
	m_pSceneMgr->setAmbientLight(Ogre::ColourValue(0, 0, 0));
	//Shadowmaps	EXPERIMENTAL
	m_pSceneMgr->setShadowTexturePixelFormat(Ogre::PF_FLOAT32_R);
	m_pSceneMgr->setShadowTextureSelfShadow(true);
	//rttSceneManager->setShadowCasterRenderBackFaces(false);
	m_pSceneMgr->setShadowTextureCasterMaterial("Ogre/DepthShadowmap/Caster/Float");
	//rttSceneManager->setShadowTextureReceiverMaterial("Ogre/DepthShadowmap/BasicTemplateMaterial");
	m_pSceneMgr->setShadowTextureSize(1024);
	m_isMoving = false;

	buildUnits();

	m_pRSQ = m_pSceneMgr->createRayQuery(Ray());
	m_pRSQ->setQueryMask(OGRE_HEAD_MASK);

	m_pCamera = m_pSceneMgr->createCamera("GameCamera");
	m_pCamera->setPosition(Ogre::Vector3(5.5,10.5,6.5));
	m_pCamera->lookAt(Ogre::Vector3(5.5,0,-4));
	m_pCamera->setNearClipDistance(5);

	m_pCamera->setAspectRatio(
		Real(OgreFramework::getSingletonPtr()->m_pViewport->getActualWidth()) /
		Real(OgreFramework::getSingletonPtr()->m_pViewport->getActualHeight()));

	OgreFramework::getSingletonPtr()->m_pViewport->setCamera(m_pCamera);
	m_pCurrentObject = NULL;

	buildGUI();

	setUnbufferedMode();

	createScene();
}

bool GameState::pause()
{
	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Pausing GameState...");
	OgreFramework::getSingletonPtr()->m_pGUISystem->setGUISheet(0);
	return true;
}

void GameState::resume()
{
	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Resuming GameState...");

	//buildGUI();

	//OgreFramework::getSingletonPtr()->m_pViewport->setCamera(m_pCamera);

	OgreFramework::getSingletonPtr()->m_pViewport->setCamera(m_pCamera);
//	OgreFramework::getSingletonPtr()->m_pGUIRenderer->setTargetSceneManager(m_pSceneMgr);

	OgreFramework::getSingletonPtr()->m_pGUISystem->setGUISheet(CEGUI::WindowManager::getSingleton().getWindow("AOF_GUI_GAME"));

	m_bQuit = false;
}

void GameState::exit()
{
	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Leaving GameState...");

	OgreFramework::getSingletonPtr()->m_pGUISystem->setGUISheet(0);

	m_pSceneMgr->destroyCamera(m_pCamera);
	m_pSceneMgr->destroyQuery(m_pRSQ);
	if(m_pSceneMgr)
	{
		OgreFramework::getSingletonPtr()->m_pRoot->destroySceneManager(m_pSceneMgr);
	}
}

void GameState::createScene()
{
	//m_pSceneMgr->createLight("Light")->setPosition(75,75,75);

	//BUTCHERED  Proof of concept/scratchboard

	int tileSize = 1;
	float posX = 0;
	float posY = 0;
	//float posZ = 0;
	Entity* tileVector[8][8];
	string tileType = "DirtTile";
	SceneNode* nodeVector[8][8];

	Entity* rangeMarker[8][8];
	//m_mainPlayer.rangeNode[8][8];
	string rangeType = "Range";

	//Entity* blueMarine = rttSceneManager->createEntity("BlueMarine", "BlueMarine.mesh");
	//blueMarine->setCastShadows(true);
	//SceneNode* blueMarineNode = rttSceneManager->getRootSceneNode()->createChildSceneNode("BlueMarine");
	//blueMarineNode->attachObject(blueMarine);
	//blueMarineNode->yaw(Degree(90));
	Entity* redMarine = m_pSceneMgr->createEntity("RedMarine", "ColorMarine.mesh");
	redMarine->setCastShadows(true);
	redMarine->setMaterialName("RedMarine");
	SceneNode* redMarineNode = m_pSceneMgr->getRootSceneNode()->createChildSceneNode(
			"RedMarine", Ogre::Vector3(7*1.732 -.866,0,-7*1.5));
	redMarineNode->attachObject(redMarine);
	redMarineNode->yaw(Degree(-30));

	for(int x=0;x < 8*tileSize; x+=tileSize) //build our columns
	{
		for(int y=0; y<8*tileSize;y+=tileSize) //build our rows
		{
			posX = x *1.732;
			if(y%2 != 0)//test for odd
			{
				posX -= .866;  //steps over
			}
			tileVector[x][y] = m_pSceneMgr->createEntity(tileType +
					Ogre::StringConverter::toString(x) + Ogre::StringConverter::toString(y),
					"DirtTile.mesh", "RTT");
			tileVector[x][y]->setCastShadows(true);
			nodeVector[x][y] = m_pSceneMgr->getRootSceneNode()->createChildSceneNode(
					tileType + Ogre::StringConverter::toString(x) +
					Ogre::StringConverter::toString(y), Ogre::Vector3(posX, posY, -y*1.5));
			nodeVector[x][y]->attachObject(tileVector[x][y]);

			rangeMarker[x][y] = m_pSceneMgr->createEntity(rangeType +
					Ogre::StringConverter::toString(x) + Ogre::StringConverter::toString(y),
					"Range.mesh", "RTT");
			rangeMarker[x][y]->setMaterialName("Range");
			m_mainPlayer.rangeNode[x][y] = m_pSceneMgr->getRootSceneNode()->createChildSceneNode(rangeType + Ogre::StringConverter::toString(x) + Ogre::StringConverter::toString(y), Ogre::Vector3(posX, posY, -y*1.5));
			m_mainPlayer.rangeNode[x][y]->attachObject(rangeMarker[x][y]);
			m_mainPlayer.rangeNode[x][y]->yaw(Ogre::Degree(0));
			m_mainPlayer.rangeNode[x][y]->setVisible(false);
		}
	}
	//end BUTCHERING
	Entity *groundPlane = m_pSceneMgr->createEntity("Ground", "Plane.mesh");
	groundPlane->setMaterialName("Claygreen");
	groundPlane->setCastShadows(false);
	SceneNode *groundPlaneNode = m_pSceneMgr->getRootSceneNode()->createChildSceneNode("Ground", Ogre::Vector3(2.25,0,0));
	groundPlaneNode->attachObject(groundPlane);
	//groundPlaneNode->scale(25,25,25);

	// Create a light
	Light *mainLight = m_pSceneMgr->createLight("MainLight");
	mainLight->setType(Light::LT_POINT);
	//mainLight->mCastShadows=true;
	mainLight->setPosition(20,30,15);
	mainLight->setCastShadows(true);
}

bool GameState::keyPressed(const OIS::KeyEvent& keyEventRef)
{
	if(m_bSettingsMode == true)
	{
		/*
		if(OgreFramework::getSingletonPtr()->m_pKeyboard->isKeyDown(OIS::KC_S))
		{
			OgreBites::SelectMenu* pMenu =
					(OgreBites::SelectMenu*)OgreFramework::getSingletonPtr()->m_pTrayMgr->getWidget(
							"ChatModeSelMenu");
			if(pMenu->getSelectionIndex() + 1 < (int)pMenu->getNumItems())
			{
				pMenu->selectItem(pMenu->getSelectionIndex() + 1);
			}
		}

		if(OgreFramework::getSingletonPtr()->m_pKeyboard->isKeyDown(OIS::KC_W))
		{
			OgreBites::SelectMenu* pMenu = (OgreBites::SelectMenu*)OgreFramework::getSingletonPtr()->m_pTrayMgr->getWidget("ChatModeSelMenu");
			if(pMenu->getSelectionIndex() - 1 >= 0)
			{
				pMenu->selectItem(pMenu->getSelectionIndex() - 1);
			}
		}
		*/
	}

	if(m_bChatMode == true)
	{
		OgreFramework::getSingletonPtr()->m_pGUISystem->injectKeyDown(keyEventRef.key);
		OgreFramework::getSingletonPtr()->m_pGUISystem->injectChar(keyEventRef.text);
	}

	if(OgreFramework::getSingletonPtr()->m_pKeyboard->isKeyDown(OIS::KC_ESCAPE))
	{
		//pushAppState(findByName("PauseState"));
		return true;
	}
/*
	if(OgreFramework::getSingletonPtr()->m_pKeyboard->isKeyDown(OIS::KC_I))
	{
		if(m_pDetailsPanel->getTrayLocation() == OgreBites::TL_NONE)
		{
			OgreFramework::getSingletonPtr()->m_pTrayMgr->moveWidgetToTray(
					m_pDetailsPanel, OgreBites::TL_TOPLEFT, 0);
			m_pDetailsPanel->show();
		}
		else
		{
			OgreFramework::getSingletonPtr()->m_pTrayMgr->removeWidgetFromTray(m_pDetailsPanel);
			m_pDetailsPanel->hide();
		}
	}
*/
	if(OgreFramework::getSingletonPtr()->m_pKeyboard->isKeyDown(OIS::KC_TAB))
	{
		m_bSettingsMode = !m_bSettingsMode;
		m_bChatMode = !m_bChatMode;

		if(m_bChatMode)
			setBufferedMode();
		else
			setUnbufferedMode();

		return true;
	}

	if(m_bSettingsMode && (OgreFramework::getSingletonPtr()->m_pKeyboard->isKeyDown(OIS::KC_RETURN) ||
		OgreFramework::getSingletonPtr()->m_pKeyboard->isKeyDown(OIS::KC_NUMPADENTER)))
	{
		CEGUI::Editbox *pChatInputBox = (CEGUI::Editbox*)m_pChatWnd->getChild("ChatInputBox");
		CEGUI::MultiLineEditbox *pChatContentBox = (CEGUI::MultiLineEditbox*)m_pChatWnd->getChild("ChatContentBox");
		pChatContentBox->setText(pChatContentBox->getText() + pChatInputBox->getText() + "\n");
		pChatInputBox->setText("");
		pChatContentBox->setCaratIndex(pChatContentBox->getText().size());
		pChatContentBox->ensureCaratIsVisible();
	}

	if(!m_bSettingsMode || (m_bSettingsMode &&
			!OgreFramework::getSingletonPtr()->m_pKeyboard->isKeyDown(OIS::KC_O)))
		OgreFramework::getSingletonPtr()->keyPressed(keyEventRef);

	switch (keyEventRef.key)
	{
		case OIS::KC_NUMPAD7: //Move North West
		{
			moveCursor(RTT::NORTHWEST);
			break;
		}
		case OIS::KC_NUMPAD4: //Move North EDIT::::::::WEST!
		{
			moveCursor(RTT::WEST);
			break;
		}
		case OIS::KC_NUMPAD9: //Move North East
		{
			moveCursor(RTT::NORTHEAST);
			break;
		}
		case OIS::KC_NUMPAD1://Move South West
		{
			moveCursor(RTT::SOUTHWEST);
			break;
		}
		case OIS::KC_NUMPAD6: //Move South  EDIT:::::::::EAST!
		{
			moveCursor(RTT::EAST);
			break;
		}
		case OIS::KC_NUMPAD3: //Move South East
		{
			moveCursor(RTT::SOUTHEAST);
			break;
		}
		case OIS::KC_M: //Move 'dialog'
		{
			if(!m_isMoving)
			{
				moveUnit(m_mainPlayer.ogreUnits);
				break;
			}
			else
				makeMove(m_mainPlayer.ogreUnits);
			break;
		}
		case OIS::KC_F: //Facing 'dialog'
		{
			m_isMoving = false;
			showRange(m_mainPlayer.ogreUnits, m_isMoving);
			break;
		}
		default:
		{
			break;
		}
	}
	return true;
}

void GameState::moveUnit(RTT::RTT_Ogre_Unit& toMove)
{
	if(!m_isMoving)
	{
		m_isMoving = true;
		m_playerCursor.m_unitNode->setPosition(toMove.m_unitNode->getPosition());
		m_playerCursor.m_locationX = toMove.m_locationX;
		m_playerCursor.m_locationY = toMove.m_locationY;
		showRange(toMove, m_isMoving);
		m_playerCursor.m_unitNode->setVisible(m_isMoving);
	}
}

void GameState::makeMove(RTT::RTT_Ogre_Unit& toMove)
{
	if(m_isMoving)
	{
		m_isMoving = false;
		showRange(toMove, m_isMoving);
		m_playerCursor.m_unitNode->setVisible(m_isMoving);
		toMove.m_unitNode->setPosition(m_playerCursor.m_unitNode->getPosition());
		toMove.m_locationX = m_playerCursor.m_locationX;
		toMove.m_locationY = m_playerCursor.m_locationY;
	}
}

void GameState::showRange(RTT::RTT_Ogre_Unit& toShow, bool& value)
{
	int radius = toShow.m_moveRange;
	cout << "Currently at: " << toShow.m_locationX << ", " << toShow.m_locationY << endl;
	cout << "Radius: " << radius << endl;

	int pivotX = toShow.m_locationX - radius;
	int pivotY = toShow.m_locationY;

	for(int i = 0; i <= radius; i++ )
	{
		for(int j = 0; j < (radius*2) +1 -i; j++)
		{
			//cout << "j = " << j << endl;
			if((pivotX + j >= 0) && (pivotX + j <= 7) && (pivotY >= 0) && (pivotY <= 7))
			{
				LogManager::getSingletonPtr()->logMessage("Range: " +
						Ogre::StringConverter::toString(pivotX + j) + "," +
						Ogre::StringConverter::toString(pivotY));
				m_mainPlayer.rangeNode[pivotX + j][pivotY]->setVisible(value);
			}
		}

		if((pivotY % 2) == 0 )
		{
			pivotX++;
		}
		pivotY++;
	}

	//Reset the pivot point
	pivotX = toShow.m_locationX - radius;
	pivotY = toShow.m_locationY;

	for(int i = 0; i <= radius; i++ )
	{
		for(int j = 0; j < (radius*2)+1-i; j++)
		{
			//cout << "j = " << j << endl;
			if((pivotX + j >= 0) && (pivotX + j <= 7) &&
					(pivotY >= 0) && (pivotY <= 7))
			{
				LogManager::getSingletonPtr()->logMessage("Range: " +
						Ogre::StringConverter::toString(pivotX + j) + "," +
						Ogre::StringConverter::toString(pivotY));
				m_mainPlayer.rangeNode[pivotX + j][pivotY]->setVisible(value);
			}
		}

		if((pivotY % 2) == 0 )
		{
			pivotX++;
		}
		pivotY--;
	}

}

void GameState::moveCursor(const RTT::Direction& moveDirection)
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
				m_playerCursor.m_locationX++;
				facingDirection = 90;
				break;
			}
			case RTT::NORTHWEST:
			{
				LogManager::getSingletonPtr()->logMessage("Moving: North West");
				if(m_playerCursor.m_locationY%2 != 0)
					m_playerCursor.m_locationX--;
				m_playerCursor.m_locationY++;
				facingDirection = -150;
				break;
			}
			case RTT::NORTHEAST:
			{
				LogManager::getSingletonPtr()->logMessage("Moving: North East");
				if(m_playerCursor.m_locationY%2 == 0)
					m_playerCursor.m_locationX++;
				m_playerCursor.m_locationY++;
				facingDirection = 150;
				break;
			}
			case RTT::WEST:
			{
				LogManager::getSingletonPtr()->logMessage("Moving: West");
				m_playerCursor.m_locationX--;
				facingDirection = -90;
				break;
			}
			case RTT::SOUTHWEST:
			{
				LogManager::getSingletonPtr()->logMessage("Moving: South West");
				if(m_playerCursor.m_locationY%2 != 0)
					m_playerCursor.m_locationX--;
				m_playerCursor.m_locationY--;
				facingDirection = -30;
				break;
			}
			case RTT::SOUTHEAST:
			{
				LogManager::getSingletonPtr()->logMessage("Moving: South East");
				if(m_playerCursor.m_locationY%2 == 0)
					m_playerCursor.m_locationX++;
				m_playerCursor.m_locationY--;
				facingDirection = 30;
				break;
			}
			default:
			{
				break;
			}
		}

		LogManager::getSingletonPtr()->logMessage("Location: " +
				Ogre::StringConverter::toString(m_playerCursor.m_locationX) +"," +
				Ogre::StringConverter::toString(m_playerCursor.m_locationY));
		if(m_playerCursor.m_locationY%2 != 0)
		{
			//LogManager::getSingletonPtr()->logMessage("Real Location: "
			//+ Ogre::StringConverter::toString(unitX*1.732-.866) +"," +
			//Ogre::StringConverter::toString(-unitY*1.5));
			m_playerCursor.m_unitNode->setPosition(Ogre::Vector3(
					m_playerCursor.m_locationX*1.732-.866,0,-m_playerCursor.m_locationY*1.5));
		}
		else
		{
			//LogManager::getSingletonPtr()->logMessage("Real Location: " + Ogre::StringConverter::toString(unitX*1.732) +"," + Ogre::StringConverter::toString(-unitY*1.5));
			m_playerCursor.m_unitNode->setPosition(Ogre::Vector3(
					m_playerCursor.m_locationX*1.732,0,-m_playerCursor.m_locationY*1.5));
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

	OgreFramework::getSingletonPtr()->m_pGUISystem->injectMouseWheelChange(evt.state.Z.rel);
	OgreFramework::getSingletonPtr()->m_pGUISystem->injectMouseMove(evt.state.X.rel, evt.state.Y.rel);

	if(m_bRMouseDown)
	{
		m_pCamera->yaw(Degree(evt.state.X.rel * -0.1f));
		m_pCamera->pitch(Degree(evt.state.Y.rel * -0.1f));
	}

	return true;
}

bool GameState::mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{
	//if(OgreFramework::getSingletonPtr()->m_pTrayMgr->injectMouseDown(evt, id)) return true;

	if(id == OIS::MB_Left)
	{
		onLeftPressed(evt);
		m_bLMouseDown = true;

		OgreFramework::getSingletonPtr()->m_pGUISystem->injectMouseButtonDown(CEGUI::LeftButton);
	}
	else if(id == OIS::MB_Right)
	{
		m_bRMouseDown = true;
	}

	return true;
}

bool GameState::mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{
	//if(OgreFramework::getSingletonPtr()->m_pTrayMgr->injectMouseUp(evt, id)) return true;

	if(id == OIS::MB_Left)
	{
		m_bLMouseDown = false;
		OgreFramework::getSingletonPtr()->m_pGUISystem->injectMouseButtonUp(CEGUI::LeftButton);
	}
	else if(id == OIS::MB_Right)
	{
		m_bRMouseDown = false;
	}

	return true;
}

void GameState::onLeftPressed(const OIS::MouseEvent &evt)
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

bool GameState::onExitButtonGame(const CEGUI::EventArgs &args)
{
	m_bQuit = true;
	return true;
}

void GameState::moveCamera()
{
    if(OgreFramework::getSingletonPtr()->m_pKeyboard->isKeyDown(OIS::KC_LSHIFT))
    {
        m_pCamera->moveRelative(m_TranslateVector);
    }
    m_pCamera->moveRelative(m_TranslateVector / 10);
}

void GameState::getInput()
{
    if(m_bSettingsMode == false)
    {
        if(OgreFramework::getSingletonPtr()->m_pKeyboard->isKeyDown(OIS::KC_A))
            m_TranslateVector.x = -m_MoveScale;

        if(OgreFramework::getSingletonPtr()->m_pKeyboard->isKeyDown(OIS::KC_D))
            m_TranslateVector.x = m_MoveScale;

        if(OgreFramework::getSingletonPtr()->m_pKeyboard->isKeyDown(OIS::KC_W))
            m_TranslateVector.z = -m_MoveScale;

        if(OgreFramework::getSingletonPtr()->m_pKeyboard->isKeyDown(OIS::KC_S))
            m_TranslateVector.z = m_MoveScale;

        if(OgreFramework::getSingletonPtr()->m_pKeyboard->isKeyDown(OIS::KC_Q))
            m_TranslateVector.y = -m_MoveScale;

        if(OgreFramework::getSingletonPtr()->m_pKeyboard->isKeyDown(OIS::KC_E))
            m_TranslateVector.y = m_MoveScale;

        //camera roll
        if(OgreFramework::getSingletonPtr()->m_pKeyboard->isKeyDown(OIS::KC_Z))
            m_pCamera->roll(Angle(-m_MoveScale));

        if(OgreFramework::getSingletonPtr()->m_pKeyboard->isKeyDown(OIS::KC_X))
            m_pCamera->roll(Angle(m_MoveScale));

        //reset roll
        if(OgreFramework::getSingletonPtr()->m_pKeyboard->isKeyDown(OIS::KC_C))
            m_pCamera->roll(-(m_pCamera->getRealOrientation().getRoll()));
    }
}

void GameState::update(double timeSinceLastFrame)
{
	m_FrameEvent.timeSinceLastFrame = timeSinceLastFrame;
	//OgreFramework::getSingletonPtr()->m_pTrayMgr->frameRenderingQueued(m_FrameEvent);

	if(m_bQuit == true)
	{
		popAppState();
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

	m_MoveScale = m_MoveSpeed   * timeSinceLastFrame;
	m_RotScale  = m_RotateSpeed * timeSinceLastFrame;

	m_TranslateVector = Vector3::ZERO;

	getInput();
	moveCamera();
}

void GameState::buildGUI()
{

//	OgreFramework::getSingletonPtr()->m_pGUIRenderer->setTargetSceneManager(m_pSceneMgr);

	OgreFramework::getSingletonPtr()->m_pGUISystem->setDefaultMouseCursor((CEGUI::utf8*)"TaharezLook", (CEGUI::utf8*)"MouseArrow");
	CEGUI::MouseCursor::getSingleton().setImage("TaharezLook", "MouseArrow");
	const OIS::MouseState state = OgreFramework::getSingletonPtr()->m_pMouse->getMouseState();
	CEGUI::Point mousePos = CEGUI::MouseCursor::getSingleton().getPosition();
	CEGUI::System::getSingleton().injectMouseMove(state.X.abs-mousePos.d_x,state.Y.abs-mousePos.d_y);

	m_pMainWnd = CEGUI::WindowManager::getSingleton().getWindow("AOF_GUI_GAME");
	m_pChatWnd = CEGUI::WindowManager::getSingleton().getWindow("ChatWnd");

	OgreFramework::getSingletonPtr()->m_pGUISystem->setGUISheet(m_pMainWnd);

	CEGUI::PushButton* pExitButton = (CEGUI::PushButton*)m_pMainWnd->getChild("ExitButton_Game");
	pExitButton->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&GameState::onExitButtonGame, this));

	m_bLMouseDown = m_bRMouseDown = false;
	m_bQuit = false;
	m_bChatMode = false;

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

void GameState::setBufferedMode()
{
	CEGUI::Editbox* pModeCaption = (CEGUI::Editbox*)m_pMainWnd->getChild("ModeCaption");
	pModeCaption->setText("Buffered Input Mode");

	CEGUI::Editbox* pChatInputBox = (CEGUI::Editbox*)m_pChatWnd->getChild("ChatInputBox");
	pChatInputBox->setText("");
	pChatInputBox->activate();
	pChatInputBox->captureInput();

	CEGUI::MultiLineEditbox* pControlsPanel = (CEGUI::MultiLineEditbox*)m_pMainWnd->getChild("ControlsPanel");
	pControlsPanel->setText("[Tab] - To switch between input modes\n\nAll keys to write in the chat box.\n\nPress [Enter] or [Return] to send message.\n\n[Print] - Take screenshot\n\n[Esc] - Quit to main menu");
}

void GameState::setUnbufferedMode()
{
	CEGUI::Editbox* pModeCaption = (CEGUI::Editbox*)m_pMainWnd->getChild("ModeCaption");
	pModeCaption->setText("Unuffered Input Mode");

	CEGUI::MultiLineEditbox* pControlsPanel = (CEGUI::MultiLineEditbox*)m_pMainWnd->getChild("ControlsPanel");
	pControlsPanel->setText("[Tab] - To switch between input modes\n\n[W] - Forward\n[S] - Backwards\n[A] - Left\n[D] - Right\n\nPress [Shift] to move faster\n\n[O] - Toggle Overlays\n[Print] - Take screenshot\n\n[Esc] - Quit to main menu");
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

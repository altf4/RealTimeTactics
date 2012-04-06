//============================================================================
// Name        : RTT_Ogre_Game.cpp
// Author      : Nysomin
// Copyright   : 2012, GNU GPLv3
// Description : Ogre 3D rendering engine game file for RealTimeTactics
//               Based on Ogre Tutorial framework
//============================================================================

#include "RTT_Ogre_Game.h"

#include <string>
#include <vector>

using namespace Ogre;
using namespace std;
using namespace OIS;
using namespace RTT;

//-------------------------------------------------------------------------------------
RTT_Ogre_Game::RTT_Ogre_Game(void):
		m_mainPlayer(),
		m_playerCursor()
{
}
//-------------------------------------------------------------------------------------
RTT_Ogre_Game::~RTT_Ogre_Game(void)
{
}

//-------------------------------------------------------------------------------------
void RTT_Ogre_Game::moveUnit(RTT_Ogre_Unit& toMove)
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

void RTT_Ogre_Game::makeMove(RTT_Ogre_Unit& toMove)
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

//Algorithm: Iterate over each row in two phases (top and bottom)
//	The "pivot point" is the leftmost valid point in each row
//	Each row has exactly "radius" points in it minus one for each row away from zero
//	We reposition the pivot point at the end of each row
void RTT_Ogre_Game::showRange(RTT_Ogre_Unit& toShow, bool& value)
{
	int radius = toShow.m_moveRange;
	cout << "Currently at: " << toShow.m_locationX << ", " << toShow.m_locationY << endl;
	cout << "Radius: " << radius << endl;

	int pivotX = toShow.m_locationX - radius;
	int pivotY = toShow.m_locationY;

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

void RTT_Ogre_Game::faceUnit(RTT_Ogre_Unit& toFace)
{
	/*
	Ogre::Vector3 playerDirChange = toFace.m_unitNode->getPosition();
	int facingDirection = 0;

	switch(moveDirection)
	{
	case EAST:
		LogManager::getSingletonPtr()->logMessage("Moving: East");
		//toMove.m_locationX++;
		facingDirection = 90;
		break;
	case NORTHWEST:
		LogManager::getSingletonPtr()->logMessage("Moving: North West");
		//if(toMove.m_locationY%2 != 0)
		//	toMove.m_locationX--;
		//toMove.m_locationY++;
		facingDirection = -150;
		break;
	case NORTHEAST:
		LogManager::getSingletonPtr()->logMessage("Moving: North East");
		//if(toMove.m_locationY%2 == 0)
		//	toMove.m_locationX++;
		//toMove.m_locationY++;
		facingDirection = 150;
		break;
	case WEST:
		LogManager::getSingletonPtr()->logMessage("Moving: West");
		//toMove.m_locationX--;
		facingDirection = -90;
		break;
	case SOUTHWEST:
		LogManager::getSingletonPtr()->logMessage("Moving: South West");
		//if(toMove.m_locationY%2 != 0)
		//	toMove.m_locationX--;
		//toMove.m_locationY--;
		facingDirection = -30;
		break;
	case SOUTHEAST:
		LogManager::getSingletonPtr()->logMessage("Moving: South East");
		//if(toMove.m_locationY%2 == 0)
		//	toMove.m_locationX++;
		//toMove.m_locationY--;
		facingDirection = 30;
		break;
	default:
		break;
	}

	LogManager::getSingletonPtr()->logMessage("Location: " +
			Ogre::StringConverter::toString(toMove.m_locationX) +"," +
			Ogre::StringConverter::toString(toMove.m_locationY));
	if(toMove.m_locationY%2 != 0)
	{
		//LogManager::getSingletonPtr()->logMessage("Real Location: " + Ogre::StringConverter::toString(unitX*1.732-.866) +"," + Ogre::StringConverter::toString(-unitY*1.5));
		toMove.m_unitNode->setPosition(Ogre::Vector3(toMove.m_locationX*1.732-.866,0,-toMove.m_locationY*1.5));
	}
	else
	{
		//LogManager::getSingletonPtr()->logMessage("Real Location: " + Ogre::StringConverter::toString(unitX*1.732) +"," + Ogre::StringConverter::toString(-unitY*1.5));
		toMove.m_unitNode->setPosition(Ogre::Vector3(toMove.m_locationX*1.732,0,-toMove.m_locationY*1.5));
	}
	toMove.m_unitNode->resetOrientation();
	toMove.m_unitNode->yaw(Degree(facingDirection));//Make sure we are facing the right way
	*/
}

void RTT_Ogre_Game::moveCursor(const RTT::Direction& moveDirection)
{
	if(m_isMoving)
	{
		Ogre::Vector3 playerDirChange = m_playerCursor.m_unitNode->getPosition();
		int facingDirection = 0;
		switch(moveDirection)
		{
			case EAST:
				LogManager::getSingletonPtr()->logMessage("Moving: East");
				m_playerCursor.m_locationX++;
				facingDirection = 90;
				break;
			case NORTHWEST:
				LogManager::getSingletonPtr()->logMessage("Moving: North West");
				if(m_playerCursor.m_locationY%2 != 0)
					m_playerCursor.m_locationX--;
				m_playerCursor.m_locationY++;
				facingDirection = -150;
				break;
			case NORTHEAST:
				LogManager::getSingletonPtr()->logMessage("Moving: North East");
				if(m_playerCursor.m_locationY%2 == 0)
					m_playerCursor.m_locationX++;
				m_playerCursor.m_locationY++;
				facingDirection = 150;
				break;
			case WEST:
				LogManager::getSingletonPtr()->logMessage("Moving: West");
				m_playerCursor.m_locationX--;
				facingDirection = -90;
				break;
			case SOUTHWEST:
				LogManager::getSingletonPtr()->logMessage("Moving: South West");
				if(m_playerCursor.m_locationY%2 != 0)
					m_playerCursor.m_locationX--;
				m_playerCursor.m_locationY--;
				facingDirection = -30;
				break;
			case SOUTHEAST:
				LogManager::getSingletonPtr()->logMessage("Moving: South East");
				if(m_playerCursor.m_locationY%2 == 0)
					m_playerCursor.m_locationX++;
				m_playerCursor.m_locationY--;
				facingDirection = 30;
				break;
			default:
				break;
		}

		LogManager::getSingletonPtr()->logMessage("Location: " +
				Ogre::StringConverter::toString(m_playerCursor.m_locationX) +"," +
				Ogre::StringConverter::toString(m_playerCursor.m_locationY));
		if(m_playerCursor.m_locationY%2 != 0)
		{
			//LogManager::getSingletonPtr()->logMessage("Real Location: " + Ogre::StringConverter::toString(unitX*1.732-.866) +"," + Ogre::StringConverter::toString(-unitY*1.5));
			m_playerCursor.m_unitNode->setPosition(Ogre::Vector3(m_playerCursor.m_locationX*1.732-.866,0,-m_playerCursor.m_locationY*1.5));
		}
		else
		{
			//LogManager::getSingletonPtr()->logMessage("Real Location: " + Ogre::StringConverter::toString(unitX*1.732) +"," + Ogre::StringConverter::toString(-unitY*1.5));
			m_playerCursor.m_unitNode->setPosition(Ogre::Vector3(m_playerCursor.m_locationX*1.732,0,-m_playerCursor.m_locationY*1.5));
		}
		m_playerCursor.m_unitNode->resetOrientation();
		m_playerCursor.m_unitNode->yaw(Degree(facingDirection));//Make sure we are facing the right way
	}
}

//Buffered keyboard input and game keybindings
bool RTT_Ogre_Game::keyPressed( const KeyEvent& evt )
{
	switch (evt.key)
	{
	case KC_ESCAPE:
		LogManager::getSingletonPtr()->logMessage("Quitting!!!");
	    rttShutDown = true;
	    break;
	case KC_NUMPAD7://Move North West
		moveCursor(NORTHWEST);
		break;
	case KC_NUMPAD4://Move North EDIT::::::::WEST!
		moveCursor(WEST);
		break;
	case KC_NUMPAD9://Move North East
		moveCursor(NORTHEAST);
		break;
	case KC_NUMPAD1://Move South West
		moveCursor(SOUTHWEST);
		break;
	case KC_NUMPAD6://Move South  EDIT:::::::::EAST!
		moveCursor(EAST);
		break;
	case KC_NUMPAD3://Move South East
		moveCursor(SOUTHEAST);
		break;
	case KC_M: //Move 'dialog'
		if(!m_isMoving)
		{
			moveUnit(m_mainPlayer.ogreUnits);
			break;
		}
		else
			makeMove(m_mainPlayer.ogreUnits);
		break;
	case KC_F: //Facing 'dialog'
		m_isMoving = false;
		showRange(m_mainPlayer.ogreUnits, m_isMoving);
		break;
	default:
	    break;
	}
	return true;
}

void RTT_Ogre_Game::buildUnits(void)
{

	//mainPlayer.ogreUnits[0] = new RTT_Ogre_Unit();

	m_mainPlayer.ogreUnits.m_unitEntity = m_rttSceneManager->createEntity("BlueMarine", "ColorMarine.mesh");

	m_mainPlayer.ogreUnits.m_unitEntity->setCastShadows(true);
	m_mainPlayer.ogreUnits.m_unitEntity->setMaterialName("BlueMarine");
	m_mainPlayer.ogreUnits.m_unitNode = m_rttSceneManager->getRootSceneNode()->createChildSceneNode("BlueMarine");
	m_mainPlayer.ogreUnits.m_unitNode->attachObject(m_mainPlayer.ogreUnits.m_unitEntity);
	m_mainPlayer.ogreUnits.m_unitNode->yaw(Degree(150));
	m_mainPlayer.ogreUnits.m_locationX = 0;
	m_mainPlayer.ogreUnits.m_locationY = 0;
	m_mainPlayer.ogreUnits.m_moveRange = 3;

	m_playerCursor.m_unitEntity = m_rttSceneManager->createEntity("Cursor", "Marker.mesh");
	m_playerCursor.m_unitEntity->setMaterialName("Marker");
	m_playerCursor.m_unitNode = m_rttSceneManager->getRootSceneNode()->createChildSceneNode("Cursor");
	m_playerCursor.m_unitNode->attachObject(m_playerCursor.m_unitEntity);
	m_playerCursor.m_locationX = 0;
	m_playerCursor.m_locationY = 0;
	m_playerCursor.m_unitNode->setVisible(false);
}

void RTT_Ogre_Game::createScene(void)
{
	// Set ambient light and shadows
    m_rttSceneManager->setAmbientLight(ColourValue(0, 0, 0));
    m_rttSceneManager->setShadowTechnique(SHADOWTYPE_TEXTURE_ADDITIVE);
    //Shadowmaps	EXPERIMENTAL
    m_rttSceneManager->setShadowTexturePixelFormat(Ogre::PF_FLOAT32_R);
    m_rttSceneManager->setShadowTextureSelfShadow(true);
    //rttSceneManager->setShadowCasterRenderBackFaces(false);
    m_rttSceneManager->setShadowTextureCasterMaterial("Ogre/DepthShadowmap/Caster/Float");
    //rttSceneManager->setShadowTextureReceiverMaterial("Ogre/DepthShadowmap/BasicTemplateMaterial");
    m_rttSceneManager->setShadowTextureSize(1024);
    m_isMoving = false;
    buildUnits();

    //HACKED  Proof of concept/scratchboard

    int tileSize = 1;
    float posX = 0;
    float posY = 0;
    float posZ = 0;
    Entity* tileVector[8][8];
    string tileType = "DirtTile";
    SceneNode* nodeVector[8][8];

    Entity* rangeMarker[8][8];
    m_mainPlayer.rangeNode[8][8];
    string rangeType = "Range";

    //Entity* blueMarine = rttSceneManager->createEntity("BlueMarine", "BlueMarine.mesh");
    //blueMarine->setCastShadows(true);
    //SceneNode* blueMarineNode = rttSceneManager->getRootSceneNode()->createChildSceneNode("BlueMarine");
    //blueMarineNode->attachObject(blueMarine);
    //blueMarineNode->yaw(Degree(90));
    Entity* redMarine = m_rttSceneManager->createEntity("RedMarine", "ColorMarine.mesh");
    redMarine->setCastShadows(true);
    redMarine->setMaterialName("RedMarine");
    SceneNode* redMarineNode = m_rttSceneManager->getRootSceneNode()->createChildSceneNode("RedMarine", Ogre::Vector3(7*1.732 -.866,0,-7*1.5));
    redMarineNode->attachObject(redMarine);
    redMarineNode->yaw(Degree(-30));

    for(int x=0;x < 8*tileSize; x+=tileSize)//build our columns
    {
    	for(int y=0; y<8*tileSize;y+=tileSize)//build our rows
    	{
    		posX = x *1.732;
    		if(y%2 != 0)//test for odd
    		{
    			posX -= .866;  //steps over
    		}
    		tileVector[x][y] = m_rttSceneManager->createEntity(tileType + Ogre::StringConverter::toString(x) + Ogre::StringConverter::toString(y), "DirtTile.mesh", "RTT");
    		tileVector[x][y]->setCastShadows(true);
    		nodeVector[x][y] = m_rttSceneManager->getRootSceneNode()->createChildSceneNode(tileType + Ogre::StringConverter::toString(x) + Ogre::StringConverter::toString(y), Ogre::Vector3(posX, posY, -y*1.5));
    		nodeVector[x][y]->attachObject(tileVector[x][y]);

    		rangeMarker[x][y] = m_rttSceneManager->createEntity(rangeType + Ogre::StringConverter::toString(x) + Ogre::StringConverter::toString(y), "Range.mesh", "RTT");
    		rangeMarker[x][y]->setMaterialName("Range");
    		m_mainPlayer.rangeNode[x][y] = m_rttSceneManager->getRootSceneNode()->createChildSceneNode(rangeType + Ogre::StringConverter::toString(x) + Ogre::StringConverter::toString(y), Ogre::Vector3(posX, posY, -y*1.5));
    		m_mainPlayer.rangeNode[x][y]->attachObject(rangeMarker[x][y]);
    		m_mainPlayer.rangeNode[x][y]->yaw(Degree(90));
    		m_mainPlayer.rangeNode[x][y]->setVisible(false);
    	}
    }
/*
	Entity* hescoTest = rttSceneManager->createEntity("Hesco", "Hesco.mesh");
	hescoTest->setCastShadows(true);
	SceneNode* hescoNode = rttSceneManager->getRootSceneNode()->createChildSceneNode("Hesco", Ogre::Vector3(2*1.732 -.866,0,-1*1.5));
	hescoNode->attachObject(hescoTest);
	hescoNode->yaw(Degree(90));
	hescoTest = rttSceneManager->createEntity("Hesco2", "Hesco.mesh");
	hescoTest->setCastShadows(true);
	hescoNode = rttSceneManager->getRootSceneNode()->createChildSceneNode("Hesco2", Ogre::Vector3(3*1.732 -.866,0,-1*1.5));
	hescoNode->attachObject(hescoTest);
	hescoNode->yaw(Degree(90));
	hescoTest = rttSceneManager->createEntity("Hesco3", "Hesco.mesh");
	hescoTest->setCastShadows(true);
	hescoNode = rttSceneManager->getRootSceneNode()->createChildSceneNode("Hesco3", Ogre::Vector3(4*1.732 -.866,0,-1*1.5));
	hescoNode->attachObject(hescoTest);
	hescoNode->yaw(Degree(90));

	hescoTest = rttSceneManager->createEntity("Hesco4", "Hesco.mesh");
	hescoTest->setCastShadows(true);
	hescoNode = rttSceneManager->getRootSceneNode()->createChildSceneNode("Hesco4", Ogre::Vector3(5*1.732 ,0,-6*1.5));
	hescoNode->attachObject(hescoTest);
	hescoNode->yaw(Degree(30));
	hescoTest = rttSceneManager->createEntity("Hesco5", "Hesco.mesh");
	hescoTest->setCastShadows(true);
	hescoNode = rttSceneManager->getRootSceneNode()->createChildSceneNode("Hesco5", Ogre::Vector3(6*1.732 -.866,0,-5*1.5));
	hescoNode->attachObject(hescoTest);
	hescoNode->yaw(Degree(30));
	hescoTest = rttSceneManager->createEntity("Hesco6", "Hesco.mesh");
	hescoTest->setCastShadows(true);
	hescoNode = rttSceneManager->getRootSceneNode()->createChildSceneNode("Hesco6", Ogre::Vector3(6*1.732 ,0,-4*1.5));
	hescoNode->attachObject(hescoTest);
	hescoNode->yaw(Degree(30));
*/
    //end HACK
    Entity* groundPlane = m_rttSceneManager->createEntity("Ground", "Plane.mesh");
    groundPlane->setMaterialName("Claygreen");
    groundPlane->setCastShadows(false);
    SceneNode* groundPlaneNode = m_rttSceneManager->getRootSceneNode()->createChildSceneNode("Ground", Ogre::Vector3(2.25,0,0));
    groundPlaneNode->attachObject(groundPlane);
    //groundPlaneNode->scale(25,25,25);

    // Create a light
    Light* mainLight = m_rttSceneManager->createLight("MainLight");
    mainLight->setType(Light::LT_POINT);
    //mainLight->mCastShadows=true;
    mainLight->setPosition(20,30,15);
    mainLight->setCastShadows(true);

    //END OBJECTS       ************************************************************************************************
}

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
    int main(int argc, char *argv[])
#endif
    {
        // Create application object
        RTT_Ogre_Game app;

        try {
            app.go();
        } catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
            MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
            std::cerr << "An exception has occured: " <<
                e.getFullDescription().c_str() << std::endl;
#endif
        }

        return 0;
    }

#ifdef __cplusplus
}
#endif

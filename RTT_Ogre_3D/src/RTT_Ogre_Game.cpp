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
		m_mainPlayer()
{
}
//-------------------------------------------------------------------------------------
RTT_Ogre_Game::~RTT_Ogre_Game(void)
{
}

//-------------------------------------------------------------------------------------
void RTT_Ogre_Game::moveUnit(const RTT::Direction& moveDirection, RTT_Ogre_Unit& toMove)
{
	Ogre::Vector3 playerLocChange = toMove.m_unitNode->getPosition();
	int facingDirection = 0;
	switch(moveDirection)
	{
	case EAST:
		LogManager::getSingletonPtr()->logMessage("Moving: East");
		toMove.m_locationX++;
		facingDirection = 90;
		break;
	case NORTHWEST:
		LogManager::getSingletonPtr()->logMessage("Moving: North West");
		if(toMove.m_locationY%2 != 0)
			toMove.m_locationX--;
		toMove.m_locationY++;
		facingDirection = -150;
		break;
	case NORTHEAST:
		LogManager::getSingletonPtr()->logMessage("Moving: North East");
		if(toMove.m_locationY%2 == 0)
			toMove.m_locationX++;
		toMove.m_locationY++;
		facingDirection = 150;
		break;
	case WEST:
		LogManager::getSingletonPtr()->logMessage("Moving: West");
		toMove.m_locationX--;
		facingDirection = -90;
		break;
	case SOUTHWEST:
		LogManager::getSingletonPtr()->logMessage("Moving: South West");
		if(toMove.m_locationY%2 != 0)
			toMove.m_locationX--;
		toMove.m_locationY--;
		facingDirection = -30;
		break;
	case SOUTHEAST:
		LogManager::getSingletonPtr()->logMessage("Moving: South East");
		if(toMove.m_locationY%2 == 0)
			toMove.m_locationX++;
		toMove.m_locationY--;
		facingDirection = 30;
		break;
	default:
		break;
	}

	LogManager::getSingletonPtr()->logMessage("Location: " + Ogre::StringConverter::toString(toMove.m_locationX) +"," + Ogre::StringConverter::toString(toMove.m_locationY));
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
		moveUnit(NORTHWEST, m_mainPlayer);
		break;
	case KC_NUMPAD4://Move North EDIT::::::::WEST!
		moveUnit(WEST, m_mainPlayer);
		break;
	case KC_NUMPAD9://Move North East
		moveUnit(NORTHEAST, m_mainPlayer);
		break;
	case KC_NUMPAD1://Move South West
		moveUnit(SOUTHWEST, m_mainPlayer);
		break;
	case KC_NUMPAD6://Move South  EDIT:::::::::EAST!
		moveUnit(EAST, m_mainPlayer);
		break;
	case KC_NUMPAD3://Move South East
		moveUnit(SOUTHEAST, m_mainPlayer);
		break;
	default:
	    break;
	}
	return true;
}

void RTT_Ogre_Game::buildUnits(void)
{
    m_mainPlayer.m_unitEntity = m_rttSceneManager->createEntity("BlueMarine", "BlueMarine.mesh");
    m_mainPlayer.m_unitEntity->setCastShadows(true);
    m_mainPlayer.m_unitNode = m_rttSceneManager->getRootSceneNode()->createChildSceneNode("BlueMarine");
    m_mainPlayer.m_unitNode->attachObject(m_mainPlayer.m_unitEntity);
    m_mainPlayer.m_unitNode->yaw(Degree(150));
    m_mainPlayer.m_locationX = 0;
    m_mainPlayer.m_locationY = 0;
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

    buildUnits();

    //HACKED  Proof of concept/scratchboard

    int tileSize = 1;
    float posX = 0;
    float posY = 0;
    float posZ = 0;
    Entity* tileVector[4][4];
    string tileType = "DirtTile";
    SceneNode* nodeVector[4][4];
    //Entity* blueMarine = rttSceneManager->createEntity("BlueMarine", "BlueMarine.mesh");
    //blueMarine->setCastShadows(true);
    //SceneNode* blueMarineNode = rttSceneManager->getRootSceneNode()->createChildSceneNode("BlueMarine");
    //blueMarineNode->attachObject(blueMarine);
    //blueMarineNode->yaw(Degree(90));
    Entity* redMarine = m_rttSceneManager->createEntity("RedMarine", "RedMarine.mesh");
    redMarine->setCastShadows(true);
    SceneNode* redMarineNode = m_rttSceneManager->getRootSceneNode()->createChildSceneNode("RedMarine", Ogre::Vector3(3*1.732 -.866,0,-3*1.5));
    redMarineNode->attachObject(redMarine);
    redMarineNode->yaw(Degree(-30));

    for(int x=0;x < 4*tileSize; x+=tileSize)//build our columns
    {
    	for(int y=0; y<4*tileSize;y+=tileSize)//build our rows
    	{
    		posX = x *1.732;
    		if(y%2 != 0)//test for odd
    		{
    			posX -= .866; //steps over
    		}
    		tileVector[x][y] = m_rttSceneManager->createEntity(tileType + Ogre::StringConverter::toString(x) + Ogre::StringConverter::toString(y), "DirtTile.mesh", "RTT");
    		tileVector[x][y]->setCastShadows(true);
    		nodeVector[x][y] = m_rttSceneManager->getRootSceneNode()->createChildSceneNode(tileType + Ogre::StringConverter::toString(x) + Ogre::StringConverter::toString(y), Ogre::Vector3(posX, posY, -y*1.5));
    		nodeVector[x][y]->attachObject(tileVector[x][y]);
    	}
    }

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

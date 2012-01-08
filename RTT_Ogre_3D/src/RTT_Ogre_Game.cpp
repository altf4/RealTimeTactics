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
		mainPlayer(0),
		mainPlayerNode(0)
{
}
//-------------------------------------------------------------------------------------
RTT_Ogre_Game::~RTT_Ogre_Game(void)
{
}

//-------------------------------------------------------------------------------------
void RTT_Ogre_Game::moveCharacter(const RTT::Direction& moveDirection)
{
	Ogre::Vector3 playerLocChange = mainPlayerNode->getPosition();
	int facingDirection = 0;
	switch(moveDirection)
	{
	case EAST:
		unitX++;
		facingDirection = 90;
		break;
	case NORTHWEST:
		unitX--;
		unitY++;
		facingDirection = -150;
		break;
	case NORTHEAST:
		unitX++;
		unitY++;
		facingDirection = 150;
		break;
	case WEST:
		unitX--;
		facingDirection = -90;
		break;
	case SOUTHWEST:
		unitX--;
		unitY--;
		facingDirection = -30;
		break;
	case SOUTHEAST:
		unitX++;
		unitY--;
		facingDirection = 30;
		break;
	default:
		break;
	}

	LogManager::getSingletonPtr()->logMessage("Moving: " + Ogre::StringConverter::toString(unitX) +"," + Ogre::StringConverter::toString(unitY));
	if(unitY%2 != 0)
	{
		mainPlayerNode->setPosition(Ogre::Vector3(unitX*1.732-.866,0,-unitY*1.5));
	}
	else
		mainPlayerNode->setPosition(Ogre::Vector3(unitX*1.732,0,-unitY*1.5));

	mainPlayerNode->resetOrientation();
	mainPlayerNode->yaw(Degree(facingDirection));

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
		moveCharacter(NORTHWEST);
		break;
	case KC_NUMPAD4://Move North EDIT::::::::WEST!
		moveCharacter(WEST);
		break;
	case KC_NUMPAD9://Move North East
		moveCharacter(NORTHEAST);
		break;
	case KC_NUMPAD1://Move South West
		moveCharacter(SOUTHWEST);
		break;
	case KC_NUMPAD6://Move South  EDIT:::::::::EAST!
		moveCharacter(EAST);
		break;
	case KC_NUMPAD3://Move South East
		moveCharacter(SOUTHEAST);
		break;
	default:
	    break;
	}
	return true;
}

void RTT_Ogre_Game::buildPlayers(void)
{
    mainPlayer = rttSceneManager->createEntity("BlueMarine", "BlueMarine.mesh");
    mainPlayer->setCastShadows(true);
    mainPlayerNode = rttSceneManager->getRootSceneNode()->createChildSceneNode("BlueMarine");
    mainPlayerNode->attachObject(mainPlayer);
    mainPlayerNode->yaw(Degree(150));
    unitX = 0;
    unitY = 0;
}

void RTT_Ogre_Game::createScene(void)
{
	// Set ambient light and shadows
    rttSceneManager->setAmbientLight(ColourValue(0, 0, 0));
    rttSceneManager->setShadowTechnique(SHADOWTYPE_TEXTURE_ADDITIVE);
    //Shadowmaps	EXPERIMENTAL  **********BROKEN************

    rttSceneManager->setShadowTexturePixelFormat(Ogre::PF_FLOAT32_R);
    rttSceneManager->setShadowTextureSelfShadow(true);
    //rttSceneManager->setShadowCasterRenderBackFaces(false);
    rttSceneManager->setShadowTextureCasterMaterial("Ogre/DepthShadowmap/Caster/Float");
    //rttSceneManager->setShadowTextureReceiverMaterial("Ogre/DepthShadowmap/BasicTemplateMaterial");
    rttSceneManager->setShadowTextureSize(1024);

    buildPlayers();

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
    Entity* redMarine = rttSceneManager->createEntity("RedMarine", "RedMarine.mesh");
    redMarine->setCastShadows(true);
    SceneNode* redMarineNode = rttSceneManager->getRootSceneNode()->createChildSceneNode("RedMarine", Ogre::Vector3(3*1.732 -.866,0,-3*1.5));
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
    		tileVector[x][y] = rttSceneManager->createEntity(tileType + Ogre::StringConverter::toString(x) + Ogre::StringConverter::toString(y), "DirtTile.mesh", "RTT");
    		tileVector[x][y]->setCastShadows(true);
    		nodeVector[x][y] = rttSceneManager->getRootSceneNode()->createChildSceneNode(tileType + Ogre::StringConverter::toString(x) + Ogre::StringConverter::toString(y), Ogre::Vector3(posX, posY, -y*1.5));
    		nodeVector[x][y]->attachObject(tileVector[x][y]);
    	}
    }

	/*
	    Entity* dirtTile = rttSceneManager->createEntity("DirtTile1", "DirtTile.mesh");
	    dirtTile->setCastShadows(true);
	    SceneNode* dirtTileNode = rttSceneManager->getRootSceneNode()->createChildSceneNode("DirtTile1");
	    dirtTileNode->attachObject(dirtTile);

	    Entity* dirtTile2 = rttSceneManager->createEntity("DirtTile2", "DirtTile.mesh");
	    dirtTile2->setCastShadows(true);
	    SceneNode* dirtTile2Node = rttSceneManager->getRootSceneNode()->createChildSceneNode("DirtTile2", Ogre::Vector3(1.5,0,-.866));
	    dirtTile2Node->attachObject(dirtTile2);

	    Entity* dirtTile3 = rttSceneManager->createEntity("DirtTile3", "DirtTile.mesh");
	    dirtTile3->setCastShadows(true);
	    SceneNode* dirtTile3Node = rttSceneManager->getRootSceneNode()->createChildSceneNode("DirtTile3", Ogre::Vector3(3,0,0));
	    dirtTile3Node->attachObject(dirtTile3);

	    Entity* blueMarine = rttSceneManager->createEntity("BlueMarine", "BlueMarine.mesh");
	    blueMarine->setCastShadows(true);
	    SceneNode* blueMarineNode = rttSceneManager->getRootSceneNode()->createChildSceneNode("BlueMarine");
	    blueMarineNode->attachObject(blueMarine);
	*/

    //end HACK
    Entity* groundPlane = rttSceneManager->createEntity("Ground", "Plane.mesh");
    groundPlane->setMaterialName("Claygreen");
    groundPlane->setCastShadows(false);
    SceneNode* groundPlaneNode = rttSceneManager->getRootSceneNode()->createChildSceneNode("Ground", Ogre::Vector3(2.25,0,0));
    groundPlaneNode->attachObject(groundPlane);
    //groundPlaneNode->scale(25,25,25);


    // Create a light
    Light* mainLight = rttSceneManager->createLight("MainLight");
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

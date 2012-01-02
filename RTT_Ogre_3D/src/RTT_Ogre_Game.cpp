//============================================================================
// Name        : RTT_Ogre_Game.cpp
// Author      : Nysomin
// Copyright   : 2012, GNU GPLv3
// Description : Ogre 3D rendering engine game file for RealTimeTactics
//               Based on Ogre Tutorial framework
//============================================================================

#include "RTT_Ogre_Game.h"

#include <OgreRoot.h>
#include <OgreEntity.h>
#include <string>
#include <vector>

using namespace Ogre;
using namespace std;
using namespace OIS;
using namespace RTT;

//-------------------------------------------------------------------------------------
RTT_Ogre_Game::RTT_Ogre_Game(void)
{
}
//-------------------------------------------------------------------------------------
RTT_Ogre_Game::~RTT_Ogre_Game(void)
{
}

//-------------------------------------------------------------------------------------
void RTT_Ogre_Game::createScene(void)
{
	// Set ambient light and shadows
    rttSceneManager->setAmbientLight(ColourValue(0, 0, 0));
    rttSceneManager->setShadowTechnique(SHADOWTYPE_TEXTURE_ADDITIVE);
    //Shadowmaps	EXPERIMENTAL  **********BROKEN************
    /*
    rttSceneManager->setShadowTexturePixelFormat();
    rttSceneManager->setShadowTextureSelfShadow(true);
    rttSceneManager->setShadowTextureCasterMaterial("ShadowCaster");
    rttSceneManager->setShadowTextureReceiverMaterial("ShadowReceiver");
    */
    //HACKED  Proof of concept/scratchboard

    int tileSize = 1;
    float posX = 0;
    float posY = 0;
    float posZ = 0;
    Entity* tileVector[4][4];
    string tileType = "DirtTile";
    SceneNode* nodeVector[4][4];
    Entity* blueMarine = rttSceneManager->createEntity("BlueMarine", "BlueMarine.mesh");
    blueMarine->setCastShadows(true);
    SceneNode* blueMarineNode = rttSceneManager->getRootSceneNode()->createChildSceneNode("BlueMarine");
    blueMarineNode->attachObject(blueMarine);
    blueMarineNode->yaw(Degree(90));
    Entity* redMarine = rttSceneManager->createEntity("RedMarine", "RedMarine.mesh");
    redMarine->setCastShadows(true);
    SceneNode* redMarineNode = rttSceneManager->getRootSceneNode()->createChildSceneNode("RedMarine", Ogre::Vector3(3*1.5,0,-3*1.732 -.866));
    redMarineNode->attachObject(redMarine);
    redMarineNode->yaw(Degree(-90));

    for(int i=0;i < 4*tileSize; i+=tileSize)//build our columns
    {
    	for(int n=0; n<4*tileSize;n+=tileSize)//build our rows
    	{
    		posZ = -n *1.732;
    		if(i%2 != 0)//test for odd
    		{
    			posZ -= .866; //steps up
    		}
    		tileVector[i][n] = rttSceneManager->createEntity(tileType + Ogre::StringConverter::toString(i) + Ogre::StringConverter::toString(n), "DirtTile.mesh", "RTT");
    		tileVector[i][n]->setCastShadows(true);
    		nodeVector[i][n] = rttSceneManager->getRootSceneNode()->createChildSceneNode(tileType + Ogre::StringConverter::toString(i) + Ogre::StringConverter::toString(n), Ogre::Vector3(i*1.5, posY,posZ));
    		nodeVector[i][n]->attachObject(tileVector[i][n]);
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

    rttSceneManager->setSkyDome(true, "Examples/CloudySky", 2, 3);

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

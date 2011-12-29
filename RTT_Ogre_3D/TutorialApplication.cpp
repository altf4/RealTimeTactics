/*
-----------------------------------------------------------------------------
Filename:    TutorialApplication.cpp
-----------------------------------------------------------------------------

This source file is part of the
   ___                 __    __ _ _    _ 
  /___\__ _ _ __ ___  / / /\ \ (_) | _(_)
 //  // _` | '__/ _ \ \ \/  \/ / | |/ / |
/ \_// (_| | | |  __/  \  /\  /| |   <| |
\___/ \__, |_|  \___|   \/  \/ |_|_|\_\_|
      |___/                              
      Tutorial Framework
      http://www.ogre3d.org/tikiwiki/
-----------------------------------------------------------------------------
*/
#include "TutorialApplication.h"
#include "DotSceneLoader.h"

//-------------------------------------------------------------------------------------
TutorialApplication::TutorialApplication(void)
{
}
//-------------------------------------------------------------------------------------
TutorialApplication::~TutorialApplication(void)
{
}

//-------------------------------------------------------------------------------------
void TutorialApplication::createScene(void)
{
    // create your scene here :)
	/*hold
	Ogre::Entity* ogreBox = mSceneMgr->createEntity("Cube", "Rhino.mesh");

	Ogre::SceneNode* boxNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	boxNode->attachObject(ogreBox);

	boxNode->scale( 12, 12, 12 );

	Ogre::Entity* ogreBox2 = mSceneMgr->createEntity("Cube2", "Marine1.mesh");

	Ogre::SceneNode* boxNode2 = mSceneMgr->getRootSceneNode()->createChildSceneNode("boxNode2", Ogre::Vector3( 10, 0, 0 ));
	boxNode2->attachObject(ogreBox2);

	Ogre::Entity* ogreBox3 = mSceneMgr->createEntity("Cube3", "Marine2.mesh");

	Ogre::SceneNode* boxNode3 = mSceneMgr->getRootSceneNode()->createChildSceneNode("boxNode3", Ogre::Vector3( -10, 0, 0 ));
	boxNode3->attachObject(ogreBox3);

	Ogre::Entity* ogrePlane = mSceneMgr->createEntity("Plane", "Plane.mesh");

	Ogre::SceneNode* planeNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("planeNode", Ogre::Vector3( 0, 0, 0 ));
	planeNode->attachObject(ogrePlane);

	// Set ambient light
	mSceneMgr->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));

	// Create a light
	Ogre::Light* l = mSceneMgr->createLight("MainLight");
	l->setPosition(20,80,50);
	*/

	DotSceneLoader loader;
	loader.parseDotScene("First.scene","General",mSceneMgr);

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
        TutorialApplication app;

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

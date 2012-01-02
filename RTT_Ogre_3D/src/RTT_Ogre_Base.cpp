//============================================================================
// Name        : RTT_Ogre_Base.h
// Author      : Nysomin
// Copyright   : 2011, GNU GPLv3
// Description : Ogre 3D rendering engine base application for RealTimeTactics
//               Based on Ogre Tutorial framework
//============================================================================

#include "RTT_Ogre_Base.h"

//using namespace RTT;
using namespace Ogre;
using namespace OIS;

//-------------------------------------------------------------------------------------
RTT_Ogre_Base::RTT_Ogre_Base(void)
    : rttRoot(0),
    rttCamera(0),
    rttSceneManager(0),
    rttWindow(0),
    rttResourcesCfg(Ogre::StringUtil::BLANK),
    rttPluginsCfg(StringUtil::BLANK),
    rttShutDown(false),
    rttInputManager(0),
    rttMouse(0),
    rttKeyboard(0)
{
}
//-------------------------------------------------------------------------------------
RTT_Ogre_Base::~RTT_Ogre_Base(void)
{
	//Remove ourself as a Window listener
	WindowEventUtilities::removeWindowEventListener(rttWindow, this);
	windowClosed(rttWindow);
	//Kill Ogre
	delete rttRoot;
}

//Loads our configuration files or creates a new one
bool RTT_Ogre_Base::configure(void)
{
    // Show the configuration dialog if no configuration found and initialize the system
    if(!(rttRoot->restoreConfig() || rttRoot->showConfigDialog()))  // Test to see if the render window can/should be launched
    {
        return false;
    }
    rttWindow = rttRoot->initialise(true, "Real Time Tactics Render Window");
    return true;
}

//Setup our scene manager
void RTT_Ogre_Base::chooseSceneManager(void)
{
	rttSceneManager = rttRoot->createSceneManager("DefaultSceneManager"); //Create a generic screen manager
}

void RTT_Ogre_Base::createCamera(void)
{
    rttCamera = rttSceneManager->createCamera("PrimaryCamera"); //Create our primary camera in our screen manager
    //The camera needs positioning
    rttCamera->setPosition(Ogre::Vector3(2.25,10.5,6.5)); // Center on the gameboard and possition above
    rttCamera->lookAt(Ogre::Vector3(2.25,0,-4)); // Look back along -Z slightly for an "isometric" like view
    rttCamera->setNearClipDistance(5); // This is how close an object can be to the camera before it is "clipped", or not rendered
}

void RTT_Ogre_Base::createFrameListener(void)
{
    //OIS Input system startup
    LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");
    ParamList paramList;
    size_t windowHnd = 0;
    std::ostringstream windowHndStr;

    rttWindow->getCustomAttribute("WINDOW", &windowHnd);
    windowHndStr << windowHnd;
    paramList.insert(make_pair(std::string("WINDOW"), windowHndStr.str()));

    rttInputManager = InputManager::createInputSystem( paramList );
    rttKeyboard = static_cast<Keyboard*>(rttInputManager->createInputObject(OISKeyboard, true ));
    rttMouse = static_cast<Mouse*>(rttInputManager->createInputObject(OISMouse, true ));

    rttMouse->setEventCallback(this);
    rttKeyboard->setEventCallback(this);

    windowResized(rttWindow);    //Set initial mouse clipping size
    WindowEventUtilities::addWindowEventListener(rttWindow, this);    //Register as a Window listener

    rttRoot->addFrameListener(this);
}

void RTT_Ogre_Base::destroyScene(void)
{
	//nothing yet
}

//Create the viewport for our camera, the viewport is the rendered image, or "view" into our 3D world from a camera
void RTT_Ogre_Base::createViewports(void)
{
    //Create one viewport, entire window, this is where the camera view is rendered
    Viewport* viewPort = rttWindow->addViewport(rttCamera);
    viewPort->setBackgroundColour(ColourValue(0,0,0));
    //Alter the camera aspect ratio to match the viewport
    rttCamera->setAspectRatio(Real(viewPort->getActualWidth()) / Real(viewPort->getActualHeight()));
}

//Set up our resources
void RTT_Ogre_Base::setupResources(void)
{
    // Load resource paths from config file
    ConfigFile configFile;
    configFile.load(rttResourcesCfg);
    // Parse resource config file
    ConfigFile::SectionIterator seci = configFile.getSectionIterator();

    String secName, typeName, archName;
    while (seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
        ConfigFile::SettingsMultiMap *settings = seci.getNext();
        ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            typeName = i->first;
            archName = i->second;
            ResourceGroupManager::getSingleton().addResourceLocation(archName, typeName, secName);
        }
    }
}

void RTT_Ogre_Base::createResourceListener(void)
{
	//nothing yet
}

void RTT_Ogre_Base::loadResources(void)
{
	//Initialize all resource groups
	ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}

//--------------------------------------------GO--------------------------------------------
void RTT_Ogre_Base::go(void)
{
#ifdef _DEBUG
    rttResourcesCfg = "resources_d.cfg";
    rttPluginsCfg = "plugins_d.cfg";
#else
    rttResourcesCfg = "resources.cfg";
    rttPluginsCfg = "plugins.cfg";
#endif

    if (!setup())
        return;

    rttRoot->startRendering();

    // clean up
    destroyScene();
}
//--------------------------------------------GO--------------------------------------------

bool RTT_Ogre_Base::setup(void)
{
    rttRoot = new Root(rttPluginsCfg);// construct Ogre::Root root node
    setupResources();//Setup our game data
    //If our configuration fails then quit
    if (!configure())
    	return false;

    //Start making our scene
    chooseSceneManager();
    createCamera();
    createViewports();

    TextureManager::getSingleton().setDefaultNumMipmaps(5);//Set default mipmap level (note: some APIs ignore this)

    createResourceListener();//Create any resource listeners (for loading screens)
    loadResources();//Load the resources
    createScene();//Create the scene
    createFrameListener();//Create input frame listener

    return true;
}

//Adjust mouse clipping area
void RTT_Ogre_Base::windowResized(RenderWindow* renderWindow)
{
    unsigned int width, height, depth;
    int left, top;
    renderWindow->getMetrics(width, height, depth, left, top);

    const MouseState &mouseState = rttMouse->getMouseState();
    mouseState.width = width;
    mouseState.height = height;
}

//Unattach OIS before window shutdown (very important under Linux)
void RTT_Ogre_Base::windowClosed(RenderWindow* renderWindow)
{
    //Only close for window that created OIS
    if( renderWindow == rttWindow )
    {
        if( rttInputManager )
        {
            rttInputManager->destroyInputObject( rttMouse );
            rttInputManager->destroyInputObject( rttKeyboard );

            InputManager::destroyInputSystem(rttInputManager);
            rttInputManager = 0;
        }
    }
}

//Buffered keyboard input and game keybindings
bool RTT_Ogre_Base::keyPressed( const KeyEvent& evt )
{
	switch (evt.key)
	{
	case KC_ESCAPE:
		LogManager::getSingletonPtr()->logMessage("Quitting!!!");
	    rttShutDown = true;
	    break;
	case KC_NUMPAD4:
		LogManager::getSingletonPtr()->logMessage("Moving North West");
		break;
	case KC_NUMPAD5:
		LogManager::getSingletonPtr()->logMessage("Moving North");
		break;
	case KC_NUMPAD6:
		LogManager::getSingletonPtr()->logMessage("Moving North East");
		break;
	case KC_NUMPAD1:
		LogManager::getSingletonPtr()->logMessage("Moving South West");
		break;
	case KC_NUMPAD2:
		LogManager::getSingletonPtr()->logMessage("Moving South");
		break;
	case KC_NUMPAD3:
		LogManager::getSingletonPtr()->logMessage("Moving South East");
		break;
	default:
	    break;
	}
	return true;
}

bool RTT_Ogre_Base::keyReleased( const KeyEvent& evt )
{
	return true;
}

bool RTT_Ogre_Base::mouseMoved( const MouseEvent &arg )
{
    return true;
}

bool RTT_Ogre_Base::mousePressed( const MouseEvent &arg, MouseButtonID mouseId )
{
    return true;
}

bool RTT_Ogre_Base::mouseReleased( const MouseEvent &arg, MouseButtonID mouseId )
{
    return true;
}

bool RTT_Ogre_Base::frameRenderingQueued(const FrameEvent& evt)
{
    if(rttWindow->isClosed())
        return false;
    if(rttShutDown)
    	return false;
    //Need to capture/update each device
    rttKeyboard->capture();
    rttMouse->capture();
    return true;
}

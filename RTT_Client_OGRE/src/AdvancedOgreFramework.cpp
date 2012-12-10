//============================================================================
// Name        : AdvancedOgreFramework.cpp
// Author      : Mark Petro
// Copyright   : 2011, GNU GPLv3
// Description : Built from the Advanced OGRE Framework tutorial found here:
//	http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Advanced+Ogre+Framework&structure=Tutorials
//============================================================================

#include "AdvancedOgreFramework.h"
#include "OgreGameEvents.h"
#include "OgreMainLobbyEvents.h"
#include "OgreMatchLobbyEvents.h"

#include <fstream>

using namespace Ogre;
using namespace RTT;

template<> OgreFramework *Ogre::Singleton<OgreFramework>::msSingleton = NULL;

OgreFramework::OgreFramework()
{
	m_root = NULL;
	m_renderWnd = NULL;
	m_viewport = NULL;
	m_log = NULL;
	m_timer = NULL;

	m_inputMgr = NULL;
	m_keyboard = NULL;
	m_mouse = NULL;

	m_debugOverlay = NULL;
	m_infoOverlay = NULL;
	m_numScreenShots = 0;
	m_callbackHandler = new CallbackHandler(new OgreGameEvents(), new OgreMatchLobbyEvents(), new OgreMainLobbyEvents());
}

OgreFramework::~OgreFramework()
{
	OgreFramework::getSingletonPtr()->m_log->logMessage("Shutdown OGRE...");

	if(m_inputMgr)
	{
		OIS::InputManager::destroyInputSystem(m_inputMgr);
	}
	if(m_root)
	{
		delete m_root;
	}
}

bool OgreFramework::InitOgre(Ogre::String wndTitle, OIS::KeyListener *pKeyListener,
		OIS::MouseListener *pMouseListener)
{
	bool useRelativePaths = true;

	//Try to find <things>.cfg in a relative path. (current directory)
	//	This is useful for debugging. If not found, then try the system
	//	install path of /usr/share/RTT/Ogre/. If not found there, then
	//	something is wrong and quit.
	std::string resourcesFilePath = RESOURCES_CFG_FILENAME;
	{
		std::ifstream relativeFile(resourcesFilePath.c_str());
		if(!relativeFile.good())
		{
			useRelativePaths = false;
			resourcesFilePath = SYSTEM_INSTALL_PATH;
			resourcesFilePath += RESOURCES_CFG_FILENAME;
			std::ifstream installPathFile(resourcesFilePath.c_str());
			if(installPathFile.bad())
			{
				std::cerr << "ERROR: resouces.cfg not found in either current directory nor system"
						" install path. (/usr/share/RTT/Ogre/)";
				exit(EXIT_FAILURE);
			}
			std::cout << "Using /usr/share/RTT/Ogre/resources.cfg\n";
		}
		else
		{
			std::cout << "Using relative resources.cfg\n";
		}
	}

	std::string pluginsFilePath = PLUGINS_CFG_FILENAME;
	{
		std::ifstream relativeFile(pluginsFilePath.c_str());
		if(!relativeFile.good())
		{
			useRelativePaths = false;
			pluginsFilePath = SYSTEM_INSTALL_PATH;
			pluginsFilePath += PLUGINS_CFG_FILENAME;
			std::ifstream installPathFile(pluginsFilePath.c_str());
			if(installPathFile.bad())
			{
				std::cerr << "ERROR: plugins.cfg not found in either current directory nor system"
						" install path. (/usr/share/RTT/Ogre/)";
				exit(EXIT_FAILURE);
			}
			std::cout << "Using /usr/share/RTT/Ogre/plugins.cfg\n";
		}
		else
		{
			std::cout << "Using relative plugins.cfg\n";
		}
	}

	std::string ogrecfgFilePath = OGRE_CFG_FILENAME;
	{
		std::ifstream relativeFile(ogrecfgFilePath.c_str());
		if(!relativeFile.good())
		{
			useRelativePaths = false;
			ogrecfgFilePath = SYSTEM_INSTALL_PATH;
			ogrecfgFilePath += OGRE_CFG_FILENAME;
			std::ifstream installPathFile(ogrecfgFilePath.c_str());
			if(installPathFile.bad())
			{
				std::cerr << "ERROR: plugins.cfg not found in either current directory nor system"
						" install path. (/usr/share/RTT/Ogre/)";
				exit(EXIT_FAILURE);
			}
			std::cout << "Using /usr/share/RTT/Ogre/ogre.cfg\n";
		}
		else
		{
			std::cout << "Using relative ogre.cfg\n";
		}
	}

	std::string ogreLogFilePath;
	if(useRelativePaths)
	{
		ogreLogFilePath = OGRE_LOG_FILENAME;
	}
	else
	{
		ogreLogFilePath = SYSTEM_LOG_PATH;
		ogreLogFilePath += OGRE_LOG_FILENAME;
	}

	new Ogre::LogManager();

	m_log = Ogre::LogManager::getSingleton().createLog(ogreLogFilePath, true, true, false);
	m_log->setDebugOutputEnabled(true);

	m_root = new Ogre::Root(pluginsFilePath, ogrecfgFilePath, ogreLogFilePath);

	if(!(m_root->restoreConfig()))
	{
		if(!(m_root->showConfigDialog()))
		{
			return false;
		}
	}

	m_renderWnd = m_root->initialise(true, wndTitle);

	m_viewport = m_renderWnd->addViewport(NULL);
	m_viewport->setBackgroundColour(ColourValue(0.5f, 0.5f, 0.5f, 1.0f));

	m_viewport->setCamera(NULL);

	size_t hWnd = 0;
	OIS::ParamList paramList;
	m_renderWnd->getCustomAttribute("WINDOW", &hWnd);

	paramList.insert(OIS::ParamList::value_type("WINDOW", Ogre::StringConverter::toString(hWnd)));

	m_inputMgr = OIS::InputManager::createInputSystem(paramList);

	m_keyboard = static_cast<OIS::Keyboard*>(m_inputMgr->createInputObject(OIS::OISKeyboard, true));
	m_mouse = static_cast<OIS::Mouse*>(m_inputMgr->createInputObject(OIS::OISMouse, true));

	m_mouse->getMouseState().height = m_renderWnd->getHeight();
	m_mouse->getMouseState().width  = m_renderWnd->getWidth();

	if(pKeyListener == NULL)
	{
		m_keyboard->setEventCallback(this);
	}
	else
	{
		m_keyboard->setEventCallback(pKeyListener);
	}

	if(pMouseListener == NULL)
	{
		m_mouse->setEventCallback(this);
	}
	else
	{
		m_mouse->setEventCallback(pMouseListener);
	}

	Ogre::String secName, typeName, archName;
	Ogre::ConfigFile cf;
	cf.load(resourcesFilePath);

	Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
	while (seci.hasMoreElements())
	{
		secName = seci.peekNextKey();
		Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
		Ogre::ConfigFile::SettingsMultiMap::iterator i;
		for (i = settings->begin(); i != settings->end(); ++i)
		{
			typeName = i->first;
			archName = i->second;
			Ogre::ResourceGroupManager::getSingleton().addResourceLocation(archName,
					typeName, secName);
		}
	}
	Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();


	m_timer = new Ogre::Timer();
	m_timer->reset();

	m_GUIRenderer = &CEGUI::OgreRenderer::bootstrapSystem(*m_renderWnd);
	m_GUISystem = CEGUI::System::getSingletonPtr();

	m_GUIType = "OgreTray";

	CEGUI::SchemeManager::getSingleton().create(m_GUIType + ".scheme");
	CEGUI::WindowManager::getSingleton().loadWindowLayout((CEGUI::utf8*)"RTT_GUI.layout");

	m_debugOverlay = OverlayManager::getSingleton().getByName("Core/DebugOverlay");
	if(m_debugOverlay == NULL)
	{
		std::cerr << "ERROR: Could not find resource: Core/DebugOverlay" << std::endl;
		::exit(EXIT_FAILURE);
	}

	m_debugOverlay->show();

	m_renderWnd->setActive(true);

	OgreFramework::getSingletonPtr()->m_GUISystem->setDefaultMouseCursor(m_GUIType + "Images", (CEGUI::utf8*)"MouseArrow");
	CEGUI::MouseCursor::getSingleton().setImage(m_GUIType + "Images", "MouseArrow");
	const OIS::MouseState state = m_mouse->getMouseState();
	CEGUI::Point mousePos = CEGUI::MouseCursor::getSingleton().getPosition();
	CEGUI::System::getSingleton().injectMouseMove(state.X.abs-mousePos.d_x,state.Y.abs-mousePos.d_y);

	return true;
}

bool OgreFramework::keyPressed(const OIS::KeyEvent &keyEventRef)
{
	if(m_keyboard->isKeyDown(OIS::KC_SYSRQ))
	{
		m_renderWnd->writeContentsToTimestampedFile("RTT_Screenshot_", ".jpg");
		return true;
	}

	if(m_keyboard->isKeyDown(OIS::KC_GRAVE))
	{
		if(m_debugOverlay)
		{
			if(!m_debugOverlay->isVisible())
				m_debugOverlay->show();
			else
				m_debugOverlay->hide();
		}
	}

	return true;
}

bool OgreFramework::keyReleased(const OIS::KeyEvent &keyEventRef)
{
	return true;
}

bool OgreFramework::mouseMoved(const OIS::MouseEvent &evt)
{
	return true;
}

bool OgreFramework::mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{
	return true;
}

bool OgreFramework::mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{
	return true;
}

void OgreFramework::UpdateOgre(double timeSinceLastFrame)
{
	UpdateStats();
}

void OgreFramework::UpdateStats()
{
	static String currFps = "Current FPS: ";
    static String avgFps = "Average FPS: ";
    static String bestFps = "Best FPS: ";
    static String worstFps = "Worst FPS: ";
    static String tris = "Triangle Count: ";
    static String batches = "Batch Count: ";

    OverlayElement* guiAvg = OverlayManager::getSingleton().getOverlayElement("Core/AverageFps");
    OverlayElement* guiCurr = OverlayManager::getSingleton().getOverlayElement("Core/CurrFps");
    OverlayElement* guiBest = OverlayManager::getSingleton().getOverlayElement("Core/BestFps");
    OverlayElement* guiWorst = OverlayManager::getSingleton().getOverlayElement("Core/WorstFps");

	const RenderTarget::FrameStats& stats = m_renderWnd->getStatistics();
    guiAvg->setCaption(avgFps + StringConverter::toString(stats.avgFPS));
    guiCurr->setCaption(currFps + StringConverter::toString(stats.lastFPS));
    guiBest->setCaption(bestFps + StringConverter::toString(stats.bestFPS)
            +" "+StringConverter::toString(stats.bestFrameTime)+" ms");
    guiWorst->setCaption(worstFps + StringConverter::toString(stats.worstFPS)
            +" "+StringConverter::toString(stats.worstFrameTime)+" ms");

    OverlayElement* guiTris = OverlayManager::getSingleton().getOverlayElement("Core/NumTris");
    guiTris->setCaption(tris + StringConverter::toString(stats.triangleCount));

	OverlayElement* guiBatches = OverlayManager::getSingleton().getOverlayElement("Core/NumBatches");
    guiBatches->setCaption(batches + StringConverter::toString(stats.batchCount));

	OverlayElement* guiDbg = OverlayManager::getSingleton().getOverlayElement("Core/DebugText");
	guiDbg->setCaption("");
}

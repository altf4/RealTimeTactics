//============================================================================
// Name        : AdvancedOgreFramework.cpp
// Author      : Mark Petro
// Copyright   : 2011, GNU GPLv3
// Description : Built from the Advanced OGRE Framework tutorial found here:
//	http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Advanced+Ogre+Framework&structure=Tutorials
//============================================================================

#include "AdvancedOgreFramework.h"
#include <fstream>

using namespace Ogre;

template<> OgreFramework *Ogre::Singleton<OgreFramework>::msSingleton = NULL;

OgreFramework::OgreFramework()
{
	m_pRoot = NULL;
	m_pRenderWnd = NULL;
	m_pViewport = NULL;
	m_pLog = NULL;
	m_pTimer = NULL;

	m_pInputMgr = NULL;
	m_pKeyboard = NULL;
	m_pMouse = NULL;
	//m_pTrayMgr = NULL;

	//m_pDebugOverlay = NULL;
	//m_pInfoOverlay = NULL;
	m_iNumScreenShots = 0;
}

OgreFramework::~OgreFramework()
{
	OgreFramework::getSingletonPtr()->m_pLog->logMessage("Shutdown OGRE...");
	/*
	if(m_pTrayMgr)
	{
		delete m_pTrayMgr;
	}
	*/
	if(m_pInputMgr)
	{
		OIS::InputManager::destroyInputSystem(m_pInputMgr);
	}
	if(m_pRoot)
	{
		delete m_pRoot;
	}
}

bool OgreFramework::initOgre(Ogre::String wndTitle, OIS::KeyListener *pKeyListener,
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

	m_pLog = Ogre::LogManager::getSingleton().createLog(ogreLogFilePath, true, true, false);
	m_pLog->setDebugOutputEnabled(true);

	m_pRoot = new Ogre::Root(pluginsFilePath, ogrecfgFilePath, ogreLogFilePath);

	if(!(m_pRoot->restoreConfig() || m_pRoot->showConfigDialog()))
		return false;

	m_pRenderWnd = m_pRoot->initialise(true, wndTitle);

	m_pViewport = m_pRenderWnd->addViewport(NULL);
	m_pViewport->setBackgroundColour(ColourValue(0.5f, 0.5f, 0.5f, 1.0f));

	m_pViewport->setCamera(NULL);

	size_t hWnd = 0;
	OIS::ParamList paramList;
	m_pRenderWnd->getCustomAttribute("WINDOW", &hWnd);

	paramList.insert(OIS::ParamList::value_type("WINDOW", Ogre::StringConverter::toString(hWnd)));

	m_pInputMgr = OIS::InputManager::createInputSystem(paramList);

	m_pKeyboard = static_cast<OIS::Keyboard*>(m_pInputMgr->createInputObject(OIS::OISKeyboard, true));
	m_pMouse = static_cast<OIS::Mouse*>(m_pInputMgr->createInputObject(OIS::OISMouse, true));

	m_pMouse->getMouseState().height = m_pRenderWnd->getHeight();
	m_pMouse->getMouseState().width  = m_pRenderWnd->getWidth();

	if(pKeyListener == NULL)
	{
		m_pKeyboard->setEventCallback(this);
	}
	else
	{
		m_pKeyboard->setEventCallback(pKeyListener);
	}

	if(pMouseListener == NULL)
	{
		m_pMouse->setEventCallback(this);
	}
	else
	{
		m_pMouse->setEventCallback(pMouseListener);
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

	//m_pTrayMgr = new OgreBites::SdkTrayManager("AOFTrayMgr", m_pRenderWnd, m_pMouse, NULL);

	m_pTimer = new Ogre::Timer();
	m_pTimer->reset();

	m_pGUIRenderer = &CEGUI::OgreRenderer::bootstrapSystem(*m_pRenderWnd);
	m_pGUISystem = CEGUI::System::getSingletonPtr();

	CEGUI::SchemeManager::getSingleton().create((CEGUI::utf8*)"OgreTray.scheme");
	CEGUI::WindowManager::getSingleton().loadWindowLayout((CEGUI::utf8*)"AdvancedOgreFramework.layout");
	CEGUI::WindowManager::getSingleton().loadWindowLayout((CEGUI::utf8*)"AdvancedOgreFramework_Game.layout");
	CEGUI::WindowManager::getSingleton().loadWindowLayout((CEGUI::utf8*)"JoinCustomServer.layout");
	CEGUI::WindowManager::getSingleton().loadWindowLayout((CEGUI::utf8*)"Lobby.layout");

	m_pDebugOverlay = OverlayManager::getSingleton().getByName("Core/DebugOverlay");
	if(m_pDebugOverlay == NULL)
	{
		std::cerr << "ERROR: Could not find resource: Core/DebugOverlay" << std::endl;
		::exit(EXIT_FAILURE);
	}

	m_pDebugOverlay->show();

	m_pRenderWnd->setActive(true);

	OgreFramework::getSingletonPtr()->m_pGUISystem->setDefaultMouseCursor((CEGUI::utf8*)"OgreTrayImages", (CEGUI::utf8*)"MouseArrow");
	CEGUI::MouseCursor::getSingleton().setImage("OgreTrayImages", "MouseArrow");
	const OIS::MouseState state = m_pMouse->getMouseState();
	CEGUI::Point mousePos = CEGUI::MouseCursor::getSingleton().getPosition();
	CEGUI::System::getSingleton().injectMouseMove(state.X.abs-mousePos.d_x,state.Y.abs-mousePos.d_y);

	return true;
}

bool OgreFramework::keyPressed(const OIS::KeyEvent &keyEventRef)
{
	if(m_pKeyboard->isKeyDown(OIS::KC_SYSRQ))
	{
		m_pRenderWnd->writeContentsToTimestampedFile("AOF_Screenshot_", ".jpg");
		return true;
	}

	if(m_pKeyboard->isKeyDown(OIS::KC_GRAVE))
	{
		if(m_pDebugOverlay)
		{
			if(!m_pDebugOverlay->isVisible())
				m_pDebugOverlay->show();
			else
				m_pDebugOverlay->hide();
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

void OgreFramework::updateOgre(double timeSinceLastFrame)
{
	updateStats();
}

void OgreFramework::updateStats()
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

	const RenderTarget::FrameStats& stats = m_pRenderWnd->getStatistics();
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

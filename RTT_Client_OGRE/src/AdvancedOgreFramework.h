//============================================================================
// Name        : AdvancedOgreFramework.hpp
// Author      : Mark Petro
// Copyright   : 2011, GNU GPLv3
// Description : Built from the Advanced OGRE Framework tutorial found here:
//	http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Advanced+Ogre+Framework&structure=Tutorials
//============================================================================

#ifndef OGRE_FRAMEWORK_HPP
#define OGRE_FRAMEWORK_HPP

#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreLogManager.h>
#include <OgreOverlay.h>
#include <OgreOverlayElement.h>
#include <OgreOverlayManager.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreConfigFile.h>

#include <OISEvents.h>
#include <OISInputManager.h>
#include <OISKeyboard.h>
#include <OISMouse.h>

#include <CEGUI.h>
#include <CEGUI/RendererModules/Ogre/CEGUIOgreRenderer.h>

#include "messaging/MessageManager.h"
#include "ClientProtocolHandler.h"
#include <iostream>
#include <arpa/inet.h>
#include <vector>
#include "boost/date_time/posix_time/posix_time.hpp"
#include <openssl/sha.h>
#include "Player.h"
#include "callback/CallbackHandler.h"

#define RESOURCES_CFG_FILENAME "resources.cfg"
#define PLUGINS_CFG_FILENAME "plugins.cfg"
#define OGRE_CFG_FILENAME "ogre.cfg"
#define OGRE_LOG_FILENAME "OgreLogfile.log"
#define SYSTEM_INSTALL_PATH "/usr/share/RTT/Ogre/"
#define SYSTEM_LOG_PATH	 "/var/log/RTT/"

class OgreFramework : public Ogre::Singleton<OgreFramework>, OIS::KeyListener, OIS::MouseListener
{
public:
	OgreFramework();
	~OgreFramework();

	bool initOgre(Ogre::String wndTitle, OIS::KeyListener *pKeyListener = 0, OIS::MouseListener *pMouseListener = 0);
	void updateOgre(double timeSinceLastFrame);

	bool keyPressed(const OIS::KeyEvent &keyEventRef);
	bool keyReleased(const OIS::KeyEvent &keyEventRef);

	bool mouseMoved(const OIS::MouseEvent &evt);
	bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id);
	bool mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id);

	void updateStats();

	Ogre::Root *m_pRoot;
	Ogre::RenderWindow *m_pRenderWnd;
	Ogre::Viewport *m_pViewport;
	Ogre::Log *m_pLog;
	Ogre::Timer *m_pTimer;

	OIS::InputManager *m_pInputMgr;
	OIS::Keyboard *m_pKeyboard;
	OIS::Mouse *m_pMouse;

	CEGUI::OgreRenderer *m_pGUIRenderer;
	CEGUI::System *m_pGUISystem;

	CEGUI::String m_pGUIType;

	RTT::CallbackHandler *m_callbackHandler;

private:
	OgreFramework(const OgreFramework&);
	OgreFramework& operator= (const OgreFramework&);

	Ogre::Overlay *m_pDebugOverlay;
	Ogre::Overlay *m_pInfoOverlay;
	int	m_iNumScreenShots;
};

#endif

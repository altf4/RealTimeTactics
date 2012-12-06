//============================================================================
// Name        : AdvancedOgreFramework.hpp
// Author      : Mark Petro
// Copyright   : 2011, GNU GPLv3
// Description : Built from the Advanced OGRE Framework tutorial found here:
//	http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Advanced+Ogre+Framework&structure=Tutorials
//============================================================================

#ifndef OGRE_FRAMEWORK_HPP
#define OGRE_FRAMEWORK_HPP

#include <OGRE/OgreCamera.h>
#include <OGRE/OgreEntity.h>
#include <OGRE/OgreLogManager.h>
#include <OGRE/OgreOverlay.h>
#include <OGRE/OgreOverlayElement.h>
#include <OGRE/OgreOverlayManager.h>
#include <OGRE/OgreRoot.h>
#include <OGRE/OgreViewport.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreRenderWindow.h>
#include <OGRE/OgreConfigFile.h>

#include <OIS/OISEvents.h>
#include <OIS/OISInputManager.h>
#include <OIS/OISKeyboard.h>
#include <OIS/OISMouse.h>

#include <CEGUI/CEGUI.h>
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

	bool InitOgre(Ogre::String wndTitle, OIS::KeyListener *pKeyListener = 0, OIS::MouseListener *pMouseListener = 0);
	void UpdateOgre(double timeSinceLastFrame);

	bool keyPressed(const OIS::KeyEvent &keyEventRef);
	bool keyReleased(const OIS::KeyEvent &keyEventRef);

	bool mouseMoved(const OIS::MouseEvent &evt);
	bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id);
	bool mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id);

	void UpdateStats();

	Ogre::Root *m_root;
	Ogre::RenderWindow *m_renderWnd;
	Ogre::Viewport *m_viewport;
	Ogre::Log *m_log;
	Ogre::Timer *m_timer;

	OIS::InputManager *m_inputMgr;
	OIS::Keyboard *m_keyboard;
	OIS::Mouse *m_mouse;

	CEGUI::OgreRenderer *m_GUIRenderer;
	CEGUI::System *m_GUISystem;

	CEGUI::String m_GUIType;

	RTT::CallbackHandler *m_callbackHandler;

private:
	OgreFramework(const OgreFramework&);
	OgreFramework& operator= (const OgreFramework&);

	Ogre::Overlay *m_debugOverlay;
	Ogre::Overlay *m_infoOverlay;
	int	m_numScreenShots;
};

#endif

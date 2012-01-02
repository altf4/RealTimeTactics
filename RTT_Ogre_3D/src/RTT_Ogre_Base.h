//============================================================================
// Name        : RTT_Ogre_Base.h
// Author      : Nysomin
// Copyright   : 2011, GNU GPLv3
// Description : Ogre 3D rendering engine base application for RealTimeTactics
//               Based on Ogre Tutorial framework
//============================================================================
#ifndef __RTT_Ogre_Base_h_
#define __RTT_Ogre_Base_h_

#include <OgreRoot.h>
#include <OISEvents.h>
#include <OISInputManager.h>
#include <OISKeyboard.h>
#include <OISMouse.h>
#include <OgreWindowEventUtilities.h>

using namespace Ogre;
using namespace OIS;

namespace RTT
{
	class RTT_Ogre_Base : public FrameListener, public WindowEventListener, public KeyListener, public MouseListener
	{
		public:
			RTT_Ogre_Base(void);
			virtual ~RTT_Ogre_Base(void);

			virtual void go(void);

		protected:
			virtual bool setup();
			virtual bool configure(void);
			virtual void chooseSceneManager(void);
			virtual void createCamera(void);
			virtual void createFrameListener(void);
			virtual void createScene(void) = 0; // Override in RTT_Ogre_Game
			virtual void destroyScene(void);
			virtual void createViewports(void);
			virtual void setupResources(void);
			virtual void createResourceListener(void);
			virtual void loadResources(void);

			//FrameListener
			virtual bool frameRenderingQueued(const FrameEvent& evt);
			//KeyListeners
			virtual bool keyPressed( const KeyEvent &arg );
			virtual bool keyReleased( const KeyEvent &arg );
			// OIS::MouseListeners
			virtual bool mouseMoved( const MouseEvent &arg );
			virtual bool mousePressed( const MouseEvent &arg, MouseButtonID mouseId );
			virtual bool mouseReleased( const MouseEvent &arg, MouseButtonID mouseId );

			//WindowEventListeners
			virtual void windowResized(RenderWindow* renderWindow);//Adjust mouse clipping area
			virtual void windowClosed(RenderWindow* renderWindoww);//Unattach OIS before window shutdown (very important under Linux)

			Root *rttRoot;
			Camera* rttCamera;
			SceneManager* rttSceneManager;
			RenderWindow* rttWindow;
			String rttResourcesCfg;
			String rttPluginsCfg;

			bool rttShutDown;

			//OIS Input devices
			InputManager* rttInputManager;
			Mouse*    rttMouse;
			Keyboard* rttKeyboard;
	};
}

#endif // #ifndef __RTT_Ogre_Base_h_

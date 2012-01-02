//============================================================================
// Name        : RTT_Ogre_3D.h
// Author      : Nysomin
// Copyright   : 2011, GNU GPLv3
// Description : Ogre 3D rendering engine for RealTimeTactics
//============================================================================

#ifndef RTT_Ogre_3D_H_
#define RTT_Ogre_3D_H_

#include <OgreRoot.h>
#include <OISEvents.h>
#include <OISInputManager.h>
#include <OISKeyboard.h>
#include <OISMouse.h>
#include <OgreWindowEventUtilities.h>
#include <vector>

using namespace Ogre;
using namespace OIS;

namespace RTT
{
	class RTT_Ogre_3D : public WindowEventListener, public FrameListener, public OIS::KeyListener//, public OIS::MouseListener
	{
	public:
		RTT_Ogre_3D(void);
		virtual ~RTT_Ogre_3D(void);
		bool go(void);
	private:
	    Root* rttRoot; //The root node to Ogre 3D render engine
	    String rttPluginsCfg; //Plugin configurations
	    String rttResourcesCfg; //Resource configurations
	    RenderWindow* rttWindow; // Ogre render window
	    SceneManager* rttSceneManager; // Ogre Screen Manager
	    Camera* rttCamera; // Ogre primary camera
	    InputManager* rttInputManager;
	    Mouse*    rttMouse;
	    Keyboard* rttKeyboard;
	    vector<bool> rttKeyDown;
	protected:
	    virtual void windowResized(RenderWindow* rw); //WindowEventListener
	    virtual void windowClosed(RenderWindow* rw); //WindowEventListener
	    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);//FrameListener
	    virtual bool keyPressed(const KeyEvent& evt );
	    virtual bool keyReleased(const KeyEvent& evt );
	    bool rttShutdown;
	};
}

#endif /* RTT_Ogre_3D_H_ */

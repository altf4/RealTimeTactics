//============================================================================
// Name        : RTT_Ogre_3D.h
// Author      : Nysomin
// Copyright   : 2011, GNU GPLv3
// Description : Ogre 3D rendering engine for RealTimeTactics
//============================================================================

#ifndef RTT_Ogre_3D_H_
#define RTT_Ogre_3D_H_

#include <OgreRoot.h>

using namespace Ogre;

namespace RTT
{
	class RTT_Ogre_3D
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
	};
}

#endif /* RTT_Ogre_3D_H_ */

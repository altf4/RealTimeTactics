//============================================================================
// Name        : RTT_Ogre_Base.h
// Author      : Nysomin
// Copyright   : 2011, GNU GPLv3
// Description : Ogre 3D rendering engine base application for RealTimeTactics
//               Based on Ogre Tutorial framework
//============================================================================

#ifndef RTT_OGRE_GAME_H_
#define RTT_OGRE_GAME_H_

#include <vector>
#include "RTT_Ogre_Base.h"




using namespace RTT;

class RTT_Ogre_Game : public RTT_Ogre_Base
{
	public:
    	RTT_Ogre_Game(void);
    	virtual ~RTT_Ogre_Game(void);
	protected:
    	virtual void createScene(void);
    	virtual void moveCharacter(const RTT::Direction &arg);
    	virtual bool keyPressed( const KeyEvent &arg );
    	virtual void buildPlayers(void);
    	Ogre::Entity* mainPlayer;
    	Ogre::SceneNode* mainPlayerNode;
    	int unitX;
    	int unitY;
};

#endif /* RTT_OGRE_GAME_H_ */

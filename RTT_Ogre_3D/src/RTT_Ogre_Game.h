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
#include "RTT_Ogre_Unit.h"



using namespace RTT;

class RTT_Ogre_Game : public RTT_Ogre_Base
{
	public:
    	RTT_Ogre_Game(void);
    	virtual ~RTT_Ogre_Game(void);

	protected:
    	virtual void createScene(void);
    	virtual void moveUnit(const RTT::Direction &arg, RTT_Ogre_Unit &arg2);
    	virtual bool keyPressed( const KeyEvent &arg );
    	virtual void buildUnits(void);
    	RTT_Ogre_Unit mainPlayer;
};

#endif /* RTT_OGRE_GAME_H_ */

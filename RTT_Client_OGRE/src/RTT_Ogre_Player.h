//============================================================================
// Name        : RTT_Ogre_Player.h
// Author      : Nysomin
// Copyright   : 2012, GNU GPLv3
// Description : Loads RTT map data into Ogre
//============================================================================

#ifndef RTT_OGRE_PLAYER_H_
#define RTT_OGRE_PLAYER_H_


#include "RTT_Ogre_Unit.h"
#include "AdvancedOgreFramework.h"
#include "Enums.h"

#include <vector>

namespace RTT
{
	class RTT_Ogre_Player
	{
	public:
		RTT_Ogre_Player();
		virtual ~RTT_Ogre_Player();
		RTT_Ogre_Unit ogreUnits;
		uint currentUnit;
		Ogre::SceneNode* rangeNode[8][8];
		RTT::TeamNumber Team;
	};

}

#endif /* RTT_OGRE_PLAYER_H_ */

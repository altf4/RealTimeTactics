//============================================================================
// Name        : RTT_Ogre_Player.h
// Author      : Nysomin
// Copyright   : 2012, GNU GPLv3
// Description : Loads RTT map data into Ogre
//============================================================================

#ifndef RTT_OGRE_PLAYER_H_
#define RTT_OGRE_PLAYER_H_


#include "OgreUnit.h"
#include "AdvancedOgreFramework.h"
#include "Enums.h"
#include "Player.h"

#include <vector>

namespace RTT
{

class RTT_Ogre_Player : public Player
{
public:

	RTT_Ogre_Player();
	virtual ~RTT_Ogre_Player();

	uint m_currentUnit;
	RTT::TeamNumber m_team;
};

}

#endif /* RTT_OGRE_PLAYER_H_ */

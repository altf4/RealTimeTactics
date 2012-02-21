//============================================================================
// Name        : RTT_Ogre_Unit.h
// Author      : Nysomin
// Copyright   : 2012, GNU GPLv3
// Description : Extension of RTT Unit class for Ogre
//============================================================================

#ifndef RTT_OGRE_UNIT_H_
#define RTT_OGRE_UNIT_H_


#include "RTT_Ogre_Base.h"

namespace RTT
{
	class RTT_Ogre_Unit
	{
	public:
		RTT_Ogre_Unit();
		virtual ~RTT_Ogre_Unit();
		Ogre::Entity* unitEntity;
		Ogre::SceneNode* unitNode;
		int locationX;
		int locationY;
		int moveRange;
		Direction facingDir;
	private:
	protected:
	};

}


#endif /* RTT_OGRE_UNIT_H_ */

//============================================================================
// Name        : RTT_Ogre_Unit.h
// Author      : Nysomin
// Copyright   : 2012, GNU GPLv3
// Description : Extension of RTT Unit class for Ogre
//============================================================================

#ifndef RTT_OGRE_UNIT_H_
#define RTT_OGRE_UNIT_H_

#include "Unit.h"
//#include "AdvancedOgreFramework.hpp"
#include <OgreSceneNode.h>
#include <OgreEntity.h>

namespace RTT
{
	class RTT_Ogre_Unit :public Unit
	{
	public:
		RTT_Ogre_Unit();
		virtual ~RTT_Ogre_Unit();
/*
		Ogre::Entity* unitEntity;
		Ogre::SceneNode* unitNode;
		int locationX;
		int locationY;
		int moveRange;
		Direction facingDir;
*/
		Ogre::Entity *m_unitEntity;
		Ogre::SceneNode *m_unitNode;
		int m_locationX;
		int m_locationY;

		int m_moveRange;
		RTT::Direction m_facingDir;

	private:
	protected:
	};

}


#endif /* RTT_OGRE_UNIT_H_ */

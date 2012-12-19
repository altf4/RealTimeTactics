//============================================================================
// Name        : OgreUnit.h
// Author      : Nysomin
// Copyright   : 2012, GNU GPLv3
// Description : Extension of RTT Unit class for Ogre
//============================================================================

#ifndef RTT_OGRE_UNIT_H_
#define RTT_OGRE_UNIT_H_

#include "Unit.h"
#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreEntity.h>

namespace RTT
{

class OgreUnit :public Unit
{

public:

	OgreUnit();
	virtual ~OgreUnit();

	Ogre::Entity *m_unitEntity;
	Ogre::SceneNode *m_unitNode;
};

}


#endif /* RTT_OGRE_UNIT_H_ */

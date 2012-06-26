//============================================================================
// Name        : DemoApp.hpp
// Author      : Mark Petro
// Copyright   : 2011, GNU GPLv3
// Description : Built from the Advanced OGRE Framework tutorial found here:
//	http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Advanced+Ogre+Framework&structure=Tutorials
//============================================================================

#ifndef OGRE_DEMO_HPP
#define OGRE_DEMO_HPP

#include "AdvancedOgreFramework.h"
#include "AppStateManager.h"

class DemoApp
{
public:
	DemoApp();
	~DemoApp();

	void startDemo();

private:
	AppStateManager *m_pAppStateManager;
};

#endif

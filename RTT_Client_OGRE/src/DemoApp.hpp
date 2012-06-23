/*
 * DemoApp.hpp
 *
 *  Created on: Jun 22, 2012
 *      Author: mark
 */

//|||||||||||||||||||||||||||||||||||||||||||||||

#ifndef OGRE_DEMO_HPP
#define OGRE_DEMO_HPP

//|||||||||||||||||||||||||||||||||||||||||||||||

#include "AdvancedOgreFramework.hpp"
#include "AppStateManager.hpp"

//|||||||||||||||||||||||||||||||||||||||||||||||

class DemoApp
{
public:
	DemoApp();
	~DemoApp();

	void startDemo();

private:
	AppStateManager*	m_pAppStateManager;
};

//|||||||||||||||||||||||||||||||||||||||||||||||

#endif

//|||||||||||||||||||||||||||||||||||||||||||||||

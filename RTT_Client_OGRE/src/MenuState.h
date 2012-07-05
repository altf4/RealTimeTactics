//============================================================================
// Name        : MenuState.h
// Author      : Mark Petro
// Copyright   : 2011, GNU GPLv3
// Description : Built from the Advanced OGRE Framework tutorial found here:
//	http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Advanced+Ogre+Framework&structure=Tutorials
//============================================================================

#ifndef MENU_STATE_H
#define MENU_STATE_H

#include "AppState.h"

class MenuState : public AppState
{
public:
	MenuState();

	DECLARE_APPSTATE_CLASS(MenuState)

	void enter();
	void createScene();
	void exit();
	bool pause();
	void resume();

	bool keyPressed(const OIS::KeyEvent &keyEventRef);
	bool keyReleased(const OIS::KeyEvent &keyEventRef);

	bool mouseMoved(const OIS::MouseEvent &evt);
	bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id);
	bool mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id);

	//void buttonHit(OgreBites::Button* button);

	bool onExitButton(const CEGUI::EventArgs &args);
	bool onEnterButton(const CEGUI::EventArgs &args);
	bool onJoinCustomServerButton(const CEGUI::EventArgs &args);

	void update(double timeSinceLastFrame);

private:
	bool m_bQuit;
};

#endif

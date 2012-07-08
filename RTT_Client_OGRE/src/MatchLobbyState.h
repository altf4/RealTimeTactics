//============================================================================
// Name        : MatchLobbyState.h
// Author      : Mark Petro
// Copyright   : 2011, GNU GPLv3
// Description : Built from the Advanced OGRE Framework tutorial found here:
//	http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Advanced+Ogre+Framework&structure=Tutorials
//============================================================================

#ifndef MATCH_LOBBY_STATE_H
#define MATCH_LOBBY_STATE_H

#include "AppState.h"

class MatchLobbyState : public AppState
{
public:
	MatchLobbyState();

	DECLARE_APPSTATE_CLASS(MatchLobbyState)

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

	bool MatchExitButton(const CEGUI::EventArgs &args);
	bool MatchBackButton(const CEGUI::EventArgs &args);

	void update(double timeSinceLastFrame);

private:
	bool m_bQuit;
};

#endif

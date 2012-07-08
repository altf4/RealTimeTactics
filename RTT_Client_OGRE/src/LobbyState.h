//============================================================================
// Name        : LobbyState.h
// Author      : Mark Petro
// Copyright   : 2011, GNU GPLv3
// Description : Built from the Advanced OGRE Framework tutorial found here:
//	http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Advanced+Ogre+Framework&structure=Tutorials
//============================================================================

#ifndef LOBBY_STATE_H
#define LOBBY_STATE_H

#include "AppState.h"

class LobbyState : public AppState
{
public:
	LobbyState();

	DECLARE_APPSTATE_CLASS(LobbyState)

	void enter();
	void serverLobby();
	void exit();
	bool pause();
	void resume();

	bool keyPressed(const OIS::KeyEvent &keyEventRef);
	bool keyReleased(const OIS::KeyEvent &keyEventRef);

	bool mouseMoved(const OIS::MouseEvent &evt);
	bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id);
	bool mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id);

	bool onExitButton(const CEGUI::EventArgs &args);
	bool onBackButton(const CEGUI::EventArgs &args);
	bool listMatchesButton(const CEGUI::EventArgs &args);
	bool JoinMatchButton(const CEGUI::EventArgs &args);

	void listMatches();
	void listPlayers();

	void matchLobby(RTT::PlayerDescription *playerDescriptions, uint playerCount);

	void update(double timeSinceLastFrame);

	CEGUI::MultiColumnList *multiColumnListMatch;
	CEGUI::MultiColumnList *multiColumnListPlayer;

	RTT::PlayerDescription m_playerDescription;
	RTT::MatchDescription m_currentMatch;

private:
	bool m_bQuit;
	bool m_bInMatch;
	bool m_bInit;
};

#endif

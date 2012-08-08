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

enum GUIState
{
	MAINMENU,
	JOINCUSTOMSERVER,
	SERVERLOBBY,
	MATCHLOBBY,
	CREATEMATCH
};

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

	bool onExitButton(const CEGUI::EventArgs &args);
	bool onBackButton(const CEGUI::EventArgs &args);
	bool onEnterButton(const CEGUI::EventArgs &args);

	bool onCustomServerButton(const CEGUI::EventArgs &args);
	bool onJoinServerButton(const CEGUI::EventArgs &args);

	bool onAddressActivate(const CEGUI::EventArgs &args);
	bool onAddressDeactivate(const CEGUI::EventArgs &args);

	bool onPortActivate(const CEGUI::EventArgs &args);
	bool onPortDeactivate(const CEGUI::EventArgs &args);

	bool onUsernameActivate(const CEGUI::EventArgs &args);
	bool onUsernameDeactivate(const CEGUI::EventArgs &args);

	bool onPasswordActivate(const CEGUI::EventArgs &args);
	bool onPasswordDeactivate(const CEGUI::EventArgs &args);

	bool listMatchesButton(const CEGUI::EventArgs &args);
	bool JoinMatchButton(const CEGUI::EventArgs &args);

	bool createMatchButton(const CEGUI::EventArgs &args);
	bool createMatchSubmitButton(const CEGUI::EventArgs &args);

	bool onMatchNameActivate(const CEGUI::EventArgs &args);
	bool onMatchNameDeactivate(const CEGUI::EventArgs &args);

	bool onLeaderClick(const CEGUI::EventArgs &args);

	void listMatches();
	void ListPlayers();

	void matchLobby();
	void serverLobby();


	CEGUI::MultiColumnList *multiColumnListMatch;
	CEGUI::MultiColumnList *multiColumnListPlayer;

	RTT::PlayerDescription m_playerDescription;

	std::vector <RTT::PlayerDescription> m_currentPlayers;

	std::vector <CEGUI::DefaultWindow*> m_playerNameTextBoxes;
	std::vector <CEGUI::RadioButton*> m_isLeaderCheckBoxes;

	RTT::MatchDescription m_currentMatch;

	void update(double timeSinceLastFrame);

	void ProcessCallback(struct RTT::CallbackChange);

private:
	bool m_bQuit;
	bool m_bInMatch;
	bool m_bInit;
	GUIState mLocation;

protected:
	void TeamChangedEvent(struct RTT::CallbackChange change);
	void TeamColorChangedEvent(struct RTT::CallbackChange change);
	void MapChangedEvent(struct RTT::CallbackChange change);
	void GamespeedChangedEvent(struct RTT::CallbackChange change);
	void VictoryConditionChangedEvent(struct RTT::CallbackChange change);
	void PlayerLeftEvent(struct RTT::CallbackChange change);
	void KickedFromMatchEvent(struct RTT::CallbackChange change);
	void PlayerJoinedEvent(struct RTT::CallbackChange change);
	void MatchStartedEvent(struct RTT::CallbackChange change);
	void CallbackClosedEvent(struct RTT::CallbackChange change);
	void CallbackErrorEvent(struct RTT::CallbackChange change);
	void LeaderChangedEvent(struct RTT::CallbackChange change);

	void enableLeader(bool value);

};

#endif

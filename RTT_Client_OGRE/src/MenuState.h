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
#include "callback/MainLobbyCallbackChange.h"

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

	void Enter();
	void CreateScene();
	void Exit();
	bool Pause();
	void Resume();
	bool MatchStart();

	bool keyPressed(const OIS::KeyEvent &keyEventRef);
	bool keyReleased(const OIS::KeyEvent &keyEventRef);

	bool mouseMoved(const OIS::MouseEvent &evt);
	bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id);
	bool mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id);

	bool OnExitButton(const CEGUI::EventArgs &args);
	bool OnBackButton(const CEGUI::EventArgs &args);

	bool OnCustomServerButton(const CEGUI::EventArgs &args);
	bool OnJoinServerButton(const CEGUI::EventArgs &args);

	bool OnAddressActivate(const CEGUI::EventArgs &args);
	bool OnAddressDeactivate(const CEGUI::EventArgs &args);

	bool OnPortActivate(const CEGUI::EventArgs &args);
	bool OnPortDeactivate(const CEGUI::EventArgs &args);

	bool OnUsernameActivate(const CEGUI::EventArgs &args);
	bool OnUsernameDeactivate(const CEGUI::EventArgs &args);

	bool OnPasswordActivate(const CEGUI::EventArgs &args);
	bool OnPasswordDeactivate(const CEGUI::EventArgs &args);

	bool DisplayMatchesButton(const CEGUI::EventArgs &args);
	bool JoinMatchButton(const CEGUI::EventArgs &args);

	bool CreateMatchButton(const CEGUI::EventArgs &args);
	bool CreateMatchSubmitButton(const CEGUI::EventArgs &args);

	bool OnMatchNameActivate(const CEGUI::EventArgs &args);
	bool OnMatchNameDeactivate(const CEGUI::EventArgs &args);

	bool OnLeaderClick(const CEGUI::EventArgs &args);
	bool OnTeamChangeClick(const CEGUI::EventArgs &args);

	bool OnMatchStartButton(const CEGUI::EventArgs &args);

	void DisplayMatches();
	void ListPlayers();

	void MatchLobby();
	void ServerLobby();


	CEGUI::MultiColumnList *m_ultiColumnListMatch;
	CEGUI::MultiColumnList *m_ultiColumnListPlayer;

	RTT::PlayerDescription m_playerDescription;

	std::vector <RTT::PlayerDescription> m_currentPlayers;

	std::vector <CEGUI::DefaultWindow*> m_playerNameTextBoxes;
	std::vector <CEGUI::RadioButton*> m_isLeaderCheckBoxes;
	std::vector <CEGUI::Combobox*> m_playerTeamBoxes;

	RTT::MatchDescription m_currentMatch;

	void Update(double timeSinceLastFrame);

	void ProcessCallback(RTT::CallbackChange *);

private:
	bool m_quit;
	bool m_inMatch;
	bool m_init;
	GUIState m_location;

protected:
	void TeamChangedEvent(RTT::MainLobbyCallbackChange *change);
	void TeamColorChangedEvent(RTT::MainLobbyCallbackChange *change);
	void MapChangedEvent(RTT::MainLobbyCallbackChange *change);
	void GamespeedChangedEvent(RTT::MainLobbyCallbackChange *change);
	void VictoryConditionChangedEvent(RTT::MainLobbyCallbackChange *change);
	void PlayerLeftEvent(RTT::MainLobbyCallbackChange *change);
	void KickedFromMatchEvent(RTT::MainLobbyCallbackChange *change);
	void PlayerJoinedEvent(RTT::MainLobbyCallbackChange *change);
	void MatchStartedEvent(RTT::MainLobbyCallbackChange *change);
	void CallbackClosedEvent(RTT::MainLobbyCallbackChange *change);
	void CallbackErrorEvent(RTT::MainLobbyCallbackChange *change);
	void LeaderChangedEvent(RTT::MainLobbyCallbackChange *change);

	void EnableLeader(bool value);

};

#endif

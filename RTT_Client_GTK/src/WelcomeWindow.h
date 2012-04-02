//============================================================================
// Name        : WelcomeWindow.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : The initial window that launches for RTT_GTK
//============================================================================

#ifndef WELCOMEWINDOW_H_
#define WELCOMEWINDOW_H_

#include <gtkmm.h>
#include <stdint.h>
#include "RTT_Client_GTK.h"
#include "MatchListColumns.h"
#include "PlayerListColumns.h"
#include "TeamComboColumns.h"
#include "CallbackHandler.h"
#include "Player.h"

#include <iostream>
#include <arpa/inet.h>
#include <vector>
#include "boost/date_time/posix_time/posix_time.hpp"
#include <openssl/sha.h>

using namespace std;
using namespace Gtk;
using boost::posix_time::ptime;
using boost::posix_time::time_duration;
using boost::gregorian::date;

namespace RTT
{

class WelcomeWindow : public Gtk::Window
{
public:
	WelcomeWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade);
	~WelcomeWindow();

	Glib::RefPtr<Builder> m_welcome_builder;

	CallbackHandler *m_callbackHandler;

	//The three Welcome Window panes
	Box *m_welcome_box;
	Box *m_lobby_box;
	Box *m_match_lobby_box;

	//First Pane Widgets
	Button *m_button_custom;
	Button *m_button_connect;
	Statusbar *m_statusbar;
	Box *m_box_custom;
	Entry *m_entry_IP;
	Entry *m_entry_port;
	Entry *m_entry_username;
	Entry *m_entry_password;

	//Second Pane Widgets
	Button *m_quit_server_button;
	Button *m_create_match_button;
	Button *m_create_match_submit;
	Button *m_list_matches_button;
	Button *m_join_match_button;
	Entry *m_match_name_entry;
	ComboBoxText *m_create_match_map_combo;
	ComboBoxText *m_max_players_combo;
	CheckButton *m_set_private_check;
	Statusbar *m_status_lobby;
	Box *m_create_match_box;
	Notebook *m_match_lists;

	//Third Pane Widgets
	Button *m_leave_match_button;
	Button *m_launch_match_button;
	Statusbar *m_match_lobby_status;
	TreeView *m_player_list_view;
	ComboBoxText *m_map_name_combo;
	ComboBoxText *m_speed_combo;
	ComboBoxText *m_win_condition_combo;
	Label *m_speed_label;
	Label *m_victory_cond_label;
	Label *m_map_set_label;
	Label *m_map_size_label;

	PlayerListColumns *m_playerColumns;
	Glib::RefPtr<ListStore> m_playerListStore;

	TeamComboColumns *m_teamNumberColumns;
	Glib::RefPtr<Gtk::ListStore> m_teamNumberListStore;

	PlayerDescription m_playerDescription;
	MatchDescription m_currentMatch;

	void on_teamNumber_combo_changed(const Glib::ustring&, const Gtk::TreeIter&);
	void connect_click();
	void quit_server_click();
	void create_match_click();
	void create_match_submit_click();
	void join_match_click();
	void list_matches_click();
	void leave_match_click();
	void custom_server_click();
	void launch_match_click();
	void on_leader_toggled(const Glib::ustring& path);
	void speed_combo_changed();
	void victory_combo_changed();
	void map_combo_changed();

	void list_matches();
	//Swaps out the widgets which are only used when we are the match leader
	// isLeader: True is we are the leader, false if not
	void swap_leader_widgets(bool isLeader);

	void LaunchMainLobbyPane();
	void LaunchServerConnectPane();
	void LaunchMatchLobbyPane(PlayerDescription *playerDescriptions, uint playerCount);

	Glib::RefPtr<Gtk::ListStore> PopulateTeamNumberCombo();

	bool GetPasswordTerminal(string plaintext, unsigned char *hash);

protected:

	void TeamChangedEvent();
	void TeamColorChangedEvent();
	void MapChangedEvent();
	void GamespeedChangedEvent();
	void VictoryConditionChangedEvent();
	void PlayerLeftEvent();
	void KickedFromMatchEvent();
	void PlayerJoinedEvent();
	void LeaderChangedEvent();
	void MatchStartedEvent();
	void CallbackClosedEvent();
	void CallbackErrorEvent();

};

}
#endif /* WELCOMEWINDOW_H_ */

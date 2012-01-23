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
#include "ClientProtocolHandler.h"
#include "RTT_Client_GTK.h"
#include "MatchListColumns.h"
#include "PlayerListColumns.h"
#include "TeamComboColumns.h"
#include <iostream>
#include <arpa/inet.h>
#include <vector>
#include <pthread.h>
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

	pthread_rwlock_t globalLock;
	Glib::RefPtr<Builder> welcome_builder;

	//The three Welcome Window panes
	Box *welcome_box;
	Box *lobby_box;
	Box *match_lobby_box;

	//First Pane Widgets
	Button *button_custom;
	Button *button_connect;
	Statusbar *statusbar;
	Box *box_custom;
	Entry *entry_IP;
	Entry *entry_port;
	Entry *entry_username;
	Entry *entry_password;

	//Second Pane Widgets
	Button *quit_server_button;
	Button *create_match_button;
	Button *create_match_submit;
	Button *list_matches_button;
	Button *join_match_button;
	Entry *match_name_entry;
	ComboBoxText *create_match_map_combo;
	ComboBoxText *max_players_combo;
	CheckButton *set_private_check;
	Statusbar *status_lobby;
	Box *create_match_box;
	Notebook *match_lists;

	//Third Pane Widgets
	Button *leave_match_button;
	Statusbar *match_lobby_status;
	TreeView *player_list_view;

	PlayerListColumns *playerColumns;
	Glib::RefPtr<ListStore> playerListStore;

	TeamComboColumns *teamNumberColumns;
	Glib::RefPtr<Gtk::ListStore> teamNumberListStore;

	pthread_t threadID;

	PlayerDescription playerDescription;

	void on_teamNumber_combo_changed(const Glib::ustring&, const Gtk::TreeIter&);
	void connect_click();
	void quit_server_click();
	void create_match_click();
	void create_match_submit_click();
	void join_match_click();
	void list_matches_click();
	void leave_match_click();
	void custom_server_click();

	void list_matches();

	void LaunchMainLobbyPane();
	void LaunchServerConnectPane();
	void LaunchMatchLobbyPane(PlayerDescription *playerDescriptions, uint playerCount);

	Glib::RefPtr<Gtk::ListStore> PopulateTeamNumberCombo();

	bool GetPasswordTerminal(string plaintext, unsigned char *hash);
};

}
#endif /* WELCOMEWINDOW_H_ */

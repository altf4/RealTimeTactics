//============================================================================
// Name        : RTT_Client_GTK.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : GTK Interface to the Client_Core
//============================================================================

#include "WelcomeWindow.h"
#include "messaging/MessageManager.h"

#include "gtkmm.h"

using namespace Gtk;
using namespace RTT;
using namespace std;

Glib::RefPtr<Gtk::Builder> refBuilder;
WelcomeWindow *window;

int main( int argc, char **argv)
{
	MessageManager::Initialize(DIRECTION_TO_SERVER);

	Main kit(argc, argv);

	refBuilder = Gtk::Builder::create();
	try
	{
		refBuilder->add_from_file(WELCOME_WINDOW_GLADE_PATH_USRSHARE);
	}
	catch(Glib::FileError error)
	{
		cerr << "WARNING: WelcomeWindow.glade not found in /usr/share/RTT/GTK/UI/";
		try
		{
			refBuilder->add_from_file(WELCOME_WINDOW_GLADE_PATH_RELATIVE);
		}
		catch(Glib::FileError error)
		{
			cerr << "ERROR: WelcomeWindow.glade also not found in relative path UI/";
			exit(EXIT_FAILURE);
		}
	}

	refBuilder->get_widget_derived("window_welcome", window);

	InitWidgets();

	window->m_button_custom->signal_clicked().connect(
			sigc::mem_fun(*window, &WelcomeWindow::custom_server_click));
	window->m_button_connect->signal_clicked().connect(
			sigc::mem_fun(*window, &WelcomeWindow::connect_click));
	window->m_quit_server_button->signal_clicked().connect(
			sigc::mem_fun(*window, &WelcomeWindow::quit_server_click));
	window->m_create_match_button->signal_clicked().connect(
			sigc::mem_fun(*window, &WelcomeWindow::create_match_click));
	window->m_create_match_submit->signal_clicked().connect(
			sigc::mem_fun(*window, &WelcomeWindow::create_match_submit_click));
	window->m_list_matches_button->signal_clicked().connect(
			sigc::mem_fun(*window, &WelcomeWindow::list_matches_click));
	window->m_join_match_button->signal_clicked().connect(
			sigc::mem_fun(*window, &WelcomeWindow::join_match_click));
	window->m_leave_match_button->signal_clicked().connect(
			sigc::mem_fun(*window, &WelcomeWindow::leave_match_click));
	window->m_speed_combo->signal_changed().connect(
			sigc::mem_fun(*window, &WelcomeWindow::speed_combo_changed));
	window->m_win_condition_combo->signal_changed().connect(
			sigc::mem_fun(*window, &WelcomeWindow::victory_combo_changed));
	window->m_map_name_combo->signal_changed().connect(
			sigc::mem_fun(*window, &WelcomeWindow::map_combo_changed));
	window->m_launch_match_button->signal_clicked().connect(
			sigc::mem_fun(*window, &WelcomeWindow::launch_match_click));

	Main::run(*window);
}

void InitWidgets()
{
	refBuilder->get_widget("button_custom", window->m_button_custom);
	refBuilder->get_widget("button_connect", window->m_button_connect);
	refBuilder->get_widget("status_main", window->m_statusbar);
	refBuilder->get_widget("box_custom", window->m_box_custom);
	refBuilder->get_widget("entry_IP", window->m_entry_IP);
	refBuilder->get_widget("entry_port", window->m_entry_port);
	refBuilder->get_widget("entry_username", window->m_entry_username);
	refBuilder->get_widget("entry_password", window->m_entry_password);
	refBuilder->get_widget("welcome_box", window->m_welcome_box);
	refBuilder->get_widget("lobby_box", window->m_lobby_box);
	refBuilder->get_widget("match_lobby_box", window->m_match_lobby_box);
	refBuilder->get_widget("quit_server_button", window->m_quit_server_button);
	refBuilder->get_widget("create_match_button", window->m_create_match_button);
	refBuilder->get_widget("create_match_submit", window->m_create_match_submit);
	refBuilder->get_widget("list_matches_button", window->m_list_matches_button);
	refBuilder->get_widget("match_name_entry", window->m_match_name_entry);
	refBuilder->get_widget("create_match_map_combo", window->m_create_match_map_combo);
	refBuilder->get_widget("max_players_combo", window->m_max_players_combo);
	refBuilder->get_widget("set_private_check", window->m_set_private_check);
	refBuilder->get_widget("status_lobby", window->m_status_lobby);
	refBuilder->get_widget("create_match_box", window->m_create_match_box);
	refBuilder->get_widget("match_lists", window->m_match_lists);
	refBuilder->get_widget("join_match_button", window->m_join_match_button);
	refBuilder->get_widget("leave_match_button", window->m_leave_match_button);
	refBuilder->get_widget("match_lobby_status", window->m_match_lobby_status);
	refBuilder->get_widget("player_list_view", window->m_player_list_view);
	refBuilder->get_widget("speed_combo", window->m_speed_combo);
	refBuilder->get_widget("speed_label", window->m_speed_label);
	refBuilder->get_widget("victory_cond_label", window->m_victory_cond_label);
	refBuilder->get_widget("map_set_label", window->m_map_set_label);
	refBuilder->get_widget("win_condition_combo", window->m_win_condition_combo);
	refBuilder->get_widget("map_name_combo", window->m_map_name_combo);
	refBuilder->get_widget("map_size_label", window->m_map_size_label);
	refBuilder->get_widget("launch_match_button", window->m_launch_match_button);
}

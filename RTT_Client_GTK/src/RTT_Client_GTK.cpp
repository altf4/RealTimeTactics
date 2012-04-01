//============================================================================
// Name        : RTT_Client_GTK.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : GTK Interface to the Client_Core
//============================================================================

#include "WelcomeWindow.h"
#include "gtkmm.h"

using namespace Gtk;

Glib::RefPtr<Gtk::Builder> refBuilder;
WelcomeWindow *window;

int main( int argc, char **argv)
{
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

	window->button_custom->signal_clicked().connect(
			sigc::mem_fun(*window, &WelcomeWindow::custom_server_click));
	window->button_connect->signal_clicked().connect(
			sigc::mem_fun(*window, &WelcomeWindow::connect_click));
	window->quit_server_button->signal_clicked().connect(
			sigc::mem_fun(*window, &WelcomeWindow::quit_server_click));
	window->create_match_button->signal_clicked().connect(
			sigc::mem_fun(*window, &WelcomeWindow::create_match_click));
	window->create_match_submit->signal_clicked().connect(
			sigc::mem_fun(*window, &WelcomeWindow::create_match_submit_click));
	window->list_matches_button->signal_clicked().connect(
			sigc::mem_fun(*window, &WelcomeWindow::list_matches_click));
	window->join_match_button->signal_clicked().connect(
			sigc::mem_fun(*window, &WelcomeWindow::join_match_click));
	window->leave_match_button->signal_clicked().connect(
			sigc::mem_fun(*window, &WelcomeWindow::leave_match_click));
	window->speed_combo->signal_changed().connect(
			sigc::mem_fun(*window, &WelcomeWindow::speed_combo_changed));
	window->win_condition_combo->signal_changed().connect(
			sigc::mem_fun(*window, &WelcomeWindow::victory_combo_changed));
	window->map_name_combo->signal_changed().connect(
			sigc::mem_fun(*window, &WelcomeWindow::map_combo_changed));
	window->launch_match_button->signal_clicked().connect(
			sigc::mem_fun(*window, &WelcomeWindow::launch_match_click));

	Main::run(*window);
}

void InitWidgets()
{
	refBuilder->get_widget("button_custom", window->button_custom);
	refBuilder->get_widget("button_connect", window->button_connect);
	refBuilder->get_widget("status_main", window->statusbar);
	refBuilder->get_widget("box_custom", window->box_custom);
	refBuilder->get_widget("entry_IP", window->entry_IP);
	refBuilder->get_widget("entry_port", window->entry_port);
	refBuilder->get_widget("entry_username", window->entry_username);
	refBuilder->get_widget("entry_password", window->entry_password);
	refBuilder->get_widget("welcome_box", window->welcome_box);
	refBuilder->get_widget("lobby_box", window->lobby_box);
	refBuilder->get_widget("match_lobby_box", window->match_lobby_box);
	refBuilder->get_widget("quit_server_button", window->quit_server_button);
	refBuilder->get_widget("create_match_button", window->create_match_button);
	refBuilder->get_widget("create_match_submit", window->create_match_submit);
	refBuilder->get_widget("list_matches_button", window->list_matches_button);
	refBuilder->get_widget("match_name_entry", window->match_name_entry);
	refBuilder->get_widget("create_match_map_combo", window->create_match_map_combo);
	refBuilder->get_widget("max_players_combo", window->max_players_combo);
	refBuilder->get_widget("set_private_check", window->set_private_check);
	refBuilder->get_widget("status_lobby", window->status_lobby);
	refBuilder->get_widget("create_match_box", window->create_match_box);
	refBuilder->get_widget("match_lists", window->match_lists);
	refBuilder->get_widget("join_match_button", window->join_match_button);
	refBuilder->get_widget("leave_match_button", window->leave_match_button);
	refBuilder->get_widget("match_lobby_status", window->match_lobby_status);
	refBuilder->get_widget("player_list_view", window->player_list_view);
	refBuilder->get_widget("speed_combo", window->speed_combo);
	refBuilder->get_widget("speed_label", window->speed_label);
	refBuilder->get_widget("victory_cond_label", window->victory_cond_label);
	refBuilder->get_widget("map_set_label", window->map_set_label);
	refBuilder->get_widget("win_condition_combo", window->win_condition_combo);
	refBuilder->get_widget("map_name_combo", window->map_name_combo);
	refBuilder->get_widget("map_size_label", window->map_size_label);
	refBuilder->get_widget("launch_match_button", window->launch_match_button);
}

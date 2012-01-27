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
	refBuilder->add_from_file("UI/WelcomeWindow.glade");
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

	pthread_rwlock_init(&window->globalLock, NULL);

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

}

void *CallbackThread(void * parm)
{
	while(true)
	{
		struct CallbackChange change = ProcessCallbackCommand();
		switch( change.type )
		{
			case TEAM_CHANGE:
			{
				pthread_rwlock_wrlock(&window->globalLock);
				PlayerListColumns playerColumns;
				TeamComboColumns teamColumns;

				TreeModel::Children rows = window->playerListStore->children();
				TreeModel::iterator r;
				for(r=rows.begin(); r!=rows.end(); r++)
				{
					TreeModel::Row playerRow=*r;
					int ID = playerRow[playerColumns.ID];
					if( ID == (int)change.playerID )
					{
						playerRow[playerColumns.teamName] =
								Team::TeamNumberToString((enum TeamNumber)change.team);

//						//Get set the new team number back into the combobox's data
//						TreeValueProxy<Glib::RefPtr<TreeModel> > teamNumberListStore =
//								playerRow[playerColumns.teamChosen];
//						Glib::RefPtr<TreeModel> treeModelPtr = teamNumberListStore;
//						TreeModel::iterator chosenTeamIter = treeModelPtr->get_iter("0");
//						TreeModel::Row existingTeamRow = (*chosenTeamIter);
//						existingTeamRow[teamColumns.teamNum] = change.team;
//						existingTeamRow[teamColumns.teamString] =
//								Team::TeamNumberToString((enum TeamNumber)change.team);

						window->player_list_view->show_all();
						break;
					}
				}
				pthread_rwlock_unlock(&window->globalLock);

			}
			case COLOR_CHANGE:
			{
				pthread_rwlock_wrlock(&window->globalLock);
				//Do stuff here
				pthread_rwlock_unlock(&window->globalLock);
				break;
			}
			case MAP_CHANGE:
			{
				pthread_rwlock_wrlock(&window->globalLock);
				//Do stuff here
				pthread_rwlock_unlock(&window->globalLock);
				break;
			}
			case SPEED_CHANGE:
			{
				pthread_rwlock_wrlock(&window->globalLock);
				//Do stuff here
				pthread_rwlock_unlock(&window->globalLock);
				break;
			}
			case VICTORY_CHANGE:
			{
				pthread_rwlock_wrlock(&window->globalLock);
				//Do stuff here
				pthread_rwlock_unlock(&window->globalLock);
				break;
			}
			case PLAYER_LEFT:
			{
				pthread_rwlock_wrlock(&window->globalLock);
				TreeModel::Children rows = window->playerListStore->children();
				TreeModel::iterator r;
				for(r=rows.begin(); r!=rows.end(); r++)
				{
					TreeModel::Row row=*r;
					uint ID = row[window->playerColumns->ID];
					if( ID == change.playerID)
					{
						window->playerListStore->erase(r);
					}
				}
				for(r=rows.begin(); r!=rows.end(); r++)
				{
					TreeModel::Row row=*r;
					uint ID = row[window->playerColumns->ID];
					if( ID == change.newLeaderID )
					{
						row[window->playerColumns->isLeader] = true;
					}
				}
				window->currentMatch.leaderID = change.newLeaderID;
				window->player_list_view->show_all();
				pthread_rwlock_unlock(&window->globalLock);
				break;
			}
			case KICKED:
			{
				return NULL;
			}
			case PLAYER_JOINED:
			{
				if( window->playerDescription.ID !=	change.playerDescription.ID)
				{
					PlayerListColumns playerColumns;

					pthread_rwlock_wrlock(&window->globalLock);
					//Add a new row for the new player
					TreeModel::Row row = *(window->playerListStore->append());
					row[playerColumns.name] = change.playerDescription.name;
					row[playerColumns.teamChosen] =
							window->PopulateTeamNumberCombo();
					row[playerColumns.teamName] =
							Team::TeamNumberToString((enum TeamNumber)change.team);
					row[window->playerColumns->ID] = change.playerDescription.ID;
					row[window->playerColumns->isLeader] = false;
					window->player_list_view->show_all();

					pthread_rwlock_unlock(&window->globalLock);
				}
				break;
			}
			case LEADER_CHANGE:
			{
				pthread_rwlock_wrlock(&window->globalLock);
				window->currentMatch.leaderID = change.playerID;
				TreeModel::Children rows = window->playerListStore->children();
				TreeModel::iterator r;
				for(r=rows.begin(); r!=rows.end(); r++)
				{
					TreeModel::Row row=*r;
					uint ID = row[window->playerColumns->ID];
					if( ID == change.playerID)
					{
						row[window->playerColumns->isLeader] = true;
					}
					else
					{
						row[window->playerColumns->isLeader] = false;
					}
				}
				window->player_list_view->show_all();
				pthread_rwlock_unlock(&window->globalLock);
				break;
			}
			case MATCH_STARTED:
			{
				//TODO: Start the game!!!
				cout << "Game is starting in 3... 2... 1...\n";
				break;
			}
			case CALLBACK_ERROR:
			{
				cerr << "ERROR: Callback receive failed\n";
				break;
			}
			default:
			{
				cerr << "ERROR: Got a bad type from callback. Shouldn't get here\n";
				break;
			}
		}


	}
}

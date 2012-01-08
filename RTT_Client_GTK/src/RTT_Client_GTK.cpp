//============================================================================
// Name        : RTT_Client_GTK.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : GTK Interface to the Client_Core
//============================================================================

#include "ClientProtocolHandler.h"
#include "RTT_Client_GTK.h"
#include "MatchListColumns.h"
#include "PlayerListColumns.h"
#include "TeamComboColumns.h"
#include <iostream>
#include <gtkmm.h>
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
using namespace RTT;

Glib::RefPtr<Builder> welcome_builder;
ptime epoch(date(1970,boost::gregorian::Jan,1));

Window *welcome_window = NULL;

//The three Welcome Window panes
Box *welcome_box = NULL;
Box *lobby_box = NULL;
Box *match_lobby_box = NULL;

//First Pane Widgets
Button *button_custom = NULL;
Button *button_connect = NULL;
Statusbar *statusbar = NULL;
Box *box_custom = NULL;
Entry *entry_IP = NULL;
Entry *entry_port = NULL;
Entry *entry_username = NULL;
Entry *entry_password = NULL;

//Second Pane Widgets
Button *quit_server_button = NULL;
Button *create_match_button = NULL;
Button *create_match_submit = NULL;
Button *list_matches_button = NULL;
Button *join_match_button = NULL;
Entry *match_name_entry = NULL;
ComboBoxText *create_match_map_combo = NULL;
ComboBoxText *max_players_combo = NULL;
CheckButton *set_private_check = NULL;
Statusbar *status_lobby = NULL;
Box *create_match_box = NULL;
Notebook *match_lists = NULL;

//Third Pane Widgets
Button *leave_match_button = NULL;
Statusbar *match_lobby_status = NULL;
TreeView *player_list_view = NULL;

PlayerListColumns *playerColumns;
Glib::RefPtr<ListStore> playerListStore;

TeamComboColumns *teamNumberColumns;
Glib::RefPtr<Gtk::ListStore> teamNumberListStore;

pthread_rwlock_t globalLock;
pthread_t threadID;

PlayerDescription playerDescription;

void custom_server_click()
{
	pthread_rwlock_wrlock(&globalLock);

	statusbar->push("Set server settings, then hit Connect");
	box_custom->set_visible(true);

	pthread_rwlock_unlock(&globalLock);
}

void connect_click()
{
	pthread_rwlock_wrlock(&globalLock);

	statusbar->push("Trying to connect...");

	//A little bit of input validation here
	string serverIP = entry_IP->get_text();

	struct sockaddr_in stSockAddr;
	int Res = inet_pton(AF_INET, serverIP.c_str(), &stSockAddr.sin_addr);
	if (Res == 0)
	{
		statusbar->push("Invalid IP address");
		pthread_rwlock_unlock(&globalLock);
		return;
	}

	char *errString;
	uint serverPort = strtoul(entry_port->get_text().c_str(), &errString, 10);
	if( *errString != '\0' || entry_port->get_text().c_str() == '\0')
	{
		//Error occurred
		statusbar->push("Invalid port number");
		pthread_rwlock_unlock(&globalLock);
		return;
	}

	string givenName = entry_username->get_text();
	string hashedPassword = entry_password->get_text();

	int SocketFD = AuthToServer(serverIP, serverPort,
			givenName, (unsigned char*)hashedPassword.c_str(), &playerDescription);

	if( SocketFD > 0 )
	{
		statusbar->push("Connection Successful!");
		LaunchMainLobbyPane();

		//Launch the Callback Thread
		pthread_create(&threadID, NULL, CallbackThread, NULL);
	}
	else
	{
		statusbar->push("Failed to connect to server");
	}

	pthread_rwlock_unlock(&globalLock);
}

void create_match_submit_click()
{
	pthread_rwlock_wrlock(&globalLock);

	string matchName = match_name_entry->get_text();
	if(matchName.size() < 1 && matchName.size() > 20)
	{
		status_lobby->push("Invalid match name length");
		pthread_rwlock_unlock(&globalLock);
		return;
	}

	if(create_match_map_combo->get_active_row_number() == -1)
	{
		status_lobby->push("Please select a map");
		pthread_rwlock_unlock(&globalLock);
		return;
	}
	string mapName = create_match_map_combo->get_active_text();

	int maxPlayers = max_players_combo->get_active_row_number();
	if( maxPlayers == -1)
	{
		status_lobby->push("Please select a maximum number of players");
		pthread_rwlock_unlock(&globalLock);
		return;
	}

	//TODO: Use this once private matches are implemented
	//bool privateMatch = set_private_check->get_active();

	struct MatchOptions options;
	options.maxPlayers = maxPlayers + 2; //+2 since the combo starts at 2
	strncpy(options.name, match_name_entry->get_text().c_str(), sizeof(options.name));

	if (CreateMatch(options) )
	{
		LaunchMatchLobbyPane(&playerDescription, 1);
	}
	else
	{
		status_lobby->push("Server returned failure to create match");
		pthread_rwlock_unlock(&globalLock);
		return;
	}

	pthread_rwlock_unlock(&globalLock);
}

void create_match_click()
{
	pthread_rwlock_wrlock(&globalLock);

	create_match_box->set_visible(true);
	match_lists->set_visible(false);

	pthread_rwlock_unlock(&globalLock);
}

//Refresh the match list
void list_matches_click()
{
	pthread_rwlock_wrlock(&globalLock);

	list_matches();

	pthread_rwlock_unlock(&globalLock);
}

void list_matches()
{
	create_match_box->set_visible(false);
	match_lists->set_visible(true);

	int page = match_lists->get_current_page();

	//If no page is selected, then put us to page 1
	if( page == -1)
	{
		page = 1;
	}

	struct ServerStats stats = GetServerStats();

	//Determine how many pages we need to display these
	//	x / y (rounding up) = (x + y - 1) / y
	uint pagesNeeded = (stats.numMatches + MATCHES_PER_PAGE - 1) / MATCHES_PER_PAGE;
	if( pagesNeeded == 0 )
	{
		//We always want at least one page
		pagesNeeded = 1;
	}

	//If trying to select more than we have, select the last page
	if( page > (int)pagesNeeded-1 )
	{
		page = pagesNeeded-1;
	}

	//Remove all the existing pages
	for(int i = 0; i < match_lists->get_n_pages(); i++)
	{
		match_lists->remove_page();
	}
	//Put the new ones in
	TreeView *view;
	for(uint i = 0; i < pagesNeeded; i++)
	{
		view = manage(new TreeView());
		match_lists->append_page(*view);
	}

	//Populate the view we're currently selecting
	struct MatchDescription descriptions[MATCHES_PER_PAGE];
	uint numMatchesThisPage = ListMatches(page+1, descriptions);

	match_lists->set_current_page(page);

	MatchListColumns *columns = new MatchListColumns();

	Glib::RefPtr<ListStore> refListStore = ListStore::create(*columns);
	view->set_model(refListStore);

	for(uint i = 0; i < numMatchesThisPage; i++)
	{
		TreeModel::Row row = *(refListStore->append());
		row[columns->matchID] = (int)descriptions[i].ID;
		row[columns->maxPlayers] = (int)descriptions[i].maxPlayers;
		row[columns->currentPlayers] = (int)descriptions[i].currentPlayerCount;
		row[columns->name] = descriptions[i].name;

		ptime time = epoch + boost::posix_time::seconds(descriptions[i].timeCreated);
		string timeString = to_simple_string(time);

		row[columns->timeCreated] = timeString;
	}

	view->append_column("ID", columns->matchID);
	view->append_column("Name", columns->name);
	view->append_column("Current Players", columns->currentPlayers);
	view->append_column("Max Players", columns->maxPlayers);
	view->append_column("Time Created", columns->timeCreated);

	match_lists->show_all();
}

//Hides other windows (WelcomeWindow) and shows LobbyWindow
void LaunchMainLobbyPane()
{
	welcome_box->set_visible(false);
	lobby_box->set_visible(true);
	match_lobby_box->set_visible(false);

	//By default, show the match list first
	list_matches();
}

void LaunchServerConnectPane()
{
	welcome_box->set_visible(true);
	lobby_box->set_visible(false);
	match_lobby_box->set_visible(false);
}

void PopulateTeamNumberCombo()
{
	TreeModel::Row row = *(teamNumberListStore->append());
	row[teamNumberColumns->teamNum] = "TEAM_1";
	row[teamNumberColumns->teamString] = "Team 1";
	row = *(teamNumberListStore->append());
	row[teamNumberColumns->teamNum] = "TEAM_2";
	row[teamNumberColumns->teamString] = "Team 2";
	row = *(teamNumberListStore->append());
	row[teamNumberColumns->teamNum] = "TEAM_3";
	row[teamNumberColumns->teamString] = "Team 3";
	row = *(teamNumberListStore->append());
	row[teamNumberColumns->teamNum] = "TEAM_4";
	row[teamNumberColumns->teamString] = "Team 4";
	row = *(teamNumberListStore->append());
	row[teamNumberColumns->teamNum] = "TEAM_5";
	row[teamNumberColumns->teamString] = "Team 5";
	row = *(teamNumberListStore->append());
	row[teamNumberColumns->teamNum] = "TEAM_6";
	row[teamNumberColumns->teamString] = "Team 6";
	row = *(teamNumberListStore->append());
	row[teamNumberColumns->teamNum] = "TEAM_7";
	row[teamNumberColumns->teamString] = "Team 7";
	row = *(teamNumberListStore->append());
	row[teamNumberColumns->teamNum] = "TEAM_8";
	row[teamNumberColumns->teamString] = "Team 8";
	row = *(teamNumberListStore->append());
	row[teamNumberColumns->teamNum] = "REFEREE";
	row[teamNumberColumns->teamString] = "Referee";
	row = *(teamNumberListStore->append());
	row[teamNumberColumns->teamNum] = "SPECTATOR";
	row[teamNumberColumns->teamString] = "Spectator";
}

void LaunchMatchLobbyPane(PlayerDescription *playerDescriptions, uint playerCount)
{
	welcome_box->set_visible(false);
	lobby_box->set_visible(false);
	match_lobby_box->set_visible(true);

	//Clear out anything in the Player ListView
	player_list_view->remove_all_columns();
	player_list_view->unset_model();

	playerColumns = new PlayerListColumns();
	playerListStore = ListStore::create(*playerColumns);
	player_list_view->set_model(playerListStore);

	teamNumberColumns = new TeamComboColumns();
	teamNumberListStore = Gtk::ListStore::create(*teamNumberColumns);
	PopulateTeamNumberCombo();

	//Add a new row (for ourselves)
	for(uint i = 0; i < playerCount; i++)
	{
		TreeModel::Row row = *(playerListStore->append());
		row[playerColumns->name] = string(playerDescriptions[i].name);
		row[playerColumns->ID] = playerDescriptions[i].ID;
		row[playerColumns->teamChosen] = teamNumberListStore;
		row[playerColumns->teamName] = string("Pick One!");
	}

	player_list_view->append_column("Name", playerColumns->name);

	TreeView::Column* pColumn = manage( new Gtk::TreeView::Column("Team") );
	CellRendererCombo* pRenderer = manage(	new CellRendererCombo());
	pColumn->pack_start(*pRenderer);
	player_list_view->append_column(*pColumn);

	pRenderer->property_model() = teamNumberListStore;
	pColumn->add_attribute(pRenderer->property_text(), playerColumns->teamName);
	pColumn->add_attribute(pRenderer->property_model(), playerColumns->teamChosen);

	pRenderer->property_text_column() = 0;
	pRenderer->property_editable() = true;
	pRenderer->property_has_entry() = false;

//	m_button1.signal_clicked().connect( sigc::bind<Glib::ustring>( sigc::mem_fun(*this, &HelloWorld::on_button_clicked), "button 1") );
//	pRenderer->signal_changed().connect( sigc::ptr_fun(teamNumberEdited) );

	player_list_view->show_all();
}

void teamNumberEdited(const Glib::ustring& new_text)
{
	Gtk::TreePath path(path_string);

	//Get the row from the path:
	Gtk::TreeModel::iterator iter = teamNumberListStore->get_iter(path);
	if(iter)
	{
		//Store the user's new text in the model:
		Gtk::TreeRow row = *iter;
//		row[m_Columns.m_col_itemchosen] = new_text;
	}
}

void leave_match_click()
{
	pthread_rwlock_wrlock(&globalLock);

	if( LeaveMatch() )
	{
		LaunchMainLobbyPane();
	}
	else
	{
		match_lobby_status->push("Error on server, couldn't leave match");
	}

	pthread_rwlock_unlock(&globalLock);
}

void join_match_click()
{
	pthread_rwlock_wrlock(&globalLock);
	int page = match_lists->get_current_page();
	if( page == -1 )
	{
		status_lobby->push("Please select a match, and try again");
		pthread_rwlock_unlock(&globalLock);
		return;
	}

	TreeView *view = (TreeView*)match_lists->get_nth_page( page );
	if( view == NULL )
	{
		status_lobby->push("Please select a match, and try again");
		pthread_rwlock_unlock(&globalLock);
		return;
	}

	MatchListColumns columns;
	Glib::RefPtr<Gtk::TreeSelection> select = view->get_selection();
	if( select->count_selected_rows() != 1)
	{
		status_lobby->push("Please select a match, and try again");
		pthread_rwlock_unlock(&globalLock);
		return;
	}
	TreeModel::iterator iter = select->get_selected();
	TreeModel::Row row = *( iter );
	int matchID = row[columns.matchID];

	PlayerDescription playerDescriptions[MAX_PLAYERS_IN_MATCH];

	uint playerCount = JoinMatch(matchID, playerDescriptions);
	if( playerCount > 0 )
	{
		LaunchMatchLobbyPane(playerDescriptions, playerCount);
	}
	else
	{
		status_lobby->push("Failed to join match. Is it full?");
		pthread_rwlock_unlock(&globalLock);
		return;
	}

	pthread_rwlock_unlock(&globalLock);
}

void quit_server_click()
{
	pthread_rwlock_wrlock(&globalLock);

	ExitServer();
	LaunchServerConnectPane();

	pthread_cancel(threadID);

	pthread_rwlock_unlock(&globalLock);
}

void InitGlobalWidgets()
{
	welcome_builder->get_widget("window_welcome", welcome_window);
	welcome_builder->get_widget("button_custom", button_custom);
	welcome_builder->get_widget("button_connect", button_connect);
	welcome_builder->get_widget("status_main", statusbar);
	welcome_builder->get_widget("box_custom", box_custom);
	welcome_builder->get_widget("entry_IP", entry_IP);
	welcome_builder->get_widget("entry_port", entry_port);
	welcome_builder->get_widget("entry_username", entry_username);
	welcome_builder->get_widget("entry_password", entry_password);

	welcome_builder->get_widget("welcome_box", welcome_box);
	welcome_builder->get_widget("lobby_box", lobby_box);
	welcome_builder->get_widget("match_lobby_box", match_lobby_box);

	welcome_builder->get_widget("quit_server_button", quit_server_button);
	welcome_builder->get_widget("create_match_button", create_match_button);
	welcome_builder->get_widget("create_match_submit", create_match_submit);
	welcome_builder->get_widget("list_matches_button", list_matches_button);
	welcome_builder->get_widget("match_name_entry", match_name_entry);
	welcome_builder->get_widget("create_match_map_combo", create_match_map_combo);
	welcome_builder->get_widget("max_players_combo", max_players_combo);
	welcome_builder->get_widget("set_private_check", set_private_check);
	welcome_builder->get_widget("status_lobby", status_lobby);
	welcome_builder->get_widget("create_match_box", create_match_box);
	welcome_builder->get_widget("match_lists", match_lists);
	welcome_builder->get_widget("join_match_button", join_match_button);

	welcome_builder->get_widget("leave_match_button", leave_match_button);
	welcome_builder->get_widget("match_lobby_status", match_lobby_status);
	welcome_builder->get_widget("player_list_view", player_list_view);


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
				pthread_rwlock_wrlock(&globalLock);
				//Do stuff here
				pthread_rwlock_unlock(&globalLock);

			}
			case COLOR_CHANGE:
			{
				pthread_rwlock_wrlock(&globalLock);
				//Do stuff here
				pthread_rwlock_unlock(&globalLock);
				break;
			}
			case MAP_CHANGE:
			{
				pthread_rwlock_wrlock(&globalLock);
				//Do stuff here
				pthread_rwlock_unlock(&globalLock);
				break;
			}
			case SPEED_CHANGE:
			{
				pthread_rwlock_wrlock(&globalLock);
				//Do stuff here
				pthread_rwlock_unlock(&globalLock);
				break;
			}
			case VICTORY_CHANGE:
			{
				pthread_rwlock_wrlock(&globalLock);
				//Do stuff here
				pthread_rwlock_unlock(&globalLock);
				break;
			}
			case PLAYER_LEFT:
			{
				pthread_rwlock_wrlock(&globalLock);
				TreeModel::Children rows = playerListStore->children();
				TreeModel::iterator r;
				for(r=rows.begin(); r!=rows.end(); r++)
				{
					TreeModel::Row row=*r;
					uint ID = row[playerColumns->ID];
					if( ID == change.playerID)
					{
						playerListStore->erase(r);
					}
				}

				pthread_rwlock_unlock(&globalLock);
				break;
			}
			case KICKED:
			{
				return NULL;
			}
			case PLAYER_JOINED:
			{
				if( playerDescription.ID !=	change.playerDescription.ID)
				{
					pthread_rwlock_wrlock(&globalLock);
					//Add a new row (for ourselves)
					TreeModel::Row row = *(playerListStore->append());
					row[playerColumns->name] = change.playerDescription.name;
//					row[playerColumns->team] = change.playerDescription.team;
					row[playerColumns->ID] = change.playerDescription.ID;
					player_list_view->show_all();

					pthread_rwlock_unlock(&globalLock);
				}
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

//Asks the user for their password over terminal
//Sets *hash to the SHA256 hash of the input password
bool GetPasswordTerminal(string plaintext, unsigned char *hash)
{
	//Do the hash
	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	SHA256_Update(&sha256, plaintext.c_str(), plaintext.length());
	SHA256_Final(hash, &sha256);

	return true;
}

int main( int argc, char **argv)
{
	pthread_rwlock_init(&globalLock, NULL);

	Main kit(argc, argv);

	//Initialize widgets used in Welcome Window
	welcome_builder = Builder::create_from_file("UI/WelcomeWindow.glade");

	InitGlobalWidgets();

	button_custom->signal_clicked().connect(sigc::ptr_fun(custom_server_click));
	button_connect->signal_clicked().connect(sigc::ptr_fun(connect_click));
	quit_server_button->signal_clicked().connect(sigc::ptr_fun(quit_server_click));
	create_match_button->signal_clicked().connect(sigc::ptr_fun(create_match_click));
	create_match_submit->signal_clicked().connect(sigc::ptr_fun(create_match_submit_click));
	list_matches_button->signal_clicked().connect(sigc::ptr_fun(list_matches_click));
	join_match_button->signal_clicked().connect(sigc::ptr_fun(join_match_click));

	leave_match_button->signal_clicked().connect(sigc::ptr_fun(leave_match_click));

	Main::run(*welcome_window);
	return EXIT_SUCCESS;
}


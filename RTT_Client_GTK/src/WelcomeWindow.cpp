//============================================================================
// Name        : WelcomeWindow.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : The initial window that launches for RTT_GTK
//============================================================================

#include "WelcomeWindow.h"

using namespace RTT;

WelcomeWindow::WelcomeWindow(BaseObjectType* cobject,
		const Glib::RefPtr<Gtk::Builder>& refBuilder)
: Gtk::Window(cobject)
{

}

WelcomeWindow::~WelcomeWindow()
{

}

void WelcomeWindow::custom_server_click()
{
	pthread_rwlock_wrlock(&globalLock);

	statusbar->push("Set server settings, then hit Connect");
	box_custom->set_visible(true);

	pthread_rwlock_unlock(&globalLock);
}

void WelcomeWindow::connect_click()
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
	}
	else
	{
		statusbar->push("Failed to connect to server");
	}

	pthread_rwlock_unlock(&globalLock);
}

void WelcomeWindow::create_match_submit_click()
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

void WelcomeWindow::create_match_click()
{
	pthread_rwlock_wrlock(&globalLock);

	create_match_box->set_visible(true);
	match_lists->set_visible(false);

	pthread_rwlock_unlock(&globalLock);
}

//Refresh the match list
void WelcomeWindow::list_matches_click()
{
	pthread_rwlock_wrlock(&globalLock);

	list_matches();

	pthread_rwlock_unlock(&globalLock);
}

void WelcomeWindow::leave_match_click()
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

void WelcomeWindow::join_match_click()
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

void WelcomeWindow::quit_server_click()
{
	pthread_rwlock_wrlock(&globalLock);

	ExitServer();
	LaunchServerConnectPane();

	pthread_cancel(threadID);

	pthread_rwlock_unlock(&globalLock);
}

void WelcomeWindow::on_teamNumber_combo_changed(const Glib::ustring& path,
		const Gtk::TreeIter& iter)
{
	pthread_rwlock_wrlock(&globalLock);

	//Get the row of the currently selected player
	Glib::RefPtr<TreeSelection> selection = player_list_view->get_selection();
	TreeModel::iterator selectedIter = selection->get_selected();
	TreeModel::Row playerRow = *(selectedIter);

	PlayerListColumns playerColumns;
	TeamComboColumns teamColumns;

	//Get the row of the selected team (in the combobox, each item is a row)
	TreeModel::Row teamRow = (*iter);
	int newTeam = teamRow[teamColumns.teamNum];

	//Properly set the ComboBox text
	if( newTeam == 0 )
	{
		playerRow[playerColumns.teamName] = "Spectator";
	}
	else if( newTeam == 9 )
	{
		playerRow[playerColumns.teamName] = "Referee";
	}
	else
	{
		stringstream temp;
		temp << "Team " << newTeam;
		playerRow[playerColumns.teamName] = temp.str();
	}

	//Get set the new team number back into the combobox's data
	TreeValueProxy<Glib::RefPtr<TreeModel> > teamNumberListStore =
			playerRow[playerColumns.teamChosen];
	Glib::RefPtr<TreeModel> treeModelPtr = teamNumberListStore;
	TreeModel::iterator chosenTeamIter = treeModelPtr->get_iter(path);
	TreeModel::Row existingTeamRow = (*chosenTeamIter);
	existingTeamRow[teamColumns.teamNum] = newTeam;

	player_list_view->show_all();

	pthread_rwlock_unlock(&globalLock);
}

void WelcomeWindow::list_matches()
{
	ptime epoch(date(1970,boost::gregorian::Jan,1));

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
void WelcomeWindow::LaunchMainLobbyPane()
{
	welcome_box->set_visible(false);
	lobby_box->set_visible(true);
	match_lobby_box->set_visible(false);

	//By default, show the match list first
	list_matches();
}

void WelcomeWindow::LaunchServerConnectPane()
{
	welcome_box->set_visible(true);
	lobby_box->set_visible(false);
	match_lobby_box->set_visible(false);
}

void WelcomeWindow::PopulateTeamNumberCombo()
{
	TreeModel::Row row = *(teamNumberListStore->append());
	row[teamNumberColumns->teamNum] = 1;
	row[teamNumberColumns->teamString] = "Team 1";
	row = *(teamNumberListStore->append());
	row[teamNumberColumns->teamNum] = 2;
	row[teamNumberColumns->teamString] = "Team 2";
	row = *(teamNumberListStore->append());
	row[teamNumberColumns->teamNum] = 3;
	row[teamNumberColumns->teamString] = "Team 3";
	row = *(teamNumberListStore->append());
	row[teamNumberColumns->teamNum] = 4;
	row[teamNumberColumns->teamString] = "Team 4";
	row = *(teamNumberListStore->append());
	row[teamNumberColumns->teamNum] = 5;
	row[teamNumberColumns->teamString] = "Team 5";
	row = *(teamNumberListStore->append());
	row[teamNumberColumns->teamNum] = 6;
	row[teamNumberColumns->teamString] = "Team 6";
	row = *(teamNumberListStore->append());
	row[teamNumberColumns->teamNum] = 7;
	row[teamNumberColumns->teamString] = "Team 7";
	row = *(teamNumberListStore->append());
	row[teamNumberColumns->teamNum] = 8;
	row[teamNumberColumns->teamString] = "Team 8";
	row = *(teamNumberListStore->append());
	row[teamNumberColumns->teamNum] = 9;
	row[teamNumberColumns->teamString] = "Referee";
	row = *(teamNumberListStore->append());
	row[teamNumberColumns->teamNum] = 0;
	row[teamNumberColumns->teamString] = "Spectator";
}

void WelcomeWindow::LaunchMatchLobbyPane(PlayerDescription *playerDescriptions,
		uint playerCount)
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
		row[playerColumns->teamName] = string("Team 1");
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

	pRenderer->signal_changed().connect(
			sigc::mem_fun(*this, &WelcomeWindow::on_teamNumber_combo_changed));

	player_list_view->show_all();
}

//Asks the user for their password over terminal
//Sets *hash to the SHA256 hash of the input password
bool WelcomeWindow::GetPasswordTerminal(string plaintext, unsigned char *hash)
{
	//Do the hash
	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	SHA256_Update(&sha256, plaintext.c_str(), plaintext.length());
	SHA256_Final(hash, &sha256);

	return true;
}

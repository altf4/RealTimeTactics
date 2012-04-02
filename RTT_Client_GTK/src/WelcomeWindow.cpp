//============================================================================
// Name        : WelcomeWindow.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : The initial window that launches for RTT_GTK
//============================================================================

#include "WelcomeWindow.h"
#include "Team.h"
#include "ClientProtocolHandler.h"

using namespace RTT;

WelcomeWindow::WelcomeWindow(BaseObjectType* cobject,
		const Glib::RefPtr<Gtk::Builder>& refBuilder)
: Gtk::Window(cobject)
{
	m_callbackHandler = new CallbackHandler();
	m_callbackHandler->m_sig_team_change.connect(sigc::mem_fun(*this, &WelcomeWindow::TeamChangedEvent));
	m_callbackHandler->m_sig_color_change.connect(sigc::mem_fun(*this, &WelcomeWindow::TeamColorChangedEvent));
	m_callbackHandler->m_sig_map_change.connect(sigc::mem_fun(*this, &WelcomeWindow::MapChangedEvent));
	m_callbackHandler->m_sig_speed_change.connect(sigc::mem_fun(*this, &WelcomeWindow::GamespeedChangedEvent));
	m_callbackHandler->m_sig_victory_cond_change.connect(sigc::mem_fun(*this, &WelcomeWindow::VictoryConditionChangedEvent));
	m_callbackHandler->m_sig_player_left.connect(sigc::mem_fun(*this, &WelcomeWindow::PlayerLeftEvent));
	m_callbackHandler->m_sig_kicked.connect(sigc::mem_fun(*this, &WelcomeWindow::KickedFromMatchEvent));
	m_callbackHandler->m_sig_player_joined.connect(sigc::mem_fun(*this, &WelcomeWindow::PlayerJoinedEvent));
	m_callbackHandler->m_sig_leader_change.connect(sigc::mem_fun(*this, &WelcomeWindow::LeaderChangedEvent));
	m_callbackHandler->m_sig_match_started.connect(sigc::mem_fun(*this, &WelcomeWindow::MatchStartedEvent));
	m_callbackHandler->m_sig_callback_closed.connect(sigc::mem_fun(*this, &WelcomeWindow::CallbackClosedEvent));
	m_callbackHandler->m_sig_callback_error.connect(sigc::mem_fun(*this, &WelcomeWindow::CallbackErrorEvent));
}

WelcomeWindow::~WelcomeWindow()
{

}

//****************************************************************************
//**						Callback Event Handlers							**
//****************************************************************************

void WelcomeWindow::TeamChangedEvent()
{
	struct CallbackChange change = m_callbackHandler->PopCallbackChange();
	if(change.type == CALLBACK_ERROR)
	{
		cerr << "ERROR: Got an error in callback processing" << endl;
		return;
	}
	PlayerListColumns playerColumns;
	TeamComboColumns teamColumns;

	TreeModel::Children rows = playerListStore->children();
	TreeModel::iterator rowIter;
	for(rowIter=rows.begin(); rowIter!=rows.end(); rowIter++)
	{
		if(!rowIter)
		{
			cerr << "ERROR: A player row was corrupt\n";
			continue;
		}
		TreeModel::Row playerRow=*rowIter;
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

			player_list_view->show_all();
			break;
		}
	}
}

void WelcomeWindow::TeamColorChangedEvent()
{
	struct CallbackChange change = m_callbackHandler->PopCallbackChange();
	if(change.type == CALLBACK_ERROR)
	{
		cerr << "ERROR: Got an error in callback processing" << endl;
		return;
	}
	//TODO: Something here!
}

void WelcomeWindow::MapChangedEvent()
{
	struct CallbackChange change = m_callbackHandler->PopCallbackChange();
	if(change.type == CALLBACK_ERROR)
	{
		cerr << "ERROR: Got an error in callback processing" << endl;
		return;
	}
	stringstream ss;
	ss << change.mapDescription.width;
	ss << " x ";
	ss << change.mapDescription.length;
	map_size_label->set_text(ss.str());
}

void WelcomeWindow::GamespeedChangedEvent()
{
	struct CallbackChange change = m_callbackHandler->PopCallbackChange();
	if(change.type == CALLBACK_ERROR)
	{
		cerr << "ERROR: Got an error in callback processing" << endl;
		return;
	}
	speed_label->set_text(Match::GameSpeedToString(change.speed));
}

void WelcomeWindow::VictoryConditionChangedEvent()
{
	struct CallbackChange change = m_callbackHandler->PopCallbackChange();
	if(change.type == CALLBACK_ERROR)
	{
		cerr << "ERROR: Got an error in callback processing" << endl;
		return;
	}
	victory_cond_label->set_text(
			Match::VictoryConditionToString(change.victory));
}

void WelcomeWindow::PlayerLeftEvent()
{
	struct CallbackChange change = m_callbackHandler->PopCallbackChange();
	if(change.type == CALLBACK_ERROR)
	{
		cerr << "ERROR: Got an error in callback processing" << endl;
		return;
	}
	TreeModel::Children rows = playerListStore->children();
	TreeModel::iterator rowIter;
	for(rowIter=rows.begin(); rowIter!=rows.end(); rowIter++)
	{
		if(!rowIter)
		{
			cerr << "ERROR: A player row was corrupt\n";
			continue;
		}
		TreeModel::Row row=*rowIter;
		uint ID = row[playerColumns->ID];
		if( ID == change.playerID)
		{
			playerListStore->erase(rowIter);
		}
	}
	for(rowIter=rows.begin(); rowIter!=rows.end(); rowIter++)
	{
		TreeModel::Row row=*rowIter;
		uint ID = row[playerColumns->ID];
		if( ID == change.newLeaderID )
		{
			row[playerColumns->isLeader] = true;
		}
		else
		{
			row[playerColumns->isLeader] = false;
		}
		if(playerDescription.ID == change.newLeaderID)
		{
			row[playerColumns->leaderSelectable] = true;
			swap_leader_widgets(true);
		}
		else
		{
			row[playerColumns->leaderSelectable] = false;
			swap_leader_widgets(false);
		}
	}
	currentMatch.leaderID = change.newLeaderID;
	player_list_view->show_all();
}

void WelcomeWindow::KickedFromMatchEvent()
{
	struct CallbackChange change = m_callbackHandler->PopCallbackChange();
	if(change.type == CALLBACK_ERROR)
	{
		cerr << "ERROR: Got an error in callback processing" << endl;
		return;
	}
	//TODO: Stuff!
}

void WelcomeWindow::PlayerJoinedEvent()
{
	struct CallbackChange change = m_callbackHandler->PopCallbackChange();
	if(change.type == CALLBACK_ERROR)
	{
		cerr << "ERROR: Got an error in callback processing" << endl;
		return;
	}
	if(playerDescription.ID !=	change.playerDescription.ID)
	{
		PlayerListColumns playerColumns;

		//Add a new row for the new player
		TreeModel::Row row = *(playerListStore->append());
		row[playerColumns.name] = change.playerDescription.name;
		row[playerColumns.teamChosen] =	PopulateTeamNumberCombo();
		row[playerColumns.teamName] = Team::TeamNumberToString(
				(enum TeamNumber)change.playerDescription.team);
		row[playerColumns.ID] = change.playerDescription.ID;
		row[playerColumns.isLeader] = false;
		if(playerDescription.ID == currentMatch.leaderID)
		{
			row[playerColumns.leaderSelectable] = true;
		}
		else
		{
			row[playerColumns.leaderSelectable] = false;
		}
		player_list_view->show_all();
	}
}

void WelcomeWindow::LeaderChangedEvent()
{
	struct CallbackChange change = m_callbackHandler->PopCallbackChange();
	if(change.type == CALLBACK_ERROR)
	{
		cerr << "ERROR: Got an error in callback processing" << endl;
		return;
	}
	PlayerListColumns playerColumns;

	currentMatch.leaderID = change.playerID;

	TreeModel::Children rows = playerListStore->children();
	TreeModel::iterator rowIter;
	for(rowIter = rows.begin(); rowIter != rows.end(); rowIter++)
	{
		if(!rowIter)
		{
			cerr << "ERROR: A player row was corrupt\n";
			continue;
		}
		TreeModel::Row row = *rowIter;
		uint ID = row[playerColumns.ID];
		if( ID == change.playerID)
		{
			row[playerColumns.isLeader] = true;
		}
		else
		{
			row[playerColumns.isLeader] = false;
		}
		if(playerDescription.ID == change.playerID)
		{
			row[playerColumns.leaderSelectable] = true;
			//Swap out the game speed combo box and label
			swap_leader_widgets(true);
		}
		else
		{
			row[playerColumns.leaderSelectable] = false;
			//Swap out the game speed combo box and label
			swap_leader_widgets(false);
		}
	}
	player_list_view->show_all();
}

void WelcomeWindow::MatchStartedEvent()
{
	struct CallbackChange change = m_callbackHandler->PopCallbackChange();
	if(change.type == CALLBACK_ERROR)
	{
		cerr << "ERROR: Got an error in callback processing" << endl;
		return;
	}
	//TODO: Stuff!
}

void WelcomeWindow::CallbackClosedEvent()
{
	struct CallbackChange change = m_callbackHandler->PopCallbackChange();
	if(change.type == CALLBACK_ERROR)
	{
		cerr << "ERROR: Got an error in callback processing" << endl;
		return;
	}
}

void WelcomeWindow::CallbackErrorEvent()
{
	struct CallbackChange change = m_callbackHandler->PopCallbackChange();
	if(change.type == CALLBACK_ERROR)
	{
		cerr << "ERROR: Got an error in callback processing" << endl;
		return;
	}
	cerr << "ERROR: Callback receive failed\n";
}



void WelcomeWindow::custom_server_click()
{
	statusbar->push("Set server settings, then hit Connect");
	box_custom->set_visible(true);
}

void WelcomeWindow::connect_click()
{
	statusbar->push("Trying to connect...");

	//A little bit of input validation here
	string serverIP = entry_IP->get_text();

	struct sockaddr_in stSockAddr;
	int Res = inet_pton(AF_INET, serverIP.c_str(), &stSockAddr.sin_addr);
	if (Res == 0)
	{
		statusbar->push("Invalid IP address");
		return;
	}

	char *errString;
	uint serverPort = strtoul(entry_port->get_text().c_str(), &errString, 10);
	if( *errString != '\0' || entry_port->get_text().c_str() == '\0')
	{
		//Error occurred
		statusbar->push("Invalid port number");
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
		if(m_callbackHandler != NULL)
		{
			m_callbackHandler->Start();
		}
	}
	else
	{
		statusbar->push("Failed to connect to server");
	}
}

void WelcomeWindow::create_match_submit_click()
{
	string matchName = match_name_entry->get_text();
	if(matchName.size() < 1 && matchName.size() > 20)
	{
		status_lobby->push("Invalid match name length");
		return;
	}

	if(create_match_map_combo->get_active_row_number() == -1)
	{
		status_lobby->push("Please select a map");
		return;
	}
	string mapName = create_match_map_combo->get_active_text();

	int maxPlayers = max_players_combo->get_active_row_number();
	if( maxPlayers == -1)
	{
		status_lobby->push("Please select a maximum number of players");
		return;
	}

	//TODO: Use this once private matches are implemented
	//bool privateMatch = set_private_check->get_active();

	struct MatchOptions options;
	options.maxPlayers = maxPlayers + 2; //+2 since the combo starts at 2
	strncpy(options.name, match_name_entry->get_text().c_str(), sizeof(options.name));

	if (CreateMatch(options, &currentMatch) )
	{
		LaunchMatchLobbyPane(&playerDescription, 1);
	}
	else
	{
		status_lobby->push("Server returned failure to create match");
		return;
	}
}

void WelcomeWindow::create_match_click()
{
	create_match_box->set_visible(true);
	match_lists->set_visible(false);
}

//Refresh the match list
void WelcomeWindow::list_matches_click()
{
	list_matches();
}

void WelcomeWindow::leave_match_click()
{
	if( LeaveMatch() )
	{
		currentMatch.ID = 0;
		LaunchMainLobbyPane();
	}
	else
	{
		match_lobby_status->push("Error on server, couldn't leave match");
	}
}

void WelcomeWindow::join_match_click()
{
	int page = match_lists->get_current_page();
	if( page == -1 )
	{
		status_lobby->push("Please select a match, and try again");
		return;
	}

	TreeView *view = (TreeView*)match_lists->get_nth_page( page );
	if( view == NULL )
	{
		status_lobby->push("Please select a match, and try again");
		return;
	}

	MatchListColumns columns;
	Glib::RefPtr<Gtk::TreeSelection> select = view->get_selection();
	if( select->count_selected_rows() != 1)
	{
		status_lobby->push("Please select a match, and try again");
		return;
	}
	TreeModel::iterator iter = select->get_selected();
	TreeModel::Row row = *( iter );
	int matchID = row[columns.matchID];

	PlayerDescription playerDescriptions[MAX_PLAYERS_IN_MATCH];

	uint playerCount = JoinMatch(matchID, playerDescriptions, &currentMatch);
	if( playerCount > 0 )
	{
		currentMatch.ID = matchID;
		LaunchMatchLobbyPane(playerDescriptions, playerCount);
	}
	else
	{
		currentMatch.ID = 0;
		status_lobby->push("Failed to join match. Is it full?");
		return;
	}
}

void WelcomeWindow::quit_server_click()
{
	ExitServer();
	LaunchServerConnectPane();

	//This will wait for the callback thread to finish
	m_callbackHandler->Stop();
}

void WelcomeWindow::on_teamNumber_combo_changed(const Glib::ustring& path,
		const Gtk::TreeIter& iter)
{
	PlayerListColumns playerColumns;
	TeamComboColumns teamColumns;

	//Get the row of the currently selected player
	Glib::RefPtr<TreeSelection> selection = player_list_view->get_selection();
	TreeModel::iterator selectedIter = selection->get_selected();
	TreeModel::Row playerRow = *(selectedIter);

	//Get the row of the selected team (in the combobox, each item is a row)
	TreeModel::Row teamRow = (*iter);
	int newTeam = teamRow[teamColumns.teamNum];

	int changedPlayerID = playerRow[playerColumns.ID];
	if(ChangeTeam(changedPlayerID, (enum TeamNumber)newTeam) == false)
	{
		cerr << "WARNING: Change of team on the server failed\n";
		match_lobby_status->push("Could not change player's team");
		player_list_view->show_all();
		return;
	}

	playerRow[playerColumns.teamName] = Team::TeamNumberToString((enum TeamNumber)newTeam);

	//Get set the new team number back into the combobox's data
	TreeValueProxy<Glib::RefPtr<TreeModel> > teamNumberListStore =
			playerRow[playerColumns.teamChosen];
	Glib::RefPtr<TreeModel> treeModelPtr = teamNumberListStore;
	TreeModel::iterator chosenTeamIter = treeModelPtr->get_iter(path);
	TreeModel::Row existingTeamRow = (*chosenTeamIter);
	existingTeamRow[teamColumns.teamNum] = newTeam;

	player_list_view->show_all();
}

void WelcomeWindow::on_leader_toggled(const Glib::ustring& path)
{
	PlayerListColumns playerColumns;

	Glib::RefPtr<TreeModel> playerModelPtr = playerListStore;
	TreeModel::iterator chosenPlayerIter = playerModelPtr->get_iter(path);
	if(!chosenPlayerIter)
	{
		cerr << "ERROR: Invalid player row selected for changing leader\n";
		match_lobby_status->push("Could not change the leader");
		player_list_view->show_all();
		return;
	}
	TreeModel::Row chosenPlayerRow = (*chosenPlayerIter);
	uint newLeaderID = chosenPlayerRow[playerColumns.ID];

	if(ChangeLeader(newLeaderID) == false)
	{
		cerr << "WARNING: Change of leader on the server failed\n";
		match_lobby_status->push("Could not change the leader");
		player_list_view->show_all();
		return;
	}

	TreeModel::Children rows = playerListStore->children();
	TreeModel::iterator r;
	for(r=rows.begin(); r!=rows.end(); r++)
	{
		TreeModel::Row row=*r;
		row[playerColumns.isLeader] = false;
		row[playerColumns.leaderSelectable] = false;
	}
	chosenPlayerRow[playerColumns.isLeader] = true;
	currentMatch.leaderID = newLeaderID;

	player_list_view->show_all();
}

void WelcomeWindow::speed_combo_changed()
{
	char rowID = speed_combo->get_active_row_number();
	if( ChangeSpeed((enum GameSpeed)rowID) == false)
	{
		cerr << "ERROR: Server rejected change of game speed\n";
		match_lobby_status->push("Could not change game speed");
		return;
	}
}

void WelcomeWindow::victory_combo_changed()
{
	char rowID = win_condition_combo->get_active_row_number();
	if( ChangeVictoryCondition((enum VictoryCondition)rowID) == false)
	{
		cerr << "ERROR: Server rejected change of victory condition\n";
		match_lobby_status->push("Could not change victory condition");
		return;
	}
}

void WelcomeWindow::map_combo_changed()
{
	char rowID = win_condition_combo->get_active_row_number();
	struct MapDescription map;
	map.length = 12;
	map.width = 8;
	strcpy(map.name, "Sweet Map");
	if( ChangeMap(map) == false)
	{
		cerr << "ERROR: Server rejected change of victory condition\n";
		match_lobby_status->push("Could not change victory condition");
		return;
	}
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

void WelcomeWindow::launch_match_click()
{
	if(StartMatch())
	{
		system("RTT_Ogre_3D");
		match_lobby_status->push("Was that fun, or WHAT?!");
	}
	else
	{
		match_lobby_status->push("Failed to start match");
	}
}

//Swaps out the widgets which are only used when we are the match leader
// isLeader: True is we are the leader, false if not
void WelcomeWindow::swap_leader_widgets(bool isLeader)
{
	speed_combo->set_visible(isLeader);
	speed_label->set_visible(!isLeader);

	win_condition_combo->set_visible(isLeader);
	victory_cond_label->set_visible(!isLeader);

	map_name_combo->set_visible(isLeader);
	map_set_label->set_visible(!isLeader);
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

Glib::RefPtr<Gtk::ListStore> WelcomeWindow::PopulateTeamNumberCombo()
{
	TeamComboColumns teamColumns;
	Glib::RefPtr<Gtk::ListStore> listStore = Gtk::ListStore::create(teamColumns);

	TreeModel::Row row = *(listStore->append());
	row[teamColumns.teamNum] = 1;
	row[teamColumns.teamString] = "Team 1";
	row = *(listStore->append());
	row[teamColumns.teamNum] = 2;
	row[teamColumns.teamString] = "Team 2";
	row = *(listStore->append());
	row[teamColumns.teamNum] = 3;
	row[teamColumns.teamString] = "Team 3";
	row = *(listStore->append());
	row[teamColumns.teamNum] = 4;
	row[teamColumns.teamString] = "Team 4";
	row = *(listStore->append());
	row[teamColumns.teamNum] = 5;
	row[teamColumns.teamString] = "Team 5";
	row = *(listStore->append());
	row[teamColumns.teamNum] = 6;
	row[teamColumns.teamString] = "Team 6";
	row = *(listStore->append());
	row[teamColumns.teamNum] = 7;
	row[teamColumns.teamString] = "Team 7";
	row = *(listStore->append());
	row[teamColumns.teamNum] = 8;
	row[teamColumns.teamString] = "Team 8";
	row = *(listStore->append());
	row[teamColumns.teamNum] = 9;
	row[teamColumns.teamString] = "Referee";
	row = *(listStore->append());
	row[teamColumns.teamNum] = 0;
	row[teamColumns.teamString] = "Spectator";

	return listStore;
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

	teamNumberListStore = PopulateTeamNumberCombo();

	//Add a new row for each player
	for(uint i = 0; i < playerCount; i++)
	{
		TreeModel::Row row = *(playerListStore->append());

		row[playerColumns->name] = string(playerDescriptions[i].name);
		row[playerColumns->ID] = playerDescriptions[i].ID;
		row[playerColumns->teamChosen] = teamNumberListStore;
		row[playerColumns->teamName] = Team::TeamNumberToString(playerDescriptions[i].team);
		if( playerDescriptions[i].ID == currentMatch.leaderID)
		{
			row[playerColumns->isLeader] = true;
		}
		else
		{
			row[playerColumns->isLeader] = false;
		}
		if(playerDescription.ID == currentMatch.leaderID)
		{
			row[playerColumns->leaderSelectable] = true;
			swap_leader_widgets(true);
		}
		else
		{
			row[playerColumns->leaderSelectable] = false;
			swap_leader_widgets(false);
		}
	}

	CellRendererToggle *toggleRender = Gtk::manage( new Gtk::CellRendererToggle() );
	int cols_count = player_list_view->append_column("Leader", *toggleRender);
	Gtk::TreeViewColumn* toggleRenderColumn = player_list_view->get_column(cols_count-1);
	if(toggleRenderColumn)
	{
		toggleRenderColumn->add_attribute(
				toggleRender->property_active(), playerColumns->isLeader);
		toggleRenderColumn->add_attribute(
				toggleRender->property_activatable(), playerColumns->leaderSelectable);
	}
	toggleRender->signal_toggled().connect(sigc::mem_fun(*this,
		        &WelcomeWindow::on_leader_toggled));

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

	player_list_view->set_rules_hint(true);
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

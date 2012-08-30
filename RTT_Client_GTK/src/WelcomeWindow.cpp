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
using namespace Gtk;
using namespace std;

using boost::posix_time::ptime;
using boost::posix_time::time_duration;
using boost::gregorian::date;

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
	CallbackChange *change = m_callbackHandler->PopCallbackChange();
	if(change->m_type == CALLBACK_ERROR)
	{
		cerr << "ERROR: Got an error in callback processing" << endl;
		return;
	}
	MainLobbyCallbackChange *lobbyChange = (MainLobbyCallbackChange*)change;

	PlayerListColumns playerColumns;
	TeamComboColumns teamColumns;

	TreeModel::Children rows = m_playerListStore->children();
	TreeModel::iterator rowIter;
	for(rowIter=rows.begin(); rowIter!=rows.end(); rowIter++)
	{
		if(!rowIter)
		{
			cerr << "ERROR: A player row was corrupt\n";
			continue;
		}
		TreeModel::Row playerRow=*rowIter;
		int ID = playerRow[playerColumns.m_ID];
		if( ID == (int)lobbyChange->m_playerID )
		{
			playerRow[playerColumns.m_teamName] =
					Team::TeamNumberToString((enum TeamNumber)lobbyChange->m_team);

//						//Get set the new team number back into the combobox's data
//						TreeValueProxy<Glib::RefPtr<TreeModel> > teamNumberListStore =
//								playerRow[playerColumns.teamChosen];
//						Glib::RefPtr<TreeModel> treeModelPtr = teamNumberListStore;
//						TreeModel::iterator chosenTeamIter = treeModelPtr->get_iter("0");
//						TreeModel::Row existingTeamRow = (*chosenTeamIter);
//						existingTeamRow[teamColumns.teamNum] = change.team;
//						existingTeamRow[teamColumns.teamString] =
//								Team::TeamNumberToString((enum TeamNumber)change.team);

			m_player_list_view->show_all();
			break;
		}
	}
}

void WelcomeWindow::TeamColorChangedEvent()
{
	CallbackChange *change = m_callbackHandler->PopCallbackChange();
	if(change->m_type == CALLBACK_ERROR)
	{
		cerr << "ERROR: Got an error in callback processing" << endl;
		return;
	}
	//TODO: Something here!
}

void WelcomeWindow::MapChangedEvent()
{
	CallbackChange *change = m_callbackHandler->PopCallbackChange();
	if(change->m_type == CALLBACK_ERROR)
	{
		cerr << "ERROR: Got an error in callback processing" << endl;
		return;
	}
	MainLobbyCallbackChange *lobbyChange = (MainLobbyCallbackChange*)change;
	stringstream ss;
	ss << lobbyChange->m_mapDescription.m_width;
	ss << " x ";
	ss << lobbyChange->m_mapDescription.m_length;
	m_map_size_label->set_text(ss.str());
}

void WelcomeWindow::GamespeedChangedEvent()
{
	CallbackChange *change = m_callbackHandler->PopCallbackChange();
	if(change->m_type == CALLBACK_ERROR)
	{
		cerr << "ERROR: Got an error in callback processing" << endl;
		return;
	}
	MainLobbyCallbackChange *lobbyChange = (MainLobbyCallbackChange*)change;
	m_speed_label->set_text(Match::GameSpeedToString(lobbyChange->m_speed));
}

void WelcomeWindow::VictoryConditionChangedEvent()
{
	CallbackChange *change = m_callbackHandler->PopCallbackChange();
	if(change->m_type == CALLBACK_ERROR)
	{
		cerr << "ERROR: Got an error in callback processing" << endl;
		return;
	}
	MainLobbyCallbackChange *lobbyChange = (MainLobbyCallbackChange*)change;
	m_victory_cond_label->set_text(
			Match::VictoryConditionToString(lobbyChange->m_victory));
}

void WelcomeWindow::PlayerLeftEvent()
{
	CallbackChange *change = m_callbackHandler->PopCallbackChange();
	if(change->m_type == CALLBACK_ERROR)
	{
		cerr << "ERROR: Got an error in callback processing" << endl;
		return;
	}
	MainLobbyCallbackChange *lobbyChange = (MainLobbyCallbackChange*)change;
	TreeModel::Children rows = m_playerListStore->children();
	TreeModel::iterator rowIter;
	for(rowIter=rows.begin(); rowIter!=rows.end(); rowIter++)
	{
		if(!rowIter)
		{
			cerr << "ERROR: A player row was corrupt\n";
			continue;
		}
		TreeModel::Row row=*rowIter;
		uint ID = row[m_playerColumns->m_ID];
		if(ID == lobbyChange->m_playerID)
		{
			m_playerListStore->erase(rowIter);
		}
	}
	for(rowIter=rows.begin(); rowIter!=rows.end(); rowIter++)
	{
		TreeModel::Row row=*rowIter;
		uint ID = row[m_playerColumns->m_ID];
		if(ID == lobbyChange->m_newLeaderID )
		{
			row[m_playerColumns->m_isLeader] = true;
		}
		else
		{
			row[m_playerColumns->m_isLeader] = false;
		}
		if(m_playerDescription.m_ID == lobbyChange->m_newLeaderID)
		{
			row[m_playerColumns->m_leaderSelectable] = true;
			swap_leader_widgets(true);
		}
		else
		{
			row[m_playerColumns->m_leaderSelectable] = false;
			swap_leader_widgets(false);
		}
	}
	m_currentMatch.m_leaderID = lobbyChange->m_newLeaderID;
	m_player_list_view->show_all();
}

void WelcomeWindow::KickedFromMatchEvent()
{
	CallbackChange *change = m_callbackHandler->PopCallbackChange();
	if(change->m_type == CALLBACK_ERROR)
	{
		cerr << "ERROR: Got an error in callback processing" << endl;
		return;
	}
	//TODO: Stuff!
}

void WelcomeWindow::PlayerJoinedEvent()
{
	CallbackChange *change = m_callbackHandler->PopCallbackChange();
	if(change->m_type == CALLBACK_ERROR)
	{
		cerr << "ERROR: Got an error in callback processing" << endl;
		return;
	}
	MainLobbyCallbackChange *lobbyChange = (MainLobbyCallbackChange*)change;
	if(m_playerDescription.m_ID !=	lobbyChange->m_playerDescription.m_ID)
	{
		PlayerListColumns playerColumns;

		//Add a new row for the new player
		TreeModel::Row row = *(m_playerListStore->append());
		row[playerColumns.m_name] = lobbyChange->m_playerDescription.m_name;
		row[playerColumns.m_teamChosen] =	PopulateTeamNumberCombo();
		row[playerColumns.m_teamName] = Team::TeamNumberToString(
				(enum TeamNumber)lobbyChange->m_playerDescription.m_team);
		row[playerColumns.m_ID] = lobbyChange->m_playerDescription.m_ID;
		row[playerColumns.m_isLeader] = false;
		if(m_playerDescription.m_ID == m_currentMatch.m_leaderID)
		{
			row[playerColumns.m_leaderSelectable] = true;
		}
		else
		{
			row[playerColumns.m_leaderSelectable] = false;
		}
		m_player_list_view->show_all();
	}
}

void WelcomeWindow::LeaderChangedEvent()
{
	CallbackChange *change = m_callbackHandler->PopCallbackChange();
	if(change->m_type == CALLBACK_ERROR)
	{
		cerr << "ERROR: Got an error in callback processing" << endl;
		return;
	}
	MainLobbyCallbackChange *lobbyChange = (MainLobbyCallbackChange*)change;
	PlayerListColumns playerColumns;

	m_currentMatch.m_leaderID = lobbyChange->m_playerID;

	TreeModel::Children rows = m_playerListStore->children();
	TreeModel::iterator rowIter;
	for(rowIter = rows.begin(); rowIter != rows.end(); rowIter++)
	{
		if(!rowIter)
		{
			cerr << "ERROR: A player row was corrupt\n";
			continue;
		}
		TreeModel::Row row = *rowIter;
		uint ID = row[playerColumns.m_ID];
		if( ID == lobbyChange->m_playerID)
		{
			row[playerColumns.m_isLeader] = true;
		}
		else
		{
			row[playerColumns.m_isLeader] = false;
		}
		if(m_playerDescription.m_ID == lobbyChange->m_playerID)
		{
			row[playerColumns.m_leaderSelectable] = true;
			//Swap out the game speed combo box and label
			swap_leader_widgets(true);
		}
		else
		{
			row[playerColumns.m_leaderSelectable] = false;
			//Swap out the game speed combo box and label
			swap_leader_widgets(false);
		}
	}
	m_player_list_view->show_all();
}

void WelcomeWindow::MatchStartedEvent()
{
	CallbackChange *change = m_callbackHandler->PopCallbackChange();
	if(change->m_type == CALLBACK_ERROR)
	{
		cerr << "ERROR: Got an error in callback processing" << endl;
		return;
	}
	//TODO: Stuff!
}

void WelcomeWindow::CallbackClosedEvent()
{
	CallbackChange *change = m_callbackHandler->PopCallbackChange();
	if(change->m_type == CALLBACK_ERROR)
	{
		cerr << "ERROR: Got an error in callback processing" << endl;
		return;
	}
}

void WelcomeWindow::CallbackErrorEvent()
{
	CallbackChange *change = m_callbackHandler->PopCallbackChange();
	if(change->m_type == CALLBACK_ERROR)
	{
		cerr << "ERROR: Got an error in callback processing" << endl;
		return;
	}
	cerr << "ERROR: Callback receive failed\n";
}

void WelcomeWindow::custom_server_click()
{
	m_statusbar->push("Set server settings, then hit Connect");
	m_box_custom->set_visible(true);
}

void WelcomeWindow::connect_click()
{
	m_statusbar->push("Trying to connect...");

	//A little bit of input validation here
	string serverIP = m_entry_IP->get_text();

	struct sockaddr_in stSockAddr;
	int Res = inet_pton(AF_INET, serverIP.c_str(), &stSockAddr.sin_addr);
	if (Res == 0)
	{
		m_statusbar->push("Invalid IP address");
		return;
	}

	char *errString;
	uint serverPort = strtoul(m_entry_port->get_text().c_str(), &errString, 10);
	if( *errString != '\0' || m_entry_port->get_text().c_str() == '\0')
	{
		//Error occurred
		m_statusbar->push("Invalid port number");
		return;
	}

	string givenName = m_entry_username->get_text();
	string hashedPassword = m_entry_password->get_text();

	int SocketFD = AuthToServer(serverIP, serverPort,
			givenName, (unsigned char*)hashedPassword.c_str(), &m_playerDescription);

	if( SocketFD > 0 )
	{
		m_statusbar->push("Connection Successful!");
		LaunchMainLobbyPane();

		//Launch the Callback Thread
		if(m_callbackHandler != NULL)
		{
			m_callbackHandler->Start();
		}
	}
	else
	{
		m_statusbar->push("Failed to connect to server");
	}
}

void WelcomeWindow::create_match_submit_click()
{
	string matchName = m_match_name_entry->get_text();
	if(matchName.size() < 1 && matchName.size() > 20)
	{
		m_status_lobby->push("Invalid match name length");
		return;
	}

	if(m_create_match_map_combo->get_active_row_number() == -1)
	{
		m_status_lobby->push("Please select a map");
		return;
	}
	string mapName = m_create_match_map_combo->get_active_text();

	int maxPlayers = m_max_players_combo->get_active_row_number();
	if( maxPlayers == -1)
	{
		m_status_lobby->push("Please select a maximum number of players");
		return;
	}

	//TODO: Use this once private matches are implemented
	//bool privateMatch = set_private_check->get_active();

	struct MatchOptions options;
	options.m_maxPlayers = maxPlayers + 2; //+2 since the combo starts at 2
	strncpy(options.m_name, m_match_name_entry->get_text().c_str(), sizeof(options.m_name));

	if (CreateMatch(options, &m_currentMatch) )
	{
		vector<PlayerDescription> playerVector;
		playerVector.push_back(m_playerDescription);
		LaunchMatchLobbyPane(playerVector);
	}
	else
	{
		m_status_lobby->push("Server returned failure to create match");
		return;
	}
}

void WelcomeWindow::create_match_click()
{
	m_create_match_box->set_visible(true);
	m_match_lists->set_visible(false);
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
		m_currentMatch.m_ID = 0;
		LaunchMainLobbyPane();
	}
	else
	{
		m_match_lobby_status->push("Error on server, couldn't leave match");
	}
}

void WelcomeWindow::join_match_click()
{
	int page = m_match_lists->get_current_page();
	if( page == -1 )
	{
		m_status_lobby->push("Please select a match, and try again");
		return;
	}

	TreeView *view = (TreeView*)m_match_lists->get_nth_page( page );
	if( view == NULL )
	{
		m_status_lobby->push("Please select a match, and try again");
		return;
	}

	MatchListColumns columns;
	Glib::RefPtr<Gtk::TreeSelection> select = view->get_selection();
	if( select->count_selected_rows() != 1)
	{
		m_status_lobby->push("Please select a match, and try again");
		return;
	}
	TreeModel::iterator iter = select->get_selected();
	TreeModel::Row row = *( iter );
	int matchID = row[columns.m_matchID];


	vector<PlayerDescription> playerDescriptions;

	playerDescriptions = JoinMatch(matchID, m_currentMatch);

	if( playerDescriptions.size() > 0 )
	{
		m_currentMatch.m_ID = matchID;
		LaunchMatchLobbyPane(playerDescriptions);
	}
	else
	{
		m_currentMatch.m_ID = 0;
		m_status_lobby->push("Failed to join match. Is it full?");
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
	Glib::RefPtr<TreeSelection> selection = m_player_list_view->get_selection();
	TreeModel::iterator selectedIter = selection->get_selected();
	TreeModel::Row playerRow = *(selectedIter);

	//Get the row of the selected team (in the combobox, each item is a row)
	TreeModel::Row teamRow = (*iter);
	int newTeam = teamRow[teamColumns.m_teamNum];

	int changedPlayerID = playerRow[playerColumns.m_ID];
	if(ChangeTeam(changedPlayerID, (enum TeamNumber)newTeam) == false)
	{
		cerr << "WARNING: Change of team on the server failed\n";
		m_match_lobby_status->push("Could not change player's team");
		m_player_list_view->show_all();
		return;
	}

	playerRow[playerColumns.m_teamName] = Team::TeamNumberToString((enum TeamNumber)newTeam);

	//Get set the new team number back into the combobox's data
	TreeValueProxy<Glib::RefPtr<TreeModel> > m_teamNumberListStore =
			playerRow[playerColumns.m_teamChosen];
	Glib::RefPtr<TreeModel> treeModelPtr = m_teamNumberListStore;
	TreeModel::iterator chosenTeamIter = treeModelPtr->get_iter(path);
	TreeModel::Row existingTeamRow = (*chosenTeamIter);
	existingTeamRow[teamColumns.m_teamNum] = newTeam;

	m_player_list_view->show_all();
}

void WelcomeWindow::on_leader_toggled(const Glib::ustring& path)
{
	PlayerListColumns playerColumns;

	Glib::RefPtr<TreeModel> playerModelPtr = m_playerListStore;
	TreeModel::iterator chosenPlayerIter = playerModelPtr->get_iter(path);
	if(!chosenPlayerIter)
	{
		cerr << "ERROR: Invalid player row selected for changing leader\n";
		m_match_lobby_status->push("Could not change the leader");
		m_player_list_view->show_all();
		return;
	}
	TreeModel::Row chosenPlayerRow = (*chosenPlayerIter);
	uint newLeaderID = chosenPlayerRow[playerColumns.m_ID];

	if(ChangeLeader(newLeaderID) == false)
	{
		cerr << "WARNING: Change of leader on the server failed\n";
		m_match_lobby_status->push("Could not change the leader");
		m_player_list_view->show_all();
		return;
	}

	TreeModel::Children rows = m_playerListStore->children();
	TreeModel::iterator r;
	for(r=rows.begin(); r!=rows.end(); r++)
	{
		TreeModel::Row row=*r;
		row[playerColumns.m_isLeader] = false;
		row[playerColumns.m_leaderSelectable] = false;
	}
	chosenPlayerRow[playerColumns.m_isLeader] = true;
	m_currentMatch.m_leaderID = newLeaderID;

	m_player_list_view->show_all();
}

void WelcomeWindow::speed_combo_changed()
{
	char rowID = m_speed_combo->get_active_row_number();
	if( ChangeSpeed((enum GameSpeed)rowID) == false)
	{
		cerr << "ERROR: Server rejected change of game speed\n";
		m_match_lobby_status->push("Could not change game speed");
		return;
	}
}

void WelcomeWindow::victory_combo_changed()
{
	char rowID = m_win_condition_combo->get_active_row_number();
	if( ChangeVictoryCondition((enum VictoryCondition)rowID) == false)
	{
		cerr << "ERROR: Server rejected change of victory condition\n";
		m_match_lobby_status->push("Could not change victory condition");
		return;
	}
}

void WelcomeWindow::map_combo_changed()
{
	//char rowID = m_win_condition_combo->get_active_row_number();
	struct MapDescription map;
	map.m_length = 12;
	map.m_width = 8;
	strcpy(map.m_name, "Sweet Map");
	if( ChangeMap(map) == false)
	{
		cerr << "ERROR: Server rejected change of victory condition\n";
		m_match_lobby_status->push("Could not change victory condition");
		return;
	}
}

void WelcomeWindow::list_matches()
{
	ptime epoch(date(1970,boost::gregorian::Jan,1));

	m_create_match_box->set_visible(false);
	m_match_lists->set_visible(true);

	int page = m_match_lists->get_current_page();

	//If no page is selected, then put us to page 1
	if( page == -1)
	{
		page = 1;
	}

	struct ServerStats stats = GetServerStats();

	//Determine how many pages we need to display these
	//	x / y (rounding up) = (x + y - 1) / y
	uint pagesNeeded = (stats.m_numMatches + MATCHES_PER_PAGE - 1) / MATCHES_PER_PAGE;
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
	for(int i = 0; i < m_match_lists->get_n_pages(); i++)
	{
		m_match_lists->remove_page();
	}
	//Put the new ones in
	TreeView *view;
	for(uint i = 0; i < pagesNeeded; i++)
	{
		view = manage(new TreeView());
		m_match_lists->append_page(*view);
	}

	//Populate the view we're currently selecting
	struct MatchDescription descriptions[MATCHES_PER_PAGE];
	uint numMatchesThisPage = ListMatches(page+1, descriptions);

	m_match_lists->set_current_page(page);

	MatchListColumns *columns = new MatchListColumns();

	Glib::RefPtr<ListStore> refListStore = ListStore::create(*columns);
	view->set_model(refListStore);

	for(uint i = 0; i < numMatchesThisPage; i++)
	{
		TreeModel::Row row = *(refListStore->append());
		row[columns->m_matchID] = (int)descriptions[i].m_ID;
		row[columns->m_maxPlayers] = (int)descriptions[i].m_maxPlayers;
		row[columns->m_currentPlayers] = (int)descriptions[i].m_currentPlayerCount;
		row[columns->m_name] = descriptions[i].m_name;

		ptime time = epoch + boost::posix_time::seconds(descriptions[i].m_timeCreated);
		string timeString = to_simple_string(time);

		row[columns->m_timeCreated] = timeString;
	}

	view->append_column("ID", columns->m_matchID);
	view->append_column("Name", columns->m_name);
	view->append_column("Current Players", columns->m_currentPlayers);
	view->append_column("Max Players", columns->m_maxPlayers);
	view->append_column("Time Created", columns->m_timeCreated);

	m_match_lists->show_all();
}

void WelcomeWindow::launch_match_click()
{
	if(StartMatch())
	{
		m_match_lobby_status->push("Was that fun, or WHAT?!");
	}
	else
	{
		m_match_lobby_status->push("Failed to start match");
	}
}

//Swaps out the widgets which are only used when we are the match leader
// isLeader: True is we are the leader, false if not
void WelcomeWindow::swap_leader_widgets(bool isLeader)
{
	m_speed_combo->set_visible(isLeader);
	m_speed_label->set_visible(!isLeader);

	m_win_condition_combo->set_visible(isLeader);
	m_victory_cond_label->set_visible(!isLeader);

	m_map_name_combo->set_visible(isLeader);
	m_map_set_label->set_visible(!isLeader);
}

//Hides other windows (WelcomeWindow) and shows LobbyWindow
void WelcomeWindow::LaunchMainLobbyPane()
{
	m_welcome_box->set_visible(false);
	m_lobby_box->set_visible(true);
	m_match_lobby_box->set_visible(false);

	//By default, show the match list first
	list_matches();
}

void WelcomeWindow::LaunchServerConnectPane()
{
	m_welcome_box->set_visible(true);
	m_lobby_box->set_visible(false);
	m_match_lobby_box->set_visible(false);
}

Glib::RefPtr<Gtk::ListStore> WelcomeWindow::PopulateTeamNumberCombo()
{
	TeamComboColumns teamColumns;
	Glib::RefPtr<Gtk::ListStore> listStore = Gtk::ListStore::create(teamColumns);

	TreeModel::Row row = *(listStore->append());
	row[teamColumns.m_teamNum] = 1;
	row[teamColumns.m_teamString] = "Team 1";
	row = *(listStore->append());
	row[teamColumns.m_teamNum] = 2;
	row[teamColumns.m_teamString] = "Team 2";
	row = *(listStore->append());
	row[teamColumns.m_teamNum] = 3;
	row[teamColumns.m_teamString] = "Team 3";
	row = *(listStore->append());
	row[teamColumns.m_teamNum] = 4;
	row[teamColumns.m_teamString] = "Team 4";
	row = *(listStore->append());
	row[teamColumns.m_teamNum] = 5;
	row[teamColumns.m_teamString] = "Team 5";
	row = *(listStore->append());
	row[teamColumns.m_teamNum] = 6;
	row[teamColumns.m_teamString] = "Team 6";
	row = *(listStore->append());
	row[teamColumns.m_teamNum] = 7;
	row[teamColumns.m_teamString] = "Team 7";
	row = *(listStore->append());
	row[teamColumns.m_teamNum] = 8;
	row[teamColumns.m_teamString] = "Team 8";
	row = *(listStore->append());
	row[teamColumns.m_teamNum] = 9;
	row[teamColumns.m_teamString] = "Referee";
	row = *(listStore->append());
	row[teamColumns.m_teamNum] = 0;
	row[teamColumns.m_teamString] = "Spectator";

	return listStore;
}

void WelcomeWindow::LaunchMatchLobbyPane(
		const std::vector<PlayerDescription> &playerDescriptions)
{
	m_welcome_box->set_visible(false);
	m_lobby_box->set_visible(false);
	m_match_lobby_box->set_visible(true);

	//Clear out anything in the Player ListView
	m_player_list_view->remove_all_columns();
	m_player_list_view->unset_model();

	m_playerColumns = new PlayerListColumns();
	m_playerListStore = ListStore::create(*m_playerColumns);
	m_player_list_view->set_model(m_playerListStore);

	m_teamNumberListStore = PopulateTeamNumberCombo();

	//Add a new row for each player
	for(uint i = 0; i < playerDescriptions.size(); i++)
	{
		TreeModel::Row row = *(m_playerListStore->append());

		row[m_playerColumns->m_name] = string(playerDescriptions[i].m_name);
		row[m_playerColumns->m_ID] = playerDescriptions[i].m_ID;
		row[m_playerColumns->m_teamChosen] = m_teamNumberListStore;
		row[m_playerColumns->m_teamName] = Team::TeamNumberToString(playerDescriptions[i].m_team);
		if( playerDescriptions[i].m_ID == m_currentMatch.m_leaderID)
		{
			row[m_playerColumns->m_isLeader] = true;
		}
		else
		{
			row[m_playerColumns->m_isLeader] = false;
		}
		if(m_playerDescription.m_ID == m_currentMatch.m_leaderID)
		{
			row[m_playerColumns->m_leaderSelectable] = true;
			swap_leader_widgets(true);
		}
		else
		{
			row[m_playerColumns->m_leaderSelectable] = false;
			swap_leader_widgets(false);
		}
	}

	CellRendererToggle *toggleRender = Gtk::manage( new Gtk::CellRendererToggle() );
	int cols_count = m_player_list_view->append_column("Leader", *toggleRender);
	Gtk::TreeViewColumn* toggleRenderColumn = m_player_list_view->get_column(cols_count-1);
	if(toggleRenderColumn)
	{
		toggleRenderColumn->add_attribute(
				toggleRender->property_active(), m_playerColumns->m_isLeader);
		toggleRenderColumn->add_attribute(
				toggleRender->property_activatable(), m_playerColumns->m_leaderSelectable);
	}
	toggleRender->signal_toggled().connect(sigc::mem_fun(*this,
		        &WelcomeWindow::on_leader_toggled));

	m_player_list_view->append_column("Name", m_playerColumns->m_name);

	TreeView::Column* pColumn = manage( new Gtk::TreeView::Column("Team") );
	CellRendererCombo* pRenderer = manage(	new CellRendererCombo());
	pColumn->pack_start(*pRenderer);
	m_player_list_view->append_column(*pColumn);

	pRenderer->property_model() = m_teamNumberListStore;
	pColumn->add_attribute(pRenderer->property_text(), m_playerColumns->m_teamName);
	pColumn->add_attribute(pRenderer->property_model(), m_playerColumns->m_teamChosen);

	pRenderer->property_text_column() = 0;
	pRenderer->property_editable() = true;
	pRenderer->property_has_entry() = false;

	pRenderer->signal_changed().connect(
			sigc::mem_fun(*this, &WelcomeWindow::on_teamNumber_combo_changed));

	m_player_list_view->set_rules_hint(true);
	m_player_list_view->show_all();
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

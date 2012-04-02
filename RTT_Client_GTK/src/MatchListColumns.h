//============================================================================
// Name        : RTT_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Column definition for lists of matches
//============================================================================

#ifndef MATCHLISTCOLUMNS_H_
#define MATCHLISTCOLUMNS_H_

#include <gtkmm.h>

namespace RTT
{

class MatchListColumns : public Gtk::TreeModelColumnRecord
{
public:

	MatchListColumns()
	{ add(m_matchID); add(m_maxPlayers); add(m_currentPlayers); add(m_name); add(m_timeCreated);}

	Gtk::TreeModelColumn<int> m_matchID;
	Gtk::TreeModelColumn<int> m_maxPlayers;
	Gtk::TreeModelColumn<int> m_currentPlayers;
	Gtk::TreeModelColumn<std::string> m_name;
	Gtk::TreeModelColumn<std::string> m_timeCreated;
};

}

#endif /* MATCHLISTCOLUMNS_H_ */

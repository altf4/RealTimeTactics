//============================================================================
// Name        : RTT_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Column definition for lists of players in the match lobby
//============================================================================

#ifndef PLAYERLISTCOLUMNS_H_
#define PLAYERLISTCOLUMNS_H_


#include <gtkmm.h>

namespace RTT
{

class PlayerListColumns : public Gtk::TreeModelColumnRecord
{
public:

	PlayerListColumns()
	{ add(m_isLeader); add(m_leaderSelectable); add(m_name);
		add(m_teamName); add(m_teamChosen); add(m_ID);}

	Gtk::TreeModelColumn<bool> m_isLeader;
	Gtk::TreeModelColumn<bool> m_leaderSelectable;
	Gtk::TreeModelColumn<std::string> m_name;
	Gtk::TreeModelColumn<Glib::ustring> m_teamName;
	Gtk::TreeModelColumn<Glib::RefPtr<Gtk::TreeModel> > m_teamChosen;
	Gtk::TreeModelColumn<uint> m_ID;

};

}


#endif /* PLAYERLISTCOLUMNS_H_ */

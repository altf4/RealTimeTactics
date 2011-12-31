//============================================================================
// Name        : RTT_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Column definition for lists of players in the match lobby
//============================================================================

#ifndef PLAYERLISTCOLUMNS_H_
#define PLAYERLISTCOLUMNS_H_


#include <gtkmm.h>

using namespace Gtk;

namespace RTT
{

class PlayerListColumns : public Gtk::TreeModelColumnRecord
{
public:

	PlayerListColumns()
	{ add(name); add(teamName); add(teamChosen); add(ID);}

	TreeModelColumn<string> name;
	TreeModelColumn<Glib::ustring> teamName;
	TreeModelColumn<Glib::RefPtr<Gtk::TreeModel> > teamChosen;
	TreeModelColumn<uint> ID;

};

}


#endif /* PLAYERLISTCOLUMNS_H_ */

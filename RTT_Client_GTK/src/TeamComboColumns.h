//============================================================================
// Name        : TeamComboCoulumns.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Column definition for contents of TeamNumber combo box
//============================================================================

#ifndef TEAMCOMBOCOLUMNS_H_
#define TEAMCOMBOCOLUMNS_H_

#include <gtkmm.h>

//Tree model columns for the Combo CellRenderer in the TreeView column:
class TeamComboColumns : public Gtk::TreeModelColumnRecord
{
public:

	TeamComboColumns()
	{ add(m_teamString); add(m_teamNum); }

	//The values from which the user may choose.
	Gtk::TreeModelColumn<int> m_teamNum;
	//Extra information to help the user to choose.
	Gtk::TreeModelColumn<Glib::ustring> m_teamString;
};

#endif /* TEAMCOMBOCOLUMNS_H_ */

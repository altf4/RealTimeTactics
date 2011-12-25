//============================================================================
// Name        : RTT_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Column definition for lists of matches
//============================================================================

#ifndef MATCHLISTCOLUMNS_H_
#define MATCHLISTCOLUMNS_H_

#include <gtkmm.h>

using namespace Gtk;

namespace RTT
{

class MatchListColumns : public Gtk::TreeModelColumnRecord
{
public:

	MatchListColumns()
    { add(matchID); add(maxPlayers); }

  TreeModelColumn<int> matchID;
  TreeModelColumn<int> maxPlayers;
};

}

#endif /* MATCHLISTCOLUMNS_H_ */

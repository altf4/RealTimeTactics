//============================================================================
// Name        : Unit.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : A movement path along the hex map coordinate system
//============================================================================

#ifndef PATH_H_
#define PATH_H_

#include <boost/config.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

#include <vector>
#include "Tile.h"
#include "Gameboard.h"

using namespace boost;


typedef boost::adjacency_list < boost::listS, boost::vecS, boost::directedS,
		boost::no_property, boost::property < edge_weight_t, double > > graph_t;
typedef graph_traits < graph_t >::vertex_descriptor vertex_descriptor;
typedef graph_traits < graph_t >::edge_descriptor edge_descriptor;
typedef std::pair<int, int> Edge;

namespace RTT
{

class Path
{
public:
	std::vector <Tile*> m_tilesMovedThrough;

	//The total number of tiles moved through
	uint m_tileDistance;

	//Total movement cost of the path
	double m_movementCost;

	Path(Gameboard *board, Tile *fromTile, Tile *toTile, Unit *unit);

};


}


#endif /* PATH_H_ */

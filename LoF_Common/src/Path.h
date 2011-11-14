//============================================================================
// Name        : Unit.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : A movement path along the hex map coordinate system
//============================================================================

#ifndef PATH_H_
#define PATH_H_

#include <vector>
#include "Tile.h"
#include "Gameboard.h"

#include <boost/config.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

using namespace boost;


typedef adjacency_list < listS, vecS, directedS,
		no_property, property < edge_weight_t, double > > graph_t;
typedef graph_traits < graph_t >::vertex_descriptor vertex_descriptor;
typedef graph_traits < graph_t >::edge_descriptor edge_descriptor;
typedef pair<int, int> Edge;

using namespace std;

namespace LoF
{

class Path
{
	vector <Tile*> tilesMovedThrough;

	//The total number of tiles moved through
	uint tileDistance;

	//Total movement cost of the path
	double movementCost;

	Path(Gameboard *board, Tile *fromTile, Tile *toTile, Unit *unit);

	//Returns true on success, false on error
	//Populates the given edge_array and weights arrays
	bool BuildGraph(Edge *edge_array, double *weights);


};


}


#endif /* PATH_H_ */

//============================================================================
// Name        : Unit.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : A movement path along the hex map coordinate system
//============================================================================

#include "Path.h"

#include <iostream>
#include <fstream>

using namespace std;
using namespace LoF;


//Finds the shortest path from "fromTile" to "toTile" using Dijkstra's algorithm
//	Uses the "jump" value to determine how high the unit can travel
//		up or down elevations
Path::Path(Gameboard *board, Tile *fromTile, Tile *toTile, Unit *unit)
{
	const int num_nodes = board->tiles.size();
	const int num_edges = board->NumEdges(unit);

	//Declare the edge and weight arrays
	Edge *edge_array = (Edge*) malloc( num_edges * sizeof(Edge) );
	double *weights = (double*)malloc( num_edges * sizeof(double) );

	//Populate the edge and weight arrays


	char name[] = "ABCDE";

	graph_t g(edge_array, edge_array + num_edges, weights, num_nodes);
	property_map<graph_t, edge_weight_t>::type weightmap = get(edge_weight, g);

	vector<vertex_descriptor> p(num_vertices(g));
	vector<int> d(num_vertices(g));

	vertex_descriptor s = vertex(unit->ID, g);


	dijkstra_shortest_paths(g, s, predecessor_map(&p[0]).distance_map(&d[0]));


	cout << "distances and parents:" <<  endl;
	graph_traits < graph_t >::vertex_iterator vi, vend;
	for (tie(vi, vend) = vertices(g); vi != vend; ++vi)
	{
		cout << "distance(" << name[*vi] << ") = " << d[*vi] << ", ";
		cout << "parent(" << name[*vi] << ") = " << name[p[*vi]] <<
		endl;
	}
	cout <<  endl;

	ofstream dot_file("figs/dijkstra-eg.dot");

	dot_file << "digraph D {\n"
	<< "  rankdir=LR\n"
	<< "  size=\"4,3\"\n"
	<< "  ratio=\"fill\"\n"
	<< "  edge[style=\"bold\"]\n" << "  node[shape=\"circle\"]\n";

	graph_traits < graph_t >::edge_iterator ei, ei_end;
	for (tie(ei, ei_end) = edges(g); ei != ei_end; ++ei)
	{
		graph_traits < graph_t >::edge_descriptor e = *ei;
		graph_traits < graph_t >::vertex_descriptor
		u = source(e, g), v = target(e, g);
		dot_file << name[u] << " -> " << name[v]
		<< "[label=\"" << get(weightmap, e) << "\"";
		if (p[v] == u)
		dot_file << ", color=\"black\"";
		else
		dot_file << ", color=\"grey\"";
		dot_file << "]";
	}
	dot_file << "}";

}

//Returns true on success, false on error
//Populates the given edge_array and weights arrays
bool Path::BuildGraph(Edge *edge_array, double *weights)
{

}

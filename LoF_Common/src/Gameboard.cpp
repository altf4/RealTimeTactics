//============================================================================
// Name        : LoF_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Representation of the hex gameboard
//============================================================================

#include "Gameboard.h"
#include <iostream>
#include "InvalidGameboardException.h"

using namespace std;
using namespace LoF;

Gameboard::Gameboard(uint x_arg, uint y_arg)
{
	//The gameboard must at least be 2x2
	if( x_arg < 2 || y_arg < 2)
	{
		cerr << "ERROR: Invalid gameboard size specified. Too small.\n";
		throw InvalidGameboardException(
				"ERROR: Invalid gameboard size specified. Too small.");
		return;
	}

	xMax = x_arg;
	yMax = y_arg;

	//Create the gameboard tiles
	for(uint i = 0; i < xMax; i++)
	{
		for(uint j = 0; j < yMax; j++)
		{
			tiles[i][j] = new Tile(i,j);
		}
	}


}




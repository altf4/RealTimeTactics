//============================================================================
// Name        : RTT_Server.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Error handling for Movement
//============================================================================

#ifndef ILLEGALMOVEEXCEPTION_H_
#define ILLEGALMOVEEXCEPTION_H_

#include <stdexcept>
#include <string>

class IllegalMoveException : public std::runtime_error
{

public:
	IllegalMoveException(const std::string& message)
        : std::runtime_error(message) { };
};



#endif /* ILLEGALMOVEEXCEPTION_H_ */

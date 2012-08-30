//============================================================================
// Name        : CallbackChange.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Parent class for CallbackChanges
//
//============================================================================

#ifndef CALLBACKCHANGE_H_
#define CALLBACKCHANGE_H_

namespace RTT
{

enum CallbackType
{
	CHANGE_MAIN_LOBBY,
	CHANGE_GAME,
	CALLBACK_CLOSED,
	CALLBACK_ERROR,
	NO_CALLBACK,
};

class CallbackChange
{

public:
	enum CallbackType m_type;

	CallbackChange(enum CallbackType type){m_type = type;};

};

}

#endif /* CALLBACKCHANGE_H_ */

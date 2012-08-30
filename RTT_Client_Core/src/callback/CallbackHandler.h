//============================================================================
// Name        : CallbackHandler.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Handles incoming callback messages from game server
//============================================================================

#ifndef CALLBACKHANDLER_H_
#define CALLBACKHANDLER_H_

#include <queue>
#include "pthread.h"

#include "../ClientProtocolHandler.h"
#include "CallbackChange.h"


namespace RTT
{

class CallbackHandler
{
public:

	CallbackHandler();
	~CallbackHandler();

	bool Start();
	void Stop();

	CallbackChange *PopCallbackChange();

private:

	void *CallbackThread();
	void PushCallbackChange(CallbackChange *change);

	static void *StartThreadHelper(void *ptr);

	pthread_t m_thread;

	pthread_mutex_t m_queueMutex;
	std::queue<CallbackChange*> m_changeQueue;
};

}
#endif /* CALLBACKHANDLER_H_ */

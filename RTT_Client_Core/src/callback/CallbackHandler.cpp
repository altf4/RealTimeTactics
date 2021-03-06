//============================================================================
// Name        : CallbackHandler.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : Handles incoming callback messages from game server
//============================================================================

#include "CallbackHandler.h"
#include "messaging/MessageManager.h"

#include <iostream>

using namespace std;
using namespace RTT;

CallbackHandler::CallbackHandler()
{
	pthread_mutex_init(&m_queueMutex, NULL);
}

CallbackHandler::~CallbackHandler()
{
	pthread_join(m_thread, NULL);
}


bool CallbackHandler::Start()
{
	pthread_create(&m_thread, NULL, &CallbackHandler::StartThreadHelper, this);
	return true;
}

void *CallbackHandler::StartThreadHelper(void *ptr)
{
	return reinterpret_cast<CallbackHandler*>(ptr)->CallbackThread();
}

void CallbackHandler::Stop()
{
	pthread_join(m_thread, NULL);
}

CallbackChange* CallbackHandler::PopCallbackChange()
{
	Lock lock (&m_queueMutex);
	if(m_changeQueue.empty())
	{
		return new CallbackChange(NO_CALLBACK);
	}
	else
	{
		CallbackChange *temp = m_changeQueue.front();
		m_changeQueue.pop();
		return temp;
	}
}

void CallbackHandler::PushCallbackChange(CallbackChange *change)
{
	Lock lock(&m_queueMutex);
	m_changeQueue.push(change);
}

void *CallbackHandler::CallbackThread()
{
	bool keepGoing = true;
	while(keepGoing)
	{
		CallbackChange *change = ProcessCallbackCommand();
		if(change->m_type == CALLBACK_CLOSED)
		{
			keepGoing = false;
		}
		PushCallbackChange(change);
	}

	return NULL;
}

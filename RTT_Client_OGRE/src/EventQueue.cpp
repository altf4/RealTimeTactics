//============================================================================
// Name        : EventQueue.cpp
// Author      : AltF4
// Copyright   : 2012, GNU GPLv3
// Description : A queue that the Ogre frame loop pulls from to process events
//	that the RTT server has sent us. Events are pushed to this queue in the
//	UI_*Signal functions of events classes
//============================================================================

#include "EventQueue.h"

#include <cstddef>

using namespace RTT;
using namespace std;

EventQueue *EventQueue::m_instance = NULL;

EventQueue &EventQueue::Instance()
{
	if(m_instance == NULL)
	{
		m_instance = new EventQueue();
	}
	return *EventQueue::m_instance;
}

EventQueue::EventQueue()
{
}

void EventQueue::Enqueue(struct ServerEvent event)
{
	m_queue.push(event);
}

struct ServerEvent EventQueue::Dequeue()
{
	if(!m_queue.empty())
	{
		struct ServerEvent front = m_queue.front();
		m_queue.pop();
		return front;
	}
	else
	{
		struct ServerEvent empty;
		return empty;
	}
}

//Returns true if the queue is empty
bool EventQueue::Empty()
{
	return m_queue.empty();
}

//Deletes all entries in the queue
void EventQueue::Clear()
{
	//There is no clear function for queues. You have to assign a blank queue to the object.
	m_queue = queue<struct ServerEvent>();
}

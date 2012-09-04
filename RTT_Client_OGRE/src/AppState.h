//============================================================================
// Name        : AppState.h
// Author      : Mark Petro
// Copyright   : 2011, GNU GPLv3
// Description : Built from the Advanced OGRE Framework tutorial found here:
//	http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Advanced+Ogre+Framework&structure=Tutorials
//============================================================================

#ifndef APP_STATE_H
#define APP_STATE_H

#include "AdvancedOgreFramework.h"

class AppState;

class AppStateListener
{
public:
	AppStateListener(){};
	virtual ~AppStateListener(){};

	virtual void ManageAppState(Ogre::String stateName, AppState* state) = 0;

	virtual AppState *FindByName(Ogre::String stateName) = 0;
	virtual void ChangeAppState(AppState *state) = 0;
	virtual bool PushAppState(AppState* state) = 0;
	virtual void PopAppState() = 0;
	virtual void PauseAppState() = 0;
	virtual void Shutdown() = 0;
	virtual void PopAllAndPushAppState(AppState *state) = 0;
};

class AppState : public OIS::KeyListener, public OIS::MouseListener
{
public:
	static void Create(AppStateListener *parent, const Ogre::String name){};

	void Destroy(){delete this;}

	virtual void Enter() = 0;
	virtual void Exit() = 0;
	virtual bool Pause(){return true;}
	virtual void Resume(){};
	virtual void Update(double timeSinceLastFrame) = 0;
	virtual void ProcessCallback(RTT::CallbackChange *) = 0;

protected:
	AppState(){};

	AppState *FindByName(Ogre::String stateName){return m_parent->FindByName(stateName);}
	void ChangeAppState(AppState *state){m_parent->ChangeAppState(state);}
	bool PushAppState(AppState *state){return m_parent->PushAppState(state);}
	void PopAppState(){m_parent->PopAppState();}
	void Shutdown(){m_parent->Shutdown();}
	void PopAllAndPushAppState(AppState *state){m_parent->PopAllAndPushAppState(state);}

	AppStateListener *m_parent;

	Ogre::Camera *m_camera;
	Ogre::SceneManager *m_sceneMgr;
	Ogre::FrameEvent m_frameEvent;
};

#define DECLARE_APPSTATE_CLASS(T)					\
static void create(AppStateListener *parent, const Ogre::String name)	\
{									\
	T* myAppState = new T();					\
	myAppState->m_parent = parent;					\
	parent->ManageAppState(name, myAppState);			\
}

#endif

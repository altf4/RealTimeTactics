//============================================================================
// Name        : AppStateManager.hpp
// Author      : Mark Petro
// Copyright   : 2011, GNU GPLv3
// Description : Built from the Advanced OGRE Framework tutorial found here:
//	http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Advanced+Ogre+Framework&structure=Tutorials
//============================================================================

#ifndef APP_STATE_MANAGER_HPP
#define APP_STATE_MANAGER_HPP

#include "AppState.hpp"

class AppStateManager : public AppStateListener
{
public:
	typedef struct
	{
		Ogre::String m_name;
		AppState *m_state;
	} state_info;

	AppStateManager();
	~AppStateManager();

	void manageAppState(Ogre::String stateName, AppState* state);

	AppState *findByName(Ogre::String stateName);

	void start(AppState *state);
	void changeAppState(AppState *state);
	bool pushAppState(AppState *state);
	void popAppState();
	void pauseAppState();
	void shutdown();
	void popAllAndPushAppState(AppState *state);

protected:
	void init(AppState *state);

	std::vector<AppState*> m_ActiveStateStack;
	std::vector<state_info> m_States;
	bool m_bShutdown;
};

#endif

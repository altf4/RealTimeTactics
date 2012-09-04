//============================================================================
// Name        : AppStateManager.hpp
// Author      : Mark Petro
// Copyright   : 2011, GNU GPLv3
// Description : Built from the Advanced OGRE Framework tutorial found here:
//	http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Advanced+Ogre+Framework&structure=Tutorials
//============================================================================

#ifndef APP_STATE_MANAGER_HPP
#define APP_STATE_MANAGER_HPP

#include "AppState.h"

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

	void ManageAppState(Ogre::String stateName, AppState* state);

	AppState *FindByName(Ogre::String stateName);

	void Start(AppState *state);
	void ChangeAppState(AppState *state);
	bool PushAppState(AppState *state);
	void PopAppState();
	void PauseAppState();
	void Shutdown();
	void PopAllAndPushAppState(AppState *state);

protected:
	void init(AppState *state);

	std::vector<AppState*> m_activeStateStack;
	std::vector<state_info> m_states;
	bool m_isShutdown;
};

#endif

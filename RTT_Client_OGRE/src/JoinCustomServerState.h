//============================================================================
// Name        : JoinSuctomServerState.h
// Author      : Mark Petro
// Copyright   : 2011, GNU GPLv3
// Description : Built from the Advanced OGRE Framework tutorial found here:
//	http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Advanced+Ogre+Framework&structure=Tutorials
//============================================================================

#ifndef JOINCUSTOMSERVER_STATE_H
#define JOINCUSTOMSERVER_STATE_H

#include "AppState.h"

class JoinCustomServerState : public AppState
{
public:
	JoinCustomServerState();

	DECLARE_APPSTATE_CLASS(JoinCustomServerState)

	void enter();
	void createScene();
	void exit();
	bool pause();
	void resume();

	bool keyPressed(const OIS::KeyEvent &keyEventRef);
	bool keyReleased(const OIS::KeyEvent &keyEventRef);

	bool mouseMoved(const OIS::MouseEvent &evt);
	bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id);
	bool mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id);

	bool onExitButton(const CEGUI::EventArgs &args);
	bool onBackButton(const CEGUI::EventArgs &args);
	bool onJoinServerButton(const CEGUI::EventArgs &args);

	bool onAddressActivate(const CEGUI::EventArgs &args);
	bool onAddressDeactivate(const CEGUI::EventArgs &args);

	bool onPortActivate(const CEGUI::EventArgs &args);
	bool onPortDeactivate(const CEGUI::EventArgs &args);

	bool onUsernameActivate(const CEGUI::EventArgs &args);
	bool onUsernameDeactivate(const CEGUI::EventArgs &args);

	bool onPasswordActivate(const CEGUI::EventArgs &args);
	bool onPasswordDeactivate(const CEGUI::EventArgs &args);

	//CEGUI::Window *m_pJCSWnd;

	void update(double timeSinceLastFrame);

	RTT::PlayerDescription m_playerDescription;

	RTT::CallbackHandler *m_callbackHandler;

private:
	bool m_bQuit;
};

#endif

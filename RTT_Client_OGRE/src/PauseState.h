 //============================================================================
 // Name        : PauseState.hpp
 // Author      : Mark Petro
 // Copyright   : 2011, GNU GPLv3
 // Description : Built from the Advanced OGRE Framework tutorial found here:
 //	http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Advanced+Ogre+Framework&structure=Tutorials
 //============================================================================

#ifndef PAUSE_STATE_HPP
#define PAUSE_STATE_HPP

#include "AppState.h"

class PauseState : public AppState
{
public:
	PauseState();

	DECLARE_APPSTATE_CLASS(PauseState)

	void enter();
	void createScene();
	void exit();

	bool keyPressed(const OIS::KeyEvent &keyEventRef);
	bool keyReleased(const OIS::KeyEvent &keyEventRef);

	bool mouseMoved(const OIS::MouseEvent &evt);
	bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id);
	bool mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id);

	//void buttonHit(OgreBites::Button* button);
	void yesNoDialogClosed(const Ogre::DisplayString& question, bool yesHit);

	void update(double timeSinceLastFrame);

private:
	bool m_bQuit;
	bool m_bQuestionActive;
};

#endif

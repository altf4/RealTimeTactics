//============================================================================
// Name        : GameState.hpp
// Author      : Mark Petro
// Copyright   : 2011, GNU GPLv3
// Description : Built from the Advanced OGRE Framework tutorial found here:
//	http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Advanced+Ogre+Framework&structure=Tutorials
//============================================================================

#ifndef GAME_STATE_HPP
#define GAME_STATE_HPP

#include "AppState.h"
#include "RTT_Ogre_Unit.h"
#include "RTT_Ogre_Player.h"

#include "GameCommands.h"
#include "GameEvents.h"

//#include "DotSceneLoader.hpp"

#include <OGRE/OgreSubEntity.h>
#include <OGRE/OgreMaterialManager.h>

enum QueryFlags
{
	OGRE_HEAD_MASK	= 1<<0,
	CUBE_MASK		= 1<<1
};

class GameState : public AppState
{
public:
	GameState(void);

	DECLARE_APPSTATE_CLASS(GameState)

	void Enter();
	void CreateScene();
	void Exit();
	bool Pause();
	void Resume();

	void MoveCamera();
	void GetInput();
	void BuildGUI();

	bool keyPressed(const OIS::KeyEvent &keyEventRef);
	bool keyReleased(const OIS::KeyEvent &keyEventRef);

	bool mouseMoved(const OIS::MouseEvent &arg);
	bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id);
	bool mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id);

	void OnLeftPressed(const OIS::MouseEvent &evt);
	//void itemSelected(OgreBites::SelectMenu* menu);
	bool OnExitButtonGame(const CEGUI::EventArgs &args);

	void SetBufferedMode();
	void SetUnbufferedMode();

	void Update(double timeSinceLastFrame);

	void MoveUnitOnScreen(RTT::RTT_Ogre_Unit &arg);
	void MoveCursor(const RTT::Direction &arg);
	void ShowRange(RTT::RTT_Ogre_Unit &arg, bool &arg2);
	void MakeMove(RTT::RTT_Ogre_Unit &arg);

	void BuildUnits();

	void ProcessCallback(struct RTT::ServerEvent event);

	private:
	Ogre::SceneNode *m_ogreHeadNode;
	Ogre::Entity *m_ogreHeadEntity;
	Ogre::MaterialPtr m_ogreHeadMat;
	Ogre::MaterialPtr m_ogreHeadMatHigh;

	//OgreBites::ParamsPanel *m_pDetailsPanel;
	bool m_quit;

	Ogre::Vector3 m_translateVector;
	Ogre::Real m_moveSpeed;
	Ogre::Degree m_rotateSpeed;
	float m_moveScale;
	Ogre::Degree m_rotScale;

	Ogre::RaySceneQuery *m_RSQ;
	Ogre::SceneNode *m_currentObject;
	Ogre::Entity *m_currentEntity;
	bool m_isLMouseDown, m_isRMouseDown;

	//TODO: these two may be the same
	bool m_isSettingsMode;
	bool m_isChatMode;

	bool m_isMoving;
	RTT::RTT_Ogre_Player m_mainPlayer;
	RTT::RTT_Ogre_Unit m_playerCursor;
	CEGUI::Window *m_mainWnd;
	CEGUI::Window *m_chatWnd;
};

#endif

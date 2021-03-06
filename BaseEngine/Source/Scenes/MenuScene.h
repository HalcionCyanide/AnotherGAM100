/******************************************************************************
filename    MenuScene.h
author      Keith Cheng
DP email    keith.cheng@digipen.edu
Course: 	GAM100F17
Copyright � 2017 DigiPen (USA) Corporation

Brief Description:
The menu screen that occurs after the splash screen.
It is used to either transit to the game or exit

******************************************************************************/

/* Header Guards */
#ifndef MENU_SCENE_H
#define MENU_SCENE_H

#include "../StateManager/BaseStateManager.h"

/* Public Struct & Functions */
typedef struct MenuScene
{
	/* Public Parameters */
	enum {
		MM_MENU = 0,
		MM_TRANSITION,
		MM_INSTRUCTION,
		MM_CREDITS
	}MM_States;
	enum MM_States InternalState;
	BaseStateManager InternalStateManager;

	enum{
		SEL_START,
		SEL_INSTR,
		SEL_CRED,
		SEL_EXIT,
		SEL_TOTAL
	}MENU_SELECTED;

	short wKeyPressed;
	short sKeyPressed;

	enum MENU_SELECTED selectedMenuState;

	/* Public Function Pointers*/
	/* Initiallize, requires an instance of itself */
	void(*Initiallize)();
	/* Updates, requires an instance of itself as well as delta time */
	void(*Update)();
	/* Renders, requires an instance of itself and a passed engine*, writes the data of the scene to the screen's buffer */
	void(*Render)();
	/* Exits, requires an instance, calls free if memory was allocated */
	void(*Exit)();

}MenuScene;

///****************************************************************************
// Public Function Prototypes
///****************************************************************************
/* Setup function that initiallizes the SplashScene */
void MenuScene_Setup(MenuScene* Self);

#endif //MENU_SCENE_H
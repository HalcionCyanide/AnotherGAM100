/******************************************************************************
filename    DungeonScene.c
author      Rui An Ryan Lim & Qingping Zheng
DP email    l.ruianryan@digipen.edu
	    qingping.zheng@digipen.edu
Course: 	GAM100F17
Copyright � 2017 DigiPen (USA) Corporation

Created on 16 November 2017

Brief Description:
The dungeon scene of the game. Contains game state logic involving the battle
mechanics.

******************************************************************************/
#include "DungeonScene.h"

#include <stdio.h>

// Inclusion of utility functions
#include "../Utilities/TextDataLoader.h"
#include "../Utilities/Utilities.h"

// Included for Rendering
#include "../Engine/BaseEngine.h"
#include "../LogicalObjects/DungeonCamera.h"
#include "../Player/Player.h"

///****************************************************************************
// Private Variables
///****************************************************************************
TextDataLoader DungeonScene_Loader;
DungeonCamera DungeonScene_Camera;
Vector2 dungeon_moveDirection = { 0,0 }; // Direction of player movement

// Timers for letting the player run
double dungeon_initialRunDelay; // How long it takes before the player starts running
double dungeon_runDelayX; // How fast the player runs on X axis, speed = 1/runDelayX
double dungeon_runDelayY; // How fast the player runs on Y axis
double dungeon_runTimerX;
double dungeon_runTimerY;

//dungeon transition vairiables
short dungeon_transitionCount;
double dungeon_transitionTimer;
double dungeon_transitionTimerDelay;
short dungeon_waitToggle;
double dungeon_transitionWaitDelay;
TextDataLoader DungeonTransition_Loader;

//boss transition variables
short dungeon_bossCount;
short dungeon_bossToggle;
double dungeon_bossTimer;
double dungeon_bossDelay;
TextDataLoader DungeonBossTransition_Loader;

///****************************************************************************
// Private Function Prototypes
///****************************************************************************
// State Manager Functions
// Linked Initiallize function that will be set to the struct's Initiallize
void DungeonScene_LinkedInitiallize(DungeonScene* Self);
// Linked Update function that will be set to the struct's Update
void DungeonScene_LinkedUpdate(DungeonScene* Self, Engine* BaseEngine, double Delta);
// Linked Render function that will be set to the struct's Render
void DungeonScene_LinkedRender(DungeonScene* Self, Engine* BaseEngine);
// Linked Exit function that will be set to the struct's Exit
void DungeonScene_LinkedExit(DungeonScene* Self);

// Internal State Manager Functions
// Linked initiallize function that will be set to the InternalStateManager.Initiallize
void DungeonScene_LinkedInternalInitiallize(DungeonScene* Self);
// Linked Update function that will be set to the InternalStateManager.Update
void DungeonScene_LinkedInternalUpdate(DungeonScene* Self, Engine* BaseEngine, double Delta);
// Linked Render function that will be set to the InternalStateManager.Render
void DungeonScene_LinkedInternalRender(DungeonScene* Self, Engine* BaseEngine);
// Linked Exit function that will be set to the InternalStateManager.Exit
void DungeonScene_LinkedInternalExit(DungeonScene* Self);

//Local functions that relate to the scene.
void DungeonScene_PlayerControls(DungeonScene* self, Engine* BaseEngine, double Delta);
Vector2 parseRandomSpawnPoint(TextDataLoader map, char charToLookOutFor);
void DungeonScene_Transition(DungeonScene* Self, Engine* BaseEngine, double Delta);
void Dungeon_BossUpdate(DungeonScene* Self, Engine* BaseEngine, double Delta);
///****************************************************************************
// Function Definitions
///****************************************************************************
void DungeonScene_Setup(DungeonScene* Self)
{
	// Set up the InternalStateManager
	Self->InternalStateManager.Initiallize = DungeonScene_LinkedInternalInitiallize;
	Self->InternalStateManager.Update = DungeonScene_LinkedInternalUpdate;
	Self->InternalStateManager.Render = DungeonScene_LinkedInternalRender;
	Self->InternalStateManager.Exit = DungeonScene_LinkedInternalExit;

	// Set the current state
	Self->InternalState = DS_Exploration;

	// Set up the functions of this object
	Self->Initiallize = DungeonScene_LinkedInitiallize;
	Self->Update = DungeonScene_LinkedUpdate;
	Self->Render = DungeonScene_LinkedRender;
	Self->Exit = DungeonScene_LinkedExit;
}

// Linked Initiallize function that will be set to the struct's Initiallize
void DungeonScene_LinkedInitiallize(DungeonScene* Self)
{
	Self->InternalStateManager.Initiallize(Self);
}

// Linked Update function that will be set to the struct's Update
void DungeonScene_LinkedUpdate(DungeonScene* Self, Engine* BaseEngine, double Delta)
{
	Self->InternalStateManager.Update(Self, BaseEngine, Delta);
}

// Linked Render function that will be set to the struct's Render
void DungeonScene_LinkedRender(DungeonScene* Self, Engine* BaseEngine)
{
	Self->InternalStateManager.Render(Self, BaseEngine);
}

// Linked Exit function that will be set to the struct's Exit
void DungeonScene_LinkedExit(DungeonScene* Self)
{
	Self->InternalStateManager.Exit(Self);
}

//local function, parses the map for an E, and 
Vector2 parseRandomSpawnPoint(TextDataLoader map, char charToLookOutFor)
{
	int spawnPointOccurenceCount = 0;
	//we take note of how many potential spawnpoints there are
	for (int y = 0; y < map.NumberOfRows; ++y)
	{
		for (int x = 0; x < map.NumberOfColumns; ++x)
		{
			if (map.TextData[y][x] == charToLookOutFor) //potential spawnpoint
			{
				spawnPointOccurenceCount++;
			}
		}
	}
	
	//we now know the number of spawnpoints in the provided map
	Vector2* possiblelocations = (Vector2*)malloc(spawnPointOccurenceCount * sizeof(Vector2));

	//re-iterate and add to list of spawnpoints
	int counter = 0;
	for (int y = 0; y < map.NumberOfRows; ++y)
	{
		for (int x = 0; x < map.NumberOfColumns; ++x)
		{
			if (map.TextData[y][x] == charToLookOutFor) //potential spawnpoint
			{
				possiblelocations[counter] = Vec2(x, y);
				counter++;
			}
		}
	}
	//gimme a random spawnpoint
	int randomIndex = rand() % spawnPointOccurenceCount;
	//we got the spawnpoint, free memory
	Vector2 copyVector = Vec2(possiblelocations[randomIndex].x, possiblelocations[randomIndex].y);
	free(possiblelocations);
	return copyVector;
}

// Linked Initiallize function that will be set to the InternalStateManager
void DungeonScene_LinkedInternalInitiallize(DungeonScene* Self)
{
	// Here I will initiallize the internal state manager
	// Setup the loader that I am about to use.
	// Load the sprites that will be used in the battle scene
	TextDataLoader_Setup(&DungeonTransition_Loader);
	DungeonTransition_Loader.LoadResource(&DungeonTransition_Loader, "Resources/BattleTransition.txt");

	TextDataLoader_Setup(&DungeonScene_Loader);
	// Set the current state
	Self->InternalState = DS_Exploration;
	Self->metBoss = 0;
	Self->firstFrameOfUpdate = 0;
	Self->wKeyPressed = Self->sKeyPressed = Self->aKeyPressed = Self->dKeyPressed = 0;
	dungeon_moveDirection = Vec2(0, 0);
	//LOCAL VARIABLES
	dungeon_initialRunDelay = 0.3;
	dungeon_runDelayX = 0.1;
	dungeon_runDelayY = 0.15;
	dungeon_runTimerX = 0;
	dungeon_runTimerY = 0;
	dungeon_transitionCount = 0;
	dungeon_transitionTimer = 0;
	dungeon_transitionTimerDelay = 0.01;
	dungeon_waitToggle = 0;
	dungeon_transitionWaitDelay = 0.5;

	dungeon_bossCount = 0;
	dungeon_bossToggle = 0;
	dungeon_bossTimer = 0;
	dungeon_bossDelay = 0.01;
}

// Linked Update function that will be set to the InternalStateManager
void DungeonScene_LinkedInternalUpdate(DungeonScene* Self, Engine* BaseEngine, double Delta)
{
	isKeyPressed(VK_SPACE);

	// Do some state logic for the internal state manager
	switch (Self->InternalState)
	{
	case DS_TransitionToWorld:
		BaseEngine->InternalSceneSystem.SetCurrentScene(&BaseEngine->InternalSceneSystem, SS_WorldView);
		break;
	case DS_Exploration:
		if (Self->firstFrameOfUpdate == 0)
		{
			//ADD MORE IF NEEDED.
			char *maps[] = {
				"Resources/Dungeons/dungeon0.txt", //completed
				"Resources/Dungeons/dungeon1.txt", //completed
				"Resources/Dungeons/dungeon2.txt", //completed
				"Resources/Dungeons/dungeon3.txt",  //completed
				"Resources/Dungeons/dungeon4.txt" //DO NOT EVER RANDOM THIS
			};

			if (BaseEngine->InternalSceneSystem.InternalWorldViewScene.currentRoomIndex != 4)
			{
				int randomMap = rand() % ((sizeof(maps) / sizeof(char*)) - 1);
				DungeonScene_Loader.LoadResource(&DungeonScene_Loader, maps[randomMap]);
			}
			else
			{
				DungeonScene_Loader.LoadResource(&DungeonScene_Loader, maps[4]);
			}
			Initialize_Player(&Self->player, parseRandomSpawnPoint(DungeonScene_Loader, 'E'));
			DungeonCamera_Setup(&DungeonScene_Camera);

			Self->firstFrameOfUpdate = 1;
		}

		DungeonScene_PlayerControls(Self, BaseEngine, Delta);
		DungeonScene_Camera.UpdateCameraLogic(&DungeonScene_Camera, BaseEngine->g_console, &DungeonScene_Loader, &Self->player.position);
		break;
	case DS_TransitionToBattle:
		DungeonScene_Transition(Self, BaseEngine, Delta);
		break;
	case DS_TransitionToBoss:
		if (Self->metBoss == 0)
		{
			TextDataLoader_Setup(&DungeonBossTransition_Loader);
			/*6 8
			  0 2*/
			switch (BaseEngine->InternalSceneSystem.InternalWorldViewScene.currentRoomIndex)
			{
			case 0:
				DungeonBossTransition_Loader.LoadResource(&DungeonBossTransition_Loader, "Resources/Information/Boss_cutscene1.txt");
				break;
			case 6:
				DungeonBossTransition_Loader.LoadResource(&DungeonBossTransition_Loader, "Resources/Information/Boss_cutscene2.txt");
				break;
			case 8:
				DungeonBossTransition_Loader.LoadResource(&DungeonBossTransition_Loader, "Resources/Information/Boss_cutscene3.txt");
				break;
			case 2:
				DungeonBossTransition_Loader.LoadResource(&DungeonBossTransition_Loader, "Resources/Information/Boss_cutscene4.txt");
				break;
			case 4:
				DungeonBossTransition_Loader.LoadResource(&DungeonBossTransition_Loader, "Resources/Information/Boss_cutscene5.txt");
				break;
			}
			Self->metBoss = 1;
		}
		Dungeon_BossUpdate(Self, BaseEngine, Delta);
		//transition
		break;
	default: 
		break;
	}
}

// Linked Render function that will be set to the InternalStateManager
void DungeonScene_LinkedInternalRender(DungeonScene* Self, Engine* BaseEngine)
{
	// Renders the appropriate scene
	switch (Self->InternalState)
	{
	case DS_TransitionToWorld:
		break;
	case DS_Exploration:
		if (Self->firstFrameOfUpdate == 1)
		{
			BaseEngine->g_console->dungeon_WriteToBuffer(BaseEngine->g_console, DungeonScene_Loader.TextData, DungeonScene_Camera.CalculatedMapOffset.x, DungeonScene_Camera.CalculatedMapOffset.y, getColor(c_black, c_white));
			BaseEngine->g_console->replace_withColor(BaseEngine->g_console, 'E', ' ', getColor(c_black, c_white));
			BaseEngine->g_console->replace_withColor(BaseEngine->g_console, '#', 219, getColor(c_black, c_dgrey));
			BaseEngine->g_console->replace_withColor(BaseEngine->g_console, 'X', 'X', getColor(c_black, c_red));
			BaseEngine->g_console->replace_withColor(BaseEngine->g_console, '/', '/', getColor(c_black, c_red));
			BaseEngine->g_console->replace_withColor(BaseEngine->g_console, '\\', '\\', getColor(c_black, c_red));
			BaseEngine->g_console->text_WriteToBuffer(BaseEngine->g_console, Vec2(Self->player.position.x - DungeonScene_Camera.CalculatedMapOffset.x, Self->player.position.y - DungeonScene_Camera.CalculatedMapOffset.y), "O", getColor(c_black, c_aqua));
		}
		break;
	case DS_TransitionToBattle:
	{
		Vector2 location = { -BaseEngine->g_console->consoleSize.X + dungeon_transitionCount, 0 };
		BaseEngine->g_console->sprite_WriteToBuffer(BaseEngine->g_console, location, DungeonTransition_Loader.TextData, DungeonTransition_Loader.NumberOfRows, DungeonTransition_Loader.NumberOfColumns, getColor(c_black, c_white));
	}
	case DS_TransitionToBoss:
	{
		Vector2 location = { -BaseEngine->g_console->consoleSize.X + dungeon_bossCount, 0 };
		BaseEngine->g_console->sprite_WriteToBuffer(BaseEngine->g_console, location, DungeonBossTransition_Loader.TextData, DungeonBossTransition_Loader.NumberOfRows, DungeonBossTransition_Loader.NumberOfColumns, getColor(c_black, c_white));
		BaseEngine->g_console->replace_withColor(BaseEngine->g_console, '$', 130, getColor(c_black, c_white));
	}
		break;
	default:
		break;
	}
}

// Linked Exit function that will be set to the InternalStateManager
void DungeonScene_LinkedInternalExit(DungeonScene* Self)
{
	// Free the stuff initiallized in the Internal State Manager
	DungeonTransition_Loader.Exit(&DungeonTransition_Loader);
	DungeonScene_Loader.Exit(&DungeonScene_Loader);
	if (Self->metBoss == 1)
	{
		DungeonBossTransition_Loader.Exit(&DungeonBossTransition_Loader);
	}
}

void DungeonScene_PlayerControls(DungeonScene* self, Engine* BaseEngine, double Delta)
{
	short MovementCheck = 0;
	if (isKeyPressed('W'))
	{
		// Key press down
		if (self->wKeyPressed == 0)
		{
			self->wKeyPressed = 1;
			dungeon_moveDirection.y--;
			short plrMoveCode = MovePlayer(&self->player, Vec2(0, dungeon_moveDirection.y), DungeonScene_Loader);
			if (plrMoveCode == 1)
			{
				self->InternalState = DS_TransitionToWorld;
			}
			else if (plrMoveCode == 2)
			{
				self->InternalState = DS_TransitionToBoss;
			}
			else if (plrMoveCode == 3)
				MovementCheck = 1;
		}
	}
	else
	{
		// Key press up
		if (self->wKeyPressed == 1)
		{
			self->wKeyPressed = 0;
			dungeon_moveDirection.y++;
		}
	}

	if (isKeyPressed('S'))
	{
		// Key press down
		if (self->sKeyPressed == 0)
		{
			self->sKeyPressed = 1;
			dungeon_moveDirection.y++;
			short plrMoveCode = MovePlayer(&self->player, Vec2(0, dungeon_moveDirection.y), DungeonScene_Loader);
			if (plrMoveCode == 1)
			{
				self->InternalState = DS_TransitionToWorld;
			}
			else if (plrMoveCode == 2)
			{
				self->InternalState = DS_TransitionToBoss;
			}
			else if (plrMoveCode == 3)
				MovementCheck = 1;
		}
	}
	else
	{
		// Key press up
		if (self->sKeyPressed == 1)
		{
			self->sKeyPressed = 0;
			dungeon_moveDirection.y--;
		}
	}

	if (isKeyPressed('A'))
	{
		// Key press down
		if (self->aKeyPressed == 0)
		{
			self->aKeyPressed = 1;
			dungeon_moveDirection.x--;
			short plrMoveCode = MovePlayer(&self->player, Vec2(dungeon_moveDirection.x, 0), DungeonScene_Loader);
			if (plrMoveCode == 1)
			{
				self->InternalState = DS_TransitionToWorld;
			}
			else if (plrMoveCode == 2)
			{
				self->InternalState = DS_TransitionToBoss;
			}
			else if (plrMoveCode == 3)
				MovementCheck = 1;
		}
	}
	else
	{
		// Key press up
		if (self->aKeyPressed == 1)
		{
			self->aKeyPressed = 0;
			dungeon_moveDirection.x++;
		}
	}

	if (isKeyPressed('D'))
	{
		// Key press down
		if (self->dKeyPressed == 0)
		{
			self->dKeyPressed = 1;
			dungeon_moveDirection.x++;
			short plrMoveCode = MovePlayer(&self->player, Vec2(dungeon_moveDirection.x, 0), DungeonScene_Loader);
			if (plrMoveCode == 1)
			{
				self->InternalState = DS_TransitionToWorld;
			}
			else if (plrMoveCode == 2)
			{
				self->InternalState = DS_TransitionToBoss;
			}
			else if (plrMoveCode == 3)
				MovementCheck = 1;
		}
	}
	else
	{
		// Key press up
		if (self->dKeyPressed == 1)
		{
			self->dKeyPressed = 0;
			dungeon_moveDirection.x--;
		}
	}

	if (self->aKeyPressed == 1 || self->dKeyPressed == 1 || self->wKeyPressed == 1 || self->sKeyPressed == 1)
	{
		if ((dungeon_runTimerX += Delta) > dungeon_initialRunDelay) // Initial delay before running
		{
			if (dungeon_runTimerX > dungeon_initialRunDelay + dungeon_runDelayX) // 0.4 - 0.3 = 0.1, delay in between each "run step"
			{
				dungeon_runTimerX = dungeon_initialRunDelay;

				short plrMoveCode = MovePlayer(&self->player, Vec2(dungeon_moveDirection.x, 0), DungeonScene_Loader);
				if (plrMoveCode == 1)
				{
					self->InternalState = DS_TransitionToWorld;
				}
				else if (plrMoveCode == 2)
				{
					self->InternalState = DS_TransitionToBoss;
				}
				else if (plrMoveCode == 3)
					MovementCheck = 1;
				plrMoveCode = MovePlayer(&self->player, Vec2(0, dungeon_moveDirection.y), DungeonScene_Loader);
				if (plrMoveCode == 1)
				{
					self->InternalState = DS_TransitionToWorld;
				}
				else if (plrMoveCode == 2)
				{
					self->InternalState = DS_TransitionToBoss;
				}
				else if (plrMoveCode == 3)
					MovementCheck = 1;
			}
		}
	}
	else
	{
		dungeon_runTimerX = 0;
		dungeon_moveDirection.x = 0;
	}
	if (MovementCheck == 1)
	{
		// Check if a monster has been encountered
		Vector2 EnemyRange;
		switch (BaseEngine->InternalSceneSystem.InternalWorldViewScene.currentRoomIndex)
		{
		case 0:
			EnemyRange = Vec2(Enemy_Rat, Enemy_Frog);
			break;
		case 6:
			EnemyRange = Vec2(Enemy_Bird, Enemy_Skeleton);
			break;
		case 8:
			EnemyRange = Vec2(Enemy_Goblin, Enemy_Unicorn);
			break;
		case 2:
			EnemyRange = Vec2(Enemy_Skeleton, Enemy_Dragon);
			break;
		}
		if (BaseEngine->InternalSceneSystem.InternalWorldViewScene.currentRoomIndex != 4)
		{
			if (!isKeyPressed(VK_SHIFT))
			{
				if (EnemyEncounterHandler_RandomizeEncounter(&BaseEngine->InternalSceneSystem.InternalEncounterHandler, 2, EnemyRange.x, EnemyRange.y) == 1)
				{
					// Do something
					self->InternalState = DS_TransitionToBattle;
					BaseEngine->InternalSceneSystem.InternalEncounterHandler.PreviousSceneWasDungeon = 1;
					BaseEngine->InternalSceneSystem.InternalBattleScene.Exit(&BaseEngine->InternalSceneSystem.InternalBattleScene);
					BaseEngine->InternalSceneSystem.InternalBattleScene.Initiallize(&BaseEngine->InternalSceneSystem.InternalBattleScene);
				}
			}
		}
	}
}

void DungeonScene_Transition(DungeonScene* self, Engine* BaseEngine, double Delta)
{
	dungeon_transitionTimer += Delta;

	if (dungeon_waitToggle == 0)
	{
		if (dungeon_transitionTimer > dungeon_transitionTimerDelay)
		{
			dungeon_transitionCount++;
			dungeon_transitionTimer = 0;
			if (dungeon_transitionCount == 80)
			{
				dungeon_waitToggle = 1;
			}
		}
	}
	else if (dungeon_waitToggle == 1)
	{
		if (dungeon_transitionTimer > dungeon_transitionWaitDelay)
		{
			dungeon_transitionCount = 0;
			dungeon_transitionTimer = 0;
			dungeon_waitToggle = 0;
			self->InternalState = DS_Exploration;
			BaseEngine->InternalSceneSystem.SetCurrentScene(&BaseEngine->InternalSceneSystem, SS_Battle);
		}
	}
}

void Dungeon_BossUpdate(DungeonScene* Self, Engine* BaseEngine, double Delta)
{

	short tempToggle = 0;
	if (isKeyPressed(VK_SPACE))
	{
		tempToggle = 1;
	}

	if (dungeon_bossToggle == 0)
	{
		if ((dungeon_bossTimer += Delta) > dungeon_bossDelay)
		{
			dungeon_bossTimer = 0;
			if (dungeon_bossCount < 80)
			{
				dungeon_bossCount++;
			}
			else
			{
				dungeon_bossToggle = tempToggle;
			}
		}
	}
	else if (dungeon_bossToggle == 1)
	{
		//transit scenes, no need to translate back
		if ((dungeon_bossTimer += Delta) > dungeon_bossDelay)
		{
			dungeon_bossTimer = 0;
			dungeon_bossToggle = 0;
			//rig the bossfight
			switch (BaseEngine->InternalSceneSystem.InternalWorldViewScene.currentRoomIndex)
			{
			case 0:
				EnemyEncounterHandler_SetUpEncounter(&BaseEngine->InternalSceneSystem.InternalEncounterHandler, Boss_DatBoiLv1);
				break;
			case 6:
				EnemyEncounterHandler_SetUpEncounter(&BaseEngine->InternalSceneSystem.InternalEncounterHandler, Boss_DatBoiLv2);
				break;																						   
			case 8:																							   
				EnemyEncounterHandler_SetUpEncounter(&BaseEngine->InternalSceneSystem.InternalEncounterHandler, Boss_DatBoiLv3);
				break;																						   
			case 2:																							   
				EnemyEncounterHandler_SetUpEncounter(&BaseEngine->InternalSceneSystem.InternalEncounterHandler, Boss_DatBoiLv4);
				break;
			case 4:
				EnemyEncounterHandler_SetUpEncounter(&BaseEngine->InternalSceneSystem.InternalEncounterHandler, Boss_DatBoiLv5);
				break;
			}
			//change current scene
			BaseEngine->InternalSceneSystem.InternalBattleScene.Exit(&BaseEngine->InternalSceneSystem.InternalBattleScene);
			BaseEngine->InternalSceneSystem.InternalBattleScene.Initiallize(&BaseEngine->InternalSceneSystem.InternalBattleScene);
			BaseEngine->InternalSceneSystem.SetCurrentScene(&BaseEngine->InternalSceneSystem, SS_Battle);
			Self->InternalState = DS_Exploration; //change state safely
			dungeon_bossCount = 0;
			dungeon_bossToggle = 0;
			dungeon_bossTimer = 0;
			dungeon_bossDelay = 0.01;
		}
	}
}
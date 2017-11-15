/******************************************************************************
filename    WorldViewScene.c
author      Keith Cheng
DP email    keith.cheng@digipen.edu

Created on 14 November 2017

Brief Description:


******************************************************************************/
#include "WorldViewScene.h"

#include <stdio.h>

// Inclusion of utility functions
#include "../Utilities/TextDataLoader.h"
#include "../Utilities/Utilities.h"
#include "../Rooms/RoomFactory.h"

// Included for Rendering
#include "../Engine/BaseEngine.h"

///****************************************************************************
// Private Variables
///****************************************************************************


///****************************************************************************
// Private Function Prototypes
///****************************************************************************

// State Manager Functions
// Linked Initiallize function that will be set to the struct's Initiallize
void WorldViewScene_LinkedInitiallize(WorldViewScene* self);
// Linked Update function that will be set to the struct's Update
void WorldViewScene_LinkedUpdate(WorldViewScene* self, float Delta);
// Linked Render function that will be set to the struct's Render
void WorldViewScene_LinkedRender(WorldViewScene* self, Engine* Renderer);
// Linked Exit function that will be set to the struct's Exit
void WorldViewScene_LinkedExit(WorldViewScene* self);

// Internal State Manager Functions
// Linked initiallize function that will be set to the InternalStateManager.Initiallize
void WorldViewScene_LinkedInternalInitiallize(WorldViewScene* self);
// Linked Update function that will be set to the InternalStateManager.Update
void WorldViewScene_LinkedInternalUpdate(WorldViewScene* self, float Delta);
// Linked Render function that will be set to the InternalStateManager.Render
void WorldViewScene_LinkedInternalRender(WorldViewScene* self, Engine* Renderer);
// Linked Exit function that will be set to the InternalStateManager.Exit
void WorldViewScene_LinkedInternalExit(WorldViewScene* self);

// Split functions to make file neater
void PlayerControls(WorldViewScene* self);
void Add_Room(WorldViewScene* self, char *mapString);

///****************************************************************************
// Function Definitions
///****************************************************************************
void WorldViewScene_Setup(WorldViewScene* self)
{
	// Set up the InternalStateManager
	self->InternalStateManager.Initiallize = WorldViewScene_LinkedInternalInitiallize;
	self->InternalStateManager.Update = WorldViewScene_LinkedInternalUpdate;
	self->InternalStateManager.Render = WorldViewScene_LinkedInternalRender;
	self->InternalStateManager.Exit = WorldViewScene_LinkedInternalExit;

	// Set the current state
	self->InternalState = WVS_ROAMING;

	// Set up the functions of this object
	self->Initiallize = WorldViewScene_LinkedInitiallize;
	self->Update = WorldViewScene_LinkedUpdate;
	self->Render = WorldViewScene_LinkedRender;
	self->Exit = WorldViewScene_LinkedExit;
}

// Linked Initiallize function that will be set to the struct's Initiallize
void WorldViewScene_LinkedInitiallize(WorldViewScene* self)
{
	self->InternalStateManager.Initiallize(self);
}

// Linked Update function that will be set to the struct's Update
void WorldViewScene_LinkedUpdate(WorldViewScene* self, float Delta)
{
	self->InternalStateManager.Update(self, Delta);
}

// Linked Render function that will be set to the struct's Render
void WorldViewScene_LinkedRender(WorldViewScene* self, Engine* Renderer)
{
	self->InternalStateManager.Render(self, Renderer);
}

// Linked Exit function that will be set to the struct's Exit
void WorldViewScene_LinkedExit(WorldViewScene* self)
{
	self->InternalStateManager.Exit(self);
}

// Linked Initiallize function that will be set to the InternalStateManager
void WorldViewScene_LinkedInternalInitiallize(WorldViewScene* self)
{
	// Here I will initiallize the internal state manager
	// Setup the loader that I am about to use.

	// Initializing variables
	self->currentRoom = NULL;
	self->wKeyPressed = 0;
	self->sKeyPressed = 0;
	self->aKeyPressed = 0;
	self->dKeyPressed = 0;

	// Initializing room list and player

	InitRoomArray(&(self->roomList), 5);
	Vector2 startingPosition;
	startingPosition.x = startingPosition.y = 5;
	Initialize_Player(&self->player, startingPosition, &self->currentRoom);

	Add_Room(self, "Resources/Maps/TestMap.txt"); // Room0
	Add_Room(self, "Resources/Maps/TestMap2.txt"); // Room1

	Room* room0 = self->roomList.array[0];
	Room* room1 = self->roomList.array[1];
	room0->AddExit(room0, room1, NORTH);
	room1->AddExit(room1, room0, SOUTH);

}

// Linked Update function that will be set to the InternalStateManager
void WorldViewScene_LinkedInternalUpdate(WorldViewScene* self, float Delta)
{
	// Do some state logic for the internal state manager
	// Testing code state cycling
	switch (self->InternalState)
	{
	case WVS_ROAMING:
		PlayerControls(self);
		if (self->player.dir != D_TOTAL)
		{
			if (self->currentRoom->exits[self->player.dir] != NULL)
			{
				self->currentRoom = self->currentRoom->exits[self->player.dir];

				switch (self->player.dir)
				{
				case NORTH:
					self->player.position.y = self->currentRoom->Loader.NumberOfRows - 1;
					break;
				case SOUTH:
					self->player.position.y = 0;
					break;
				case EAST:
					self->player.position.x = 0;
					break;
				case WEST:
					self->player.position.x = self->currentRoom->Loader.NumberOfColumns - 1;
					break;
				}

			}
			self->player.dir = D_TOTAL;
		}
		break;
	case WVS_TRANSITION:
		break;
	}
}

// Linked Render function that will be set to the InternalStateManager
void WorldViewScene_LinkedInternalRender(WorldViewScene* self, Engine* Renderer)
{
	// Renders the appropriate scene
	switch (self->InternalState)
	{
	case WVS_ROAMING:
		Renderer->g_console->Ptr_writeToBuffer(Renderer->g_console, self->currentRoom->mapToRender, self->currentRoom->Loader.NumberOfRows, self->currentRoom->Loader.NumberOfColumns, getColor(c_black, c_white));
		Renderer->g_console->WriteToBuffer(Renderer->g_console, self->player.position, "O", getColor(c_black, c_white));
		break;
	}
}

// Linked Exit function that will be set to the InternalStateManager
void WorldViewScene_LinkedInternalExit(WorldViewScene* self)
{
	// Free the stuff initiallized in the Internal State Manager
	FreeRoomArray(&(self->roomList));
}


void PlayerControls(WorldViewScene* self)
{
	if (isKeyPressed('W'))
	{
		// Key press down
		if (self->wKeyPressed == 0)
		{
			self->wKeyPressed = 1;
			Vector2 moveDirection;
			moveDirection.x = 0;
			moveDirection.y = -1;
			MovePlayer(&self->player, moveDirection);
		}
	}
	else
	{
		// Key press up
		if (self->wKeyPressed == 1)
		{
			self->wKeyPressed = 0;
		}
	}

	if (isKeyPressed('S'))
	{
		// Key press down
		if (self->sKeyPressed == 0)
		{
			self->sKeyPressed = 1;
			Vector2 moveDirection;
			moveDirection.x = 0;
			moveDirection.y = 1;
			MovePlayer(&self->player, moveDirection);
		}
	}
	else
	{
		// Key press up
		if (self->sKeyPressed == 1)
		{
			self->sKeyPressed = 0;
		}
	}

	if (isKeyPressed('A'))
	{
		// Key press down
		if (self->aKeyPressed == 0)
		{
			self->aKeyPressed = 1;
			Vector2 moveDirection;
			moveDirection.x = -1;
			moveDirection.y = 0;
			MovePlayer(&self->player, moveDirection);
		}
	}
	else
	{
		// Key press up
		if (self->aKeyPressed == 1)
		{
			self->aKeyPressed = 0;
		}
	}

	if (isKeyPressed('D'))
	{
		// Key press down
		if (self->dKeyPressed == 0)
		{
			self->dKeyPressed = 1;
			Vector2 moveDirection;
			moveDirection.x = 1;
			moveDirection.y = 0;
			MovePlayer(&self->player, moveDirection);
		}
	}
	else
	{
		// Key press up
		if (self->dKeyPressed == 1)
		{
			self->dKeyPressed = 0;
		}
	}
}

void Add_Room(WorldViewScene* self, char *mapString)
{
	Room* roomToAdd = RoomFactory_CreateRoom();
	roomToAdd->LoadMap(roomToAdd, mapString);
	AddToRoomArray(&(self->roomList), roomToAdd);
	if (self->currentRoom == NULL)
	{
		self->currentRoom = roomToAdd;
	}
}
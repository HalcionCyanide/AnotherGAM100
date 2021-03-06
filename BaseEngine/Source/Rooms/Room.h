/******************************************************************************
filename	Room.h
author      	Keith Cheng
DP email	keith.cheng@digipen.edu
Course: 	GAM100F17
Copyright � 2017 DigiPen (USA) Corporation

Brief Description:
A base class for rooms

******************************************************************************/

#ifndef ROOM_H
#define ROOM_H

typedef struct Room Room;
#include "../Utilities/TextDataLoader.h"

typedef enum DIRECTION
{
	NORTH,
	EAST,
	SOUTH,
	WEST,
	D_TOTAL
} DIRECTION;

struct Room
{
	char **mapToRender; // a 2D char array
	Room *exits[D_TOTAL]; // Pointers to other rooms (Edges of map)
	TextDataLoader Loader;

	void(*Init)();
	
	void(*Free)();
	void(*AddExit)(); // Params : self,exitroom,direction
	void(*LoadMap)();
};

void Room_Init(Room* self);

void Room_Free(Room* self);

void Room_AddExit(Room* self, Room* exitRoom, DIRECTION dir);

void Room_LoadMap(Room* self, char* mapString);

#endif
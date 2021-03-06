/******************************************************************************
filename	Player.c
author      	Keith Cheng
DP email	keith.cheng@digipen.edu
Course: 	GAM100F17
Copyright � 2017 DigiPen (USA) Corporation

Brief Description:
Player class to store player variables

******************************************************************************/

#include "Player.h"

void Initialize_Player(Player* player, Vector2 position)
{
	player->position = position;
	player->dir = D_TOTAL;
}

short MovePlayer(Player *player, Vector2 move, TextDataLoader Loader)
{
	Vector2 finalPosition = Vec2(player->position.x + move.x, player->position.y + move.y);

	if (finalPosition.x < Loader.NumberOfColumns - 1 && finalPosition.x >= 0 && finalPosition.y < Loader.NumberOfRows && finalPosition.y >= 0)
	{
		if (Loader.TextData[finalPosition.y][finalPosition.x] == '#')
		{				
			return 0; //wall
		}
		else if (Loader.TextData[finalPosition.y][finalPosition.x] == '@')
		{
			return 1; //portal
		}
		else if (Loader.TextData[finalPosition.y][finalPosition.x] == 'X')
		{
			return 2; //bossfight
		}
		else
		{
			player->position = finalPosition;
			return 3;
		}
	}
	else
	{
		if (move.x > 0 && player->position.x == Loader.NumberOfColumns - 2)
		{
			player->dir = EAST;
		}
		else if (move.x < 0 && player->position.x == 0)
		{
			player->dir = WEST;
		}
		else if (move.y < 0 && player->position.y == 0)
		{
			player->dir = NORTH;
		}
		else if (move.y > 0 && player->position.y == Loader.NumberOfRows - 1)
		{
			player->dir = SOUTH;
		}
	}
	return 0;
}
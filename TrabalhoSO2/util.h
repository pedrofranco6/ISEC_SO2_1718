#pragma once
#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <windows.h>

// Defines
#define MINIDPLAYER		1000
#define BUFFSIZE		1024
#define TCHARSIZE		30

#define TCHAR_SIZE 	60
#define WHO 		60
#define COMMANDSIZE 60
#define MAXCLIENTS  20
#define MAXSHIPS	20 
#define BUFFERSIZE	20
#define ALL_PLAYERS 100

#define EXIT			100
#define CREATE_GAME		101
#define JOIN_GAME		102
#define SCORES			103
#define START_GAME		104
#define MOVE			105

#define GAME_OVER		107
#define REFRESH_BOARD	108

//errors
#define ERROR_CANNOT_CREATE_GAME	99
#define ERROR_CANNOT_JOIN_GAME		98

//messages
#define SHM_ALL 600


#define LEFT  1
#define RIGHT 2
#define UP    3
#define DOWN  4


#define BLOCK_EMPTY				0
#define BLOCK_ICE				1
#define BLOCK_BATTERY			2
#define BLOCK_MORE				3
#define BLOCK_LIFE				4
#define BLOCK_ALCOOL			5
#define BLOCK_SHIELD			6
#define BLOCK_WALL				7
#define BLOCK_DEFENCESHIP		8
#define BLOCK_ENEMYSHIP			9
#define BLOCK_ENEMYSHOT			10
#define BLOCK_FRIENDLYSHOT		11
#define NO_EFFECT		0
#define EFFECT_ICE		1
#define EFFECT_BATTERY	2
#define EFFECT_MORE		3
#define EFFECT_ALCOOL	4
#define EFFECT_SHIELD	5

#define SHIP_SPEED 100

#define SHIP_BASIC 0
#define SHIP_DODGE 1

#define LEFT 1
#define RIGHT 2
#define UP 3
#define DOWN 4

typedef struct GameInfo {
	int Id;
	int commandId;
	int nRows;
	int nColumns;
	int boardGame[30][30];

} GameInfo, *pGameInfo;

typedef struct {
	TCHAR who[24];
	TCHAR command[20];
	TCHAR playerName[64];

	int op;
	int numLocalPlayers;
	int nRows;
	int nColumns;
	int fireTime;
	int powerUpTime;
	int typeOfGame;
	int gameObjects;
	int objects[9];
	int objectsDuration;
	int playerId;
	int difficult;
	int lifes;
	int direction;
	int dimX, dimY;
}data, *pData;


#define GameInfoSize sizeof(GameInfo)

#pragma once
#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <windows.h>


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

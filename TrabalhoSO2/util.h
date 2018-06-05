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
	int boardGame[10][10];

} GameInfo, *pGameInfo;

#define GameInfoSize sizeof(GameInfo)

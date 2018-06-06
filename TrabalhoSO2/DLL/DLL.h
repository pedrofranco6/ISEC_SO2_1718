#pragma once
#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <windows.h>

#include "../util.h"
#define DLL_EXPORTS


#ifdef DLL_EXPORTS
#define DLL_IMP_API __declspec(dllexport)
#else
#define DLL_IMP_API __declspec(dllimport)
#endif

#define MAXSIZE 24
#define objSize 24
TCHAR mapName[] = TEXT("fileMap");
typedef struct {
	char nome[10];
	int id, duracao, raridade;
}Powerup;

typedef struct {
	int x, y;
}tiros;

typedef struct {
	int x, y, vidas;
}NaveDefensora;

typedef struct {
	int x, y, vida, tipo;
	LPDWORD* threadId;
}NaveInvasora;

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

typedef struct {
	int pull;	//indice de escrita
	int push; //indice de leitura
	data data[20];

}buffer, *pBuffer;

#define bufferSize sizeof(buffer)
#define dataSize sizeof(data)
extern "C"
{

	DLL_IMP_API BOOL createGame();
	DLL_IMP_API BOOL openGame();
	DLL_IMP_API HANDLE startSyncMutex();
	DLL_IMP_API HANDLE startSyncSemaphore(LPCWSTR semaphoreName);
	DLL_IMP_API void newBuffer();
	DLL_IMP_API BOOL writeBuffer(data data);
	DLL_IMP_API data readBuffer();
	DLL_IMP_API void releaseSyncHandles(HANDLE mutex, HANDLE semaphore);
	DLL_IMP_API void setInfoSHM(GameInfo gi);
	DLL_IMP_API GameInfo getInfoSHM();
}
#pragma once
#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <windows.h>

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
	TCHAR command[objSize];
	TCHAR nicknamePlayer[64];

	int op;					
	int numLocalPlayers;	
	int nRows;
	int nColumns;

	int typeOfGame;		
	int gameObjects;
	int objects[9];
	int objectsDuration;
	int playerId;

	int direction;
	int dimX, dimY;
	NaveDefensora navesDefensoras[2];
	NaveInvasora navesInimigasBasicas[2];
	NaveInvasora navesInimigasEsquiva[2];
}data, *pData;

typedef struct allData {
	data data[20];
	int pull;
	int push;
} allData, *pAllData;

typedef struct {
	int pull;	//indice de escrita
	int push; //indice de leitura
	data data[20];

}buffer, *pBuffer;

extern "C"
{

	DLL_IMP_API BOOL writeString(int number);
	DLL_IMP_API int readString();
	DLL_IMP_API BOOL startGame();

}
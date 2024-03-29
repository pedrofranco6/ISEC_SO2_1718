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
typedef struct messageData {
	int type;
	int id;
	int command;
}MSGdata, *pMSGdata;

typedef struct {
	MSGdata buffer[20];
	int pull;
	int push;
	int messagelengh;

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
	DLL_IMP_API BOOL writeBuffer(MSGdata data);
	DLL_IMP_API MSGdata readBuffer();
	DLL_IMP_API void releaseSyncHandles(HANDLE mutex, HANDLE semaphore);
	DLL_IMP_API void setInfoSHM(GameInfo gi);
	DLL_IMP_API GameInfo getInfoSHM();
}
#include "DLL.h"
#include "../util.h"
#include <string.h>

HANDLE mapFileGame, mapFileBuffer;
pData dataView;
pBuffer bufferView;
pGameInfo gameInfoView;
HANDLE hBuffer, mBuffer, sBuffR, sBuffW;
HANDLE mutex, semaphoreReader, semaphoreWriter;


BOOL createGame() {

	mapFileGame = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		GameInfoSize,
		_TEXT("fileMap")
	);
	mapFileBuffer = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		bufferSize,
		_TEXT("bufferMap")
	);
	if (mapFileGame == NULL && mapFileBuffer == NULL) {
		_tprintf(TEXT("[DLL - ERROR] Creating File Map Object... (%d)\n"), GetLastError());
		CloseHandle(mapFileGame);
		CloseHandle(mapFileBuffer);
		return FALSE;
	}

	bufferView = (pBuffer)MapViewOfFile(
		mapFileBuffer,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		bufferSize
	);

	if (bufferView == NULL) {
		_tprintf(TEXT("[DLL - ERROR] Accessing File Map Object (BfferView)... (%d)\n"), GetLastError());
		CloseHandle(mapFileBuffer);
		return FALSE;
	}

	gameInfoView = (pGameInfo)MapViewOfFile(
		mapFileGame,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		GameInfoSize
	);

	if (bufferView == NULL || gameInfoView == NULL) {
		_tprintf(TEXT("[DLL - ERROR] Accessing File Map Object (BfferView)... (%d)\n"), GetLastError());
		CloseHandle(mapFileBuffer);
		CloseHandle(mapFileGame);
		return FALSE;
	}


	newBuffer();
	return TRUE;
}
BOOL openGame() {

	HANDLE mapFile, bufferFile;

	mapFile = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,
		FALSE,
		_TEXT("fileMap")
	);

	if (mapFile == NULL) {
		_tprintf(TEXT("[DLL - ERROR] Cannot Open File Mapping... (%d)\n"), GetLastError());
		CloseHandle(mapFile);
		return FALSE;
	}

	gameInfoView = (pGameInfo)MapViewOfFile(
		mapFile,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		dataSize
	);

	if (gameInfoView == NULL) {
		_tprintf(TEXT("[DLL - ERROR] Accessing File Map Object (dataView)... (%d)\n"), GetLastError());
		CloseHandle(mapFile);
		return FALSE;
	}

	bufferFile = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,
		FALSE,
		_TEXT("bufferMap")
	);

	if (bufferFile == NULL) {
		_tprintf(TEXT("[DLL - ERROR] Cannot Open File Mapping... (%d)\n"), GetLastError());
		CloseHandle(mapFile);
		CloseHandle(bufferFile);
		return FALSE;
	}

	bufferView = (pBuffer)MapViewOfFile(
		bufferFile,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		bufferSize
	);


	if (bufferView == NULL) {
		_tprintf(TEXT("[DLL - ERROR] Accessing File Map Object (bufferView)... (%d)\n"), GetLastError());
		CloseHandle(bufferView);
		return FALSE;
	}


	return TRUE;
}
HANDLE startSyncMutex() {

	HANDLE mutex;

	mutex = CreateMutex(NULL, FALSE, NULL);
	if (mutex == NULL) {
		_tprintf(TEXT("[ERROR] mutex wasn't created... %d"), GetLastError());
		return NULL;
	}

	return mutex;
}
HANDLE startSyncSemaphore(int sizeOfSemaphore) {
	HANDLE semaphore;

	semaphore = CreateSemaphore(
		NULL,
		sizeOfSemaphore,
		bufferSize,
		NULL);

	if (semaphore == NULL) {
		_tprintf(TEXT("[ERROR] semaphore wasn't created... %d"), GetLastError());
		return NULL;
	}

	return semaphore;
}
void newBuffer() {

	int i = 0;
	data auxData;


	auxData.op = 0;
	auxData.command[objSize] = (TCHAR) _TEXT("Empty");

	for (i; i < 20; i++) {
		bufferView->data[i] = auxData;
	}

	bufferView->pull = 0;
	bufferView->push = 0;
}


BOOL writeBuffer(data data)
{

	bufferView->data[bufferView->push] = data;
	bufferView->push = (bufferView->push + 1) % 20;

	return TRUE;
}
data readBuffer()
{

	data auxData;

	auxData = bufferView->data[bufferView->pull];
	bufferView->pull = (bufferView->pull + 1) % 20;

	return auxData;
}


void setInfoSHM(GameInfo gi) {
	gameInfoView->commandId = gi.commandId;
	gameInfoView->nRows = 30;
	gameInfoView->nColumns = 30;
	gameInfoView->Id = gi.Id;

	memcpy(gameInfoView->boardGame, gi.boardGame, sizeof(int) * 30 * 30);
}

GameInfo getInfoSHM() {
	GameInfo gi = *gameInfoView;
	return gi;
}


void releaseSyncHandles(HANDLE mutex, HANDLE semaphore) {

	ReleaseMutex(mutex);
	ReleaseSemaphore(semaphore, 1, NULL);

}






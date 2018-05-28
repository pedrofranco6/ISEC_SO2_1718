#include "DLL.h"

#include <string.h>


static allData allDataAux;


HANDLE mapFileGame, mapFileBuffer;
pData dataView;
pBuffer bufferView;
HANDLE hBuffer, mBuffer, sBuffR, sBuffW;
HANDLE mutex, semaphoreReader, semaphoreWriter;


BOOL createFileMap() {

	mapFileGame = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		sizeof(data),
		_TEXT("fileMap")
	);
	mapFileBuffer = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		sizeof(buffer),
		_TEXT("bufferMap")
	);
	if (mapFileGame == NULL && mapFileBuffer == NULL) {
		_tprintf(TEXT("[DLL - ERROR] Creating File Map Object... (%d)\n"), GetLastError());
		CloseHandle(mapFileGame);
		CloseHandle(mapFileBuffer);
		return FALSE;
	}
	return TRUE;
}

BOOL createFileViewMap() {

	dataView = (pData)MapViewOfFile(
		mapFileGame,
		FILE_MAP_ALL_ACCESS, 
		0, 
		0, 
		sizeof(data));

	if (dataView == NULL) {
		_tprintf(_T("[DLL - ERROR] Cannot Open File View Mapping... (%d)\n"));
		return FALSE;
	}
	return TRUE;
}
BOOL creatMutex() {
	mutex = CreateMutex(NULL, FALSE, _T("mutex"));
	if (mutex == NULL) {
		_tprintf(_T("[DLL - ERROR] Cannot create mutex... (%d)\n"));
		return FALSE;
	}
	return TRUE;
}

BOOL createSeamphore(int sizeOfSemaphore) {


	semaphoreWriter = CreateSemaphore(
		NULL,
		sizeOfSemaphore,
		sizeOfSemaphore,
		_T("semaphoreWriter")
		);
	semaphoreReader = CreateSemaphore(
		NULL,
		sizeOfSemaphore,
		sizeOfSemaphore,
		_T("semaphoreReader")
	);

	if (semaphoreWriter == NULL && semaphoreReader == NULL) {
		_tprintf(TEXT("[ERROR] semaphore wasn't created... %d"), GetLastError());
		return FALSE;
	}

	return TRUE;
}

BOOL openBuffer() //a ser chamado no gateway
{
	mapFileBuffer = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, _T("bufferMap"));;
	if (hBuffer == NULL) {
		_tprintf(_T("(DEBUG)DLL:Erro-> OPEN FILEMAP BUFFER\n"));
		return FALSE;
	}
	bufferView = (pBuffer)MapViewOfFile(mapFileBuffer, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(buffer));
	if (bufferView == NULL) {
		_tprintf(_T("(DEBUG)DLL:Erro-> OPEN MAP VIEW BUFFER\n"));
		return FALSE;
	}

	return TRUE;
}




BOOL writeBuffer(data data)
{
	WaitForSingleObject(semaphoreReader, INFINITE);
	WaitForSingleObject(mutex, INFINITE);

	bufferView->data[bufferView->pull] = data;
	bufferView->push = (bufferView->push + 1) % MAXSIZE;
	ReleaseMutex(mutex);
	ReleaseSemaphore(semaphoreReader, 1, NULL);


	return TRUE;
}
data readBuffer()
{


	WaitForSingleObject(semaphoreWriter, INFINITE);
	WaitForSingleObject(mutex, INFINITE);
	data auxData;

	auxData = bufferView->data[bufferView->push];
	bufferView->pull = (bufferView->pull + 1) % MAXSIZE;


	ReleaseMutex(mutex); //liberta mutex de buffer
	ReleaseSemaphore(semaphoreReader, 1, NULL); //liberta uma posição no semaforo de escritas
}

void createBuffer() {

}




void releaseSyncHandles(HANDLE mutex, HANDLE semaphore) {

	ReleaseMutex(mutex);
	ReleaseSemaphore(semaphore, 1, NULL);

}


BOOL startGame() {

	createFileMap();
	createFileViewMap();
	creatMutex();

	return true;
}






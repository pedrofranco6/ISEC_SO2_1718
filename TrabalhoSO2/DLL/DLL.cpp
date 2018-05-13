#include "DLL.h"

#include <string.h>

BOOL createFileMap() {


	mapFile = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		sizeof(Jogo),
		mapName
	);
	if (mapFile == NULL) {
		_tprintf(TEXT("[DLL - ERROR] Creating File Map Object... (%d)\n"), GetLastError());
		CloseHandle(mapFile);
		return FALSE;
	}
	return TRUE;
}


BOOL openFileMap() {


	mapFile = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,
		FALSE,
		mapName
	);

	if (mapFile == NULL) {
		_tprintf(TEXT("[DLL - ERROR] Cannot Open File Mapping... (%d)\n"), GetLastError());
		CloseHandle(mapFile);
		return FALSE;
	}

	return TRUE;

}

BOOL createFileViewMap() {
	viewMapFile = (pJogo)MapViewOfFile(
		mapFile, 
		FILE_MAP_ALL_ACCESS, 
		0, 
		0, 
		sizeof(Jogo));

	if (viewMapFile == NULL) {
		_tprintf(_T("[DLL - ERROR] Cannot Open File View Mapping... (%d)\n"));
		return FALSE;
	}
}
BOOL creatMutex() {

	mutex = CreateMutex(NULL, FALSE, _T("mutex"));
	if (mutex == NULL) {
		_tprintf(_T("[DLL - ERROR] Cannot create mutex... (%d)\n"));
		return FALSE;
	}
}

pJogo getGame()
{
	
	if (viewMapFile != NULL)
	{
		return viewMapFile;
	}
	else
		return NULL;

}


BOOL writeString(int number)
{
	WaitForSingleObject(mutex, INFINITE);
		viewMapFile->cmd = number;

	ReleaseMutex(mutex);


	return TRUE;
}
int readString()
{


	WaitForSingleObject(mutex, INFINITE);
	int aux;
	aux = viewMapFile->cmd;


	ReleaseMutex(mutex);
	return aux;
}


BOOL startGame() {

	createFileMap();
	createFileViewMap();
	creatMutex();

	return true;
}



#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <windows.h>
#include "../DLL/DLL.h"

HMODULE DLL;
HANDLE *handleThreadsNavesInimigas;
DWORD threadIds;
HANDLE threadTrinco;
HANDLE handleMsgs;
BOOL(*startgame) ();
BOOL(*writeB)(data data);
data(*readB) ();
HANDLE hThreadSharedMemory;

HANDLE eventReader;
HANDLE eventWriter;


data setUpJogo() {
	data jogo;
	jogo.dimX = 1000;
	jogo.dimY = 1000;

	return jogo;
}
DWORD WINAPI threadbasicas(LPVOID data) {
	threadTrinco = CreateMutex(NULL, FALSE, _T("threadMutex"));
	
	while (1) {
		WaitForSingleObject(threadTrinco, INFINITE);
		//send data

		ReleaseMutex(threadTrinco);

	}

	return 0;
}

DWORD WINAPI threadesquivas(LPVOID data) {
	threadTrinco = CreateMutex(NULL, FALSE, _T("threadMutex"));

	while (1) {
		WaitForSingleObject(threadTrinco, INFINITE);
		//send data
		ReleaseMutex(threadTrinco);

	}

	return 0;
}

DWORD WINAPI awaitMessages(LPVOID dados) {

	int optionAux = 0;
	data option;
	while (1) {
		
		option = readB();
		_tprintf(TEXT("\n chegou do gateway: %d\n"), option.op);
		}
	}



void gerarNavesInimigas(data jogo) {
	int i;
	int navesInimigasBasicas = sizeof(jogo.navesInimigasBasicas) / sizeof(jogo.navesInimigasBasicas[0]);
	int navesInimigasEsquivas = sizeof(jogo.navesInimigasEsquiva) / sizeof(jogo.navesInimigasEsquiva[0]);
	int navesInimigas = navesInimigasBasicas + navesInimigasEsquivas;

	handleThreadsNavesInimigas = (HANDLE*)malloc(navesInimigas * sizeof(HANDLE));

	for (i = 0; i < navesInimigasBasicas; i++) {
		handleThreadsNavesInimigas[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadbasicas, NULL, 0, &threadIds);
	}

	for (i = 0; i < navesInimigasEsquivas; i++) {
		handleThreadsNavesInimigas[i + navesInimigasBasicas] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadesquivas, NULL, 0, &threadIds);
	}
	_tprintf(_T("Naves inimigas criadas com sucesso"));
	
}



DWORD WINAPI listenClientSharedMemory(LPVOID params) {
	data dataGame;
	eventReader = CreateEvent(NULL, TRUE, FALSE, TEXT("Global\eventReader"));
	eventWriter = CreateEvent(NULL, TRUE, FALSE, TEXT("Global\eventWriter"));

	while (1) {
		data(*getData)();

		//Wait for any client trigger the event by typing any option
		WaitForSingleObject(eventReader, INFINITE);

		
		//GETDATA IN CORRECT PULL POSITION
		getData = (data(*)()) GetProcAddress(DLL, "readBuffer");
		if (getData == NULL) {
			_tprintf(TEXT("[SHM ERROR] Loading getDataSHM function from DLL (%d)\n"), GetLastError());
			return NULL;
		}
		dataGame = getData();
		_tprintf(TEXT("Lido: %d"), dataGame.op);
		ResetEvent(eventReader);
	}
}


void initializeSharedMemory() {

	pBuffer circularBufferPointer = NULL;

	BOOL(*createFileMap)();
	pBuffer(*getBuffer)();


	_tprintf(TEXT("STARTING SHARED MEMORY....................\n"));

	//CREATEFILEMAP
	createFileMap = (BOOL(*)()) GetProcAddress(DLL, "createGame");
	if (createFileMap == NULL) {
		_tprintf(TEXT("[SHM ERROR] Loading createFileMapping function from DLL (%d)\n"), GetLastError());
		return;
	}

	if (!createFileMap()) {
		_tprintf(TEXT("[SHM ERROR] Creating File Map Object... (%d)"), GetLastError());
		return;
	}

	//CREATE A THREAD RESPONSABLE FOR SHM ONLY
	hThreadSharedMemory = CreateThread(
		NULL,
		0,
		listenClientSharedMemory,
		NULL,
		0,
		0);

	if (hThreadSharedMemory == NULL) {
		_tprintf(TEXT("[ERROR] Creating Shared Memory Thread... (%d)"), GetLastError());
		return;
	}
	_tprintf(TEXT("Shared Memory has started correctly.......\n"));
}


int _tmain()
{
	data jogo;
	HANDLE hMapFile;

	DLL = LoadLibrary(_T("DLL"));
	if (DLL == NULL) {
		_tprintf(_T("[ERROR] Loading DLL!!"));
		return 0;
	}
	else
		_tprintf(_T("DLL lida com sucesso!\n"));

	writeB = (BOOL(*)(data data))GetProcAddress(DLL, "writeBuffer");
	readB = (data(*)())GetProcAddress(DLL, "readBuffer");
	startgame = (BOOL(*)())GetProcAddress(DLL, "createGame");

	if (writeB == NULL || readB == NULL || startgame == NULL) {
		_tprintf(TEXT("[SHM ERROR] Loading function from DLL (%d)\n"), GetLastError());
		return 0;
	}

	startgame();

	initializeSharedMemory();
	WaitForSingleObject(hThreadSharedMemory, INFINITE);

    return 0;
}



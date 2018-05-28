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
int (*message)();


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

DWORD WINAPI awaitMessages(LPVOID data) {

	int option, optionAux = 0;
	while (1) {
		
		option = message();
		_tprintf(TEXT("\n chegou do gateway: %d\n"), optionAux);
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

int main()
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

	message = (int(*)())GetProcAddress(DLL, "readString");
	startgame = (BOOL(*)())GetProcAddress(DLL, "startGame");

	if (message == NULL || startgame == NULL) {
		_tprintf(TEXT("[SHM ERROR] Loading function from DLL (%d)\n"), GetLastError());
		return 0;
	}

	startgame();
	jogo = setUpJogo();
	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(jogo), TEXT("jogo"));

	handleMsgs = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)awaitMessages, NULL, 0, 0);

	gerarNavesInimigas(jogo);
	while (1);
    return 0;
}


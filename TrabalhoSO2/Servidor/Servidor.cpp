#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <windows.h>
#include "../DLL/DLL.h"

HANDLE *handleThreadsNavesInimigas;
DWORD *threadIds;

Jogo setUpJogo() {
	Jogo jogo;
	jogo.dimX = 1000;
	jogo.dimY = 1000;

	return jogo;
}

void gerarNavesInimigas(Jogo jogo) {
	int i;
	int navesInimigasBasicas = sizeof(jogo.navesInimigasBasicas) / sizeof(jogo.navesInimigasBasicas[0]);
	int navesInimigasEsquivas = sizeof(jogo.navesInimigasEsquiva) / sizeof(jogo.navesInimigasEsquiva[0]);
	int navesInimigas = navesInimigasBasicas + navesInimigasEsquivas;

	handleThreadsNavesInimigas = (HANDLE*)malloc(navesInimigas * sizeof(HANDLE));

	for (i = 0; i < navesInimigasBasicas; i++) {
		handleThreadsNavesInimigas[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)navesInimigas, NULL, 0, &threadIds[i]);
	}

	for (i = 0; i < navesInimigasEsquivas; i++) {
		handleThreadsNavesInimigas[i + navesInimigasBasicas] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)navesInimigas, NULL, 0, &threadIds[i + navesInimigasBasicas]);
	}
}

int main()
{
	Jogo jogo;
	HANDLE hMapFile;

	jogo = setUpJogo();
	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(jogo), TEXT("jogo"));
	
	gerarNavesInimigas(jogo);


    return 0;
}


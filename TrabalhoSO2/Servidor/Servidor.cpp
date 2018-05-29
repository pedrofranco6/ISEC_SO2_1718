#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>
#include "../DLL/DLL.h"
#include "servidor.h"
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

Game game;
GameInfo gameInfo;
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


void initGame(data dataGame) {
	game.nRows = dataGame.dimX;
	game.nColumns = dataGame.dimY;
	game.Created = TRUE;
	game.running = FALSE;
	game.nPlayers = 0;
	game.objectsDuration = dataGame.objectsDuration;
	game.nObjects = dataGame.gameObjects;
	game.difficult = dataGame.difficult;
	game.lifes = dataGame.lifes;
	game.fireTime = dataGame.fireTime;
	game.powerUpTime = dataGame.powerUpTime;

	game.boardGame = (int **)malloc(sizeof(int) * game.nRows);
	for (int i = 0; i < game.nRows; i++) {
		game.boardGame[i] = (int *)malloc(sizeof(int)* game.nColumns);
	}

	for (int i = 0; i < game.nRows; i++) {
		for (int j = 0; j < game.nColumns; j++) {
			game.boardGame[i][j] = BLOCK_EMPTY;
			if (i == 0 || j == 0 || i == game.nRows - 1 || j == game.nColumns - 1) {
				game.boardGame[i][j] = BLOCK_WALL;
			}
		}
	}


}

void initObjects() {

	for (int i = 0; i < game.nObjects; i++)
	{
		game.object[i] = initRandomObject();
	}

}
Objects initRandomObject() {

	Object aux;
	int x, y;
	srand(time(NULL));
	do {
		x = rand() % game.nColumns;
	} while (game.boardGame[x][0] != 0);

	aux.x = x;
	aux.y = 0;
	aux.duration = game.objectsDuration;
	int nGenerated = rand() % 100 + 1;

	if (nGenerated < 25) {

	}
	else if (nGenerated < 50 && nGenerated > 25) {
		game.boardGame[x][y] = BLOCK_SHIELD;
		aux.block = BLOCK_SHIELD;
	}
	else if (nGenerated < 65 && nGenerated > 50) {
		game.boardGame[x][y] = BLOCK_MORE;
		aux.block = BLOCK_MORE;
	}
	else if (nGenerated < 80 && nGenerated > 65) {
		game.boardGame[x][y] = BLOCK_ICE;
		aux.block = BLOCK_ICE;
	}
	else if (nGenerated < 95 && nGenerated > 80) {
		game.boardGame[x][y] = BLOCK_BATTERY;
		aux.block = BLOCK_BATTERY;
	}
	else if (nGenerated < 95 && nGenerated > 80) {
		game.boardGame[x][y] = BLOCK_ALCOOL;
		aux.block = BLOCK_ALCOOL;
	}
	else {
		_tprintf(_T("Error creating objects"));
	}
	return aux;
}

void ObjectEffect(int block, int player) {

	switch (block)
	{
	case BLOCK_SHIELD:
		game.playerShips[player].effect = EFFECT_SHIELD;
		game.playerShips[player].timeEffect = game.objectsDuration;
		break;
	case BLOCK_ICE:
		for (int i = 0; i < game.nInvadesBasic + game.nInvadesDodge; i++)
			game.invadeShips[i].blocked = TRUE;
		game.playerShips[player].effect = EFFECT_ICE;
		game.playerShips[player].timeEffect = game.objectsDuration;
		break;
	case BLOCK_BATTERY:
		for (int i = 0; i < game.nPlayers; i++)
			game.playerShips[i].speed *= 2;
		game.playerShips[player].effect = EFFECT_BATTERY;
		game.playerShips[player].timeEffect = game.objectsDuration;
		break;
	case BLOCK_LIFE:
		game.playerShips[player].vidas++;
		break;
	case BLOCK_ALCOOL:
		game.playerShips[player].effect = EFFECT_ALCOOL;
		game.playerShips[player].timeEffect = game.objectsDuration;
		break;

	default:
		break;
	}
}


DefenceShip initShip(int id) {
	DefenceShip defenceShip;
	int x, y;


	do {
		srand(time(NULL));
		x = rand() % ((game.nColumns - game.nColumns / 20) - 1) + 1;
		srand(time(NULL));
		y = rand() % (game.nRows - 1) + 1;
	} while (!game.boardGame[x][y]);
	game.boardGame[x][y] = BLOCK_DEFENCESHIP;
	defenceShip.id = id;
	defenceShip.alive = TRUE;
	defenceShip.x = x;
	defenceShip.y = y;
	defenceShip.speed = SHIP_SPEED;
	defenceShip.effect = NO_EFFECT;
	return defenceShip;
}




void joinGame(data dataGame) {

	// Initialize the Player and the Snake
	_tcscpy(game.playerShips[game.nPlayers].user, dataGame.playerName);
	game.playerShips[game.nPlayers] = initShip(game.nPlayers);
	game.nPlayers++;


	gameInfo.commandId = JOIN_GAME;
	gameInfo.Id = dataGame.playerId;
	//sendInfoToPlayers(gameInfo);


}


void gerarNavesInimigas(data jogo) {
	int i;
	int navesInimigasBasicas = game.nInvadesBasic;
	int navesInimigasEsquivas = game.nInvadesDodge;
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



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
DWORD timerThread;
HANDLE eventReader;
HANDLE eventWriter;
HANDLE users[MAXCLIENTS];
Game game;
GameInfo gameInfo;
int invadeShipId;
data setUpJogo() {
	data jogo;
	jogo.dimX = 1000;
	jogo.dimY = 1000;

	return jogo;
}

DWORD WINAPI threadPowerUp(LPVOID data) {

	int player = *static_cast<int*>(data);
	int time = 0;
	delete data;

	while (time < game.objectsDuration) {

		Sleep(1000);
		time++;
	}
	switch (game.playerShips[player].effect) {

	case BLOCK_ICE:
		for (int i = 0; i < game.nInvadesBasic + game.nInvadesDodge; i++)
			game.invadeShips[i].blocked = FALSE;

		break;
	case BLOCK_BATTERY:
		for (int i = 0; i < game.nPlayers; i++)
			game.playerShips[i].speed /= 2;

		break;
	}
	game.playerShips[player].effect = NO_EFFECT;
	return 0;


}
DWORD WINAPI threadbasicas(LPVOID data) {
	threadTrinco = CreateMutex(NULL, FALSE, _T("threadMutex"));
	int i = invadeShipId;
	delete data;
	int x;
	int fire = game.fireTime;
	game.invadeShips[i].tipo = SHIP_BASIC;
	game.invadeShips[i].vida = 1;
	int pos = 1;
	do {
		srand((unsigned int)time(NULL));
		x = rand() % ((game.nColumns) - 1) + 1;
	} while (game.boardGame[x][1] != 0);
	game.invadeShips[i].x = x;
	game.invadeShips[i].y = 0;
	game.boardGame[x][1] == BLOCK_ENEMYSHIP;
	while (1) {
		WaitForSingleObject(threadTrinco, INFINITE);
		
		Sleep(SHIP_SPEED * 10);
		if (x == game.nColumns - 1)
		{
			game.invadeShips[i].y++;
			pos = -1;
		}
		else if (x == 0) {
			game.invadeShips[i].y++;
			pos = 1;
		}
		game.invadeShips[i].x += pos;
		if (fire == 0) {
			fire = game.fireTime;
			//FIREEEEEEE()
		}
		else
			fire--;
		ReleaseMutex(threadTrinco);

	}

	return 0;
}

DWORD WINAPI threadesquivas(LPVOID data) {
	int i = (int) data;
	int aux;
	_tprintf(TEXT("\n chegou do gateway: int : %d\n"), i);
	int x = 0, y = 0;
	int fire = 1.4*game.fireTime;
	game.invadeShips[i].tipo = SHIP_DODGE;
	game.invadeShips[i].vida = 3;
	do {
		srand((unsigned int)time(NULL));
		x = rand() % ((game.nRows) - 1) + 1;
		_tprintf(TEXT("\n chegou do gateway: x: %d\n"), x);
	} while (game.boardGame[x][1] != 0);
	game.invadeShips[i].x = x;
	game.invadeShips[i].y = 1;
	_tprintf(TEXT("\n chegou do gateway: %d %d\n"),x, game.invadeShips[i].y);
	game.boardGame[x][1] = BLOCK_ENEMYSHIP;
	while (1) {
		Sleep(SHIP_SPEED * 11);
		do {

			do {
				aux = rand() % 4;
				switch (aux)
				{
				case(0):
					y++; break;
				case(1):
					y--; break;
				case(2):
					x++; break;
				case(3):
					x--; break;
				}
			} while ((y + game.invadeShips[i].y <= game.nColumns-1 || x + game.invadeShips[i].x <= game.nRows-1));
		} while (  game.boardGame[x + game.invadeShips[i].x][ y + game.invadeShips[i].y] != 0);
		game.boardGame[game.invadeShips[i].x][game.invadeShips[i].y] = BLOCK_EMPTY;
		game.boardGame[x + game.invadeShips[i].x][y + game.invadeShips[i].y] = BLOCK_ENEMYSHIP;
		if (fire == 0) {
			fire = 1.4*game.fireTime;
			//FIREEEEEEE()
		}
		else
			fire--;

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
	game.nRows = dataGame.nRows;
	game.nColumns = dataGame.nColumns;
	game.Created = TRUE;
	game.running = FALSE;
	game.nPlayers = 0;
	game.objectsDuration = dataGame.objectsDuration;
	game.nObjects = dataGame.gameObjects;
	game.difficult = dataGame.difficult;
	game.lifes = dataGame.lifes;
	game.fireTime = dataGame.fireTime;
	game.powerUpTime = dataGame.powerUpTime;
	game.nInvadesBasic = 0;
	game.nInvadesDodge = 2;
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

void auxGameForNow() {
	data dataGame;
	dataGame.difficult = 1;
	dataGame.nColumns = 10;
	dataGame.nRows = 10;
	dataGame.direction = 10;
	dataGame.fireTime = 5;
	dataGame.gameObjects = 4;
	dataGame.lifes = 3;
	dataGame.objectsDuration = 5;
	dataGame.powerUpTime = 5;
	dataGame.fireTime = 5;
	initGame(dataGame);
}


Objects initRandomObject() {

	Object aux;
	unsigned int x;
	srand((unsigned int)time(NULL));
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
		game.boardGame[x][0] = BLOCK_SHIELD;
		aux.block = BLOCK_SHIELD;
	}
	else if (nGenerated < 65 && nGenerated > 50) {
		game.boardGame[x][0] = BLOCK_MORE;
		aux.block = BLOCK_MORE;
	}
	else if (nGenerated < 80 && nGenerated > 65) {
		game.boardGame[x][0] = BLOCK_ICE;
		aux.block = BLOCK_ICE;
	}
	else if (nGenerated < 95 && nGenerated > 80) {
		game.boardGame[x][0] = BLOCK_BATTERY;
		aux.block = BLOCK_BATTERY;
	}
	else if (nGenerated < 95 && nGenerated > 80) {
		game.boardGame[x][0] = BLOCK_ALCOOL;
		aux.block = BLOCK_ALCOOL;
	}
	else {
		_tprintf(_T("Error creating objects"));
	}
	return aux;
}


void initObjects() {

	for (int i = 0; i < game.nObjects; i++)
	{
		game.object[i] = initRandomObject();
	}

}



void ObjectEffect(int block, int player) {
	HANDLE thread;
	switch (block)
	{
	case BLOCK_SHIELD:
		game.playerShips[player].effect = EFFECT_SHIELD;
		thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadPowerUp, &player, 0, &timerThread);

		break;
	case BLOCK_ICE:
		for (int i = 0; i < game.nInvadesBasic + game.nInvadesDodge; i++)
			game.invadeShips[i].blocked = TRUE;
		game.playerShips[player].effect = EFFECT_ICE;
		thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadPowerUp, &player, 0, &timerThread);
		break;
	case BLOCK_BATTERY:
		for (int i = 0; i < game.nPlayers; i++)
			game.playerShips[i].speed *= 2;
		game.playerShips[player].effect = EFFECT_BATTERY;
		thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadPowerUp, &player, 0, &timerThread);
		break;
	case BLOCK_LIFE:
		game.playerShips[player].vidas++;
		thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadPowerUp, &player, 0, &timerThread);
		break;
	case BLOCK_ALCOOL:
		game.playerShips[player].effect = EFFECT_ALCOOL;
		thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadPowerUp, &player, 0, &timerThread);
		break;

	default:
		break;
	}
}


DefenceShip initShip(int id) {
	DefenceShip defenceShip;
	int x, y;


	do {
		srand((unsigned int)time(NULL));
		x = rand() % ((game.nColumns - game.nColumns / 20) - 1) + 1;
		srand((unsigned int)time(NULL));
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

	wcscpy_s(game.playerShips[game.nPlayers].user, dataGame.playerName);
	game.playerShips[game.nPlayers] = initShip(game.nPlayers);
	game.nPlayers++;


	gameInfo.commandId = JOIN_GAME;
	gameInfo.Id = dataGame.playerId;
	//sendInfoToPlayers(gameInfo);


}


void gerarNavesInimigas() {
	int i;
	int navesInimigasBasicas = game.nInvadesBasic;
	int navesInimigasEsquivas = game.nInvadesDodge;
	int navesInimigas = navesInimigasBasicas + navesInimigasEsquivas;

	handleThreadsNavesInimigas = (HANDLE*)malloc(navesInimigas * sizeof(HANDLE));

	for (i = 0; i < navesInimigasBasicas; i++) {
		invadeShipId = i;
		handleThreadsNavesInimigas[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadbasicas, NULL, 0,&threadIds);
	}

	for (i = 0; i < navesInimigasEsquivas; i++) {
		invadeShipId = i;
		handleThreadsNavesInimigas[i + navesInimigasBasicas] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadesquivas, (LPVOID)i, 0, &threadIds);
	}
	_tprintf(_T("Naves inimigas criadas com sucesso\n"));

}






DWORD WINAPI listenClientSharedMemory(LPVOID params) {
	data dataGame;
	eventReader = CreateEvent(NULL, TRUE, FALSE, TEXT("Global\eventReader"));
	eventWriter = CreateEvent(NULL, TRUE, FALSE, TEXT("Global\eventWriter"));

	do  {
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
	} while ((dataGame.op != -1));
}


void initializeSharedMemory() {

	BOOL(*createFileMap)();


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


	auxGameForNow();

	gerarNavesInimigas();
	Sleep(3000);
	system("cls");
	while (1) {
		for (int j = 0; j < game.nRows; j++) {
			for (int i = 0; i < game.nColumns; i++) {
				_tprintf(_T("%d"), game.boardGame[i][j]);
			}
			_tprintf(_T("\n"));
		}
		Sleep(2000);
	}
	return 0;
}



#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>
#include "../DLL/DLL.h"
#include "servidor.h"
#include "resource.h"

// CONFIG 
HANDLE users[MAXCLIENTS];
HINSTANCE hThisInst;
int windowMode = 0;
HWND hWnd;
HMODULE DLL;

// GAME
Game game;
GameInfo gameInfo;

int invadeShipId;
//DLL FUNCTIONS
BOOL(*startgame) ();
BOOL(*writeB)(data data);
data(*readB) ();
void(*setGameInfo) (GameInfo gi);
HANDLE(*startMutex)();
HANDLE(*startSemaphore)(LPCWSTR semaphoreName);

// THREADS-MUTEX HANDLES
HANDLE GameThread;
HANDLE *handleThreadsNavesInimigas;
HANDLE TrincoOfThreads;
DWORD threadIds;
HANDLE handleMsgs;
HANDLE hThreadSharedMemory;
DWORD timerThread;
HANDLE eventReader;
HANDLE eventWriter;
HANDLE canWrite, canRead;

// FUNCTIONS
void GameInfoSend() {
	WaitForSingleObject(TrincoOfThreads, INFINITE);
	gameInfo.commandId = 1;
	gameInfo.Id = 1;	// For all players see
	gameInfo.nRows = game.nRows;
	gameInfo.nColumns = game.nColumns;

	for (int i = 0; i < game.nRows; i++) {
		for (int j = 0; j < game.nColumns; j++) {
			gameInfo.boardGame[i][j] = game.boardGame[i][j];
		}
	}
	sendGameInfo(gameInfo);
	SetEvent(eventReader);
	ReleaseMutex(TrincoOfThreads);
}


DWORD WINAPI gameThread(LPVOID params) {
	_tprintf(TEXT("\n-----GAMETHREAD----\n"));
	game.running = 1;
	//	gameCount = 0;

	while (game.running == 1) {
		//	gameCount++;
		//	moveShips();
		//	manageObjects();

		//	updateGameInfoBoard();
		GameInfoSend();

		//	verifyEndGame();

	//	Sleep(450);

	}
	return 0;
}

BOOL endGame() {

	for (int i = 0; i < MAXCLIENTS; i++)
	{
		if (game.playerShips[i].alive) {
			return FALSE;
		}
	}
	for (int i = 0; i < MAXSHIPS; i++)
	{
		if (game.invadeShips[i].alive) {
			return FALSE;
		}
	}


	game.Created = FALSE;
	game.running = FALSE;
	return TRUE;
}


void moveShip(int id, int dir) {
	int i;
	for (i = 0; i < MAXCLIENTS; i++) {
		if (game.playerShips[i].id == id && game.playerShips[i].alive) {
			break;
		}
	}
	if (game.playerShips[i].effect == EFFECT_ALCOOL) {
		switch (dir)
		{
		case RIGHT:
			dir = LEFT;
			break;
		case LEFT:
			dir = RIGHT;
			break;
		case UP:
			dir = DOWN;
			break;
		case DOWN:
			dir = UP;
			break;
		default:
			break;
		}
		//FALTA MOVER
	}
}

void manageCommands(data dataGame) {
	switch (dataGame.op) {
	case EXIT:
		_tprintf(TEXT("Goodbye.."));
		break;

		break;
	case JOIN_GAME:
		if (game.Created && !game.running) {
			joinGame(dataGame);
		}
		else {
			gameInfo.commandId = ERROR_CANNOT_JOIN_GAME;
			gameInfo.Id = dataGame.playerId;
			setGameInfo(gameInfo);
		}
		break;
	case SCORES:
		break;
	case MOVE:
		moveShip(dataGame.playerId, dataGame.direction);
		break;
	default:
		break;
	}
}


BOOL hasColision(int x, int y, int type) {
	if (type == 1) {

		for (int i = x; i < x + 3; i++)
			for (int j = y; j < y + 3; j++)
				if (game.boardGame[i][j] != BLOCK_EMPTY)
					return true;
	}
	else if (type == 2) {

		for (int i = x; i < x + 2; i++)
			for (int j = y; j < y + 2; j++)
				if (game.boardGame[i][j] != BLOCK_EMPTY)
					return true;
	}
	else

		return false;
}
void drawBlock(int x, int y, int type, int blockType) {

	if (type == 1) {
		for (int i = x; i < x + 3; i++)
			for (int j = y; j < y + 3; j++)
				game.boardGame[i][j] = blockType;
	}
	else if (type == 2) {
		for (int i = x; i < x + 2; i++)
			for (int j = y; j < y + 2; j++)
				game.boardGame[i][j] = blockType;
	}
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

DWORD WINAPI EnemyFire(LPVOID data) {
	InvadeShip *enemyShip = (InvadeShip *)data;
	int x = enemyShip->x + 2, y = enemyShip->y + +3;
	game.boardGame[x][y] = BLOCK_ENEMYSHOT;
	do {
		Sleep(SHIP_SPEED * 10);
		WaitForSingleObject(TrincoOfThreads, INFINITE);
		game.boardGame[x][y] = BLOCK_EMPTY;
		y++;
		game.boardGame[x][y] = BLOCK_ENEMYSHOT;
		ReleaseMutex(TrincoOfThreads);
	} while (y != game.nRows - 2);
	Sleep(SHIP_SPEED * 10);
	game.boardGame[x][y] = BLOCK_EMPTY;
	return 0;

}
DWORD WINAPI PowerUpTimeWait(LPVOID data) {
	Object obj;
	do {
		
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PowerUp, NULL, 0, NULL);
		Sleep(game.powerUpTime * 1000);
	} while (game.running);

	return 0;
}

DWORD WINAPI PowerUp(LPVOID data) {
	Object object;
	object.block = initRandomObject();
	int x, y = 1;

	do {
		srand((unsigned int)time(NULL));
		x = rand() % ((game.nRows) - 1) + 1;
	} while (hasColision(x, y, 2) == true);
	drawBlock(x, y, 2, object.block);
	do {

		Sleep(SHIP_SPEED * 10);
		WaitForSingleObject(TrincoOfThreads, INFINITE);
		drawBlock(x, y, 2, BLOCK_EMPTY);
		y++;
		drawBlock(x, y, 2, object.block);
		ReleaseMutex(TrincoOfThreads);
	} while (y != game.nRows - 1);
	Sleep(SHIP_SPEED * 10);
	drawBlock(x, y, 2, BLOCK_EMPTY);
	return 0;

}

DWORD WINAPI threadbasicas(LPVOID data) {
	WaitForSingleObject(TrincoOfThreads, INFINITE);
	int i = (int)data;
	int x;
	boolean changedpos = false;
	int fire = game.fireTime;
	game.invadeShips[i].tipo = SHIP_BASIC;
	game.invadeShips[i].vida = 1;
	int pos = 1;
	do {
		srand((unsigned int)time(NULL));
		x = rand() % ((game.nRows) - 1) + 1;
	} while (hasColision(x, 1, 1) == true);
	game.invadeShips[i].x = x;
	game.invadeShips[i].y = 1;
	drawBlock(x, 1, 1, BLOCK_ENEMYSHIP);
	ReleaseMutex(TrincoOfThreads);

	while (1) {
		WaitForSingleObject(TrincoOfThreads, INFINITE);

		Sleep(SHIP_SPEED * 10);
		drawBlock(game.invadeShips[i].x, game.invadeShips[i].y, 1, BLOCK_EMPTY);

		if (game.invadeShips[i].x == game.nRows - 4 && changedpos == false)
		{
			game.invadeShips[i].y++;
			pos = -1;
			changedpos = true;

		}
		else if (game.invadeShips[i].x == 1 && changedpos == false) {
			game.invadeShips[i].y++;
			pos = 1;
			changedpos = true;
		}
		else {
			game.invadeShips[i].x += pos;
			changedpos = false;
		}
		drawBlock(game.invadeShips[i].x, game.invadeShips[i].y, 1, BLOCK_ENEMYSHIP);
		if (fire == 0) {
			fire = game.fireTime;
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)EnemyFire, &game.invadeShips[i], 0, &threadIds);

		}
		else
			fire--;
		ReleaseMutex(TrincoOfThreads);
	}

	return 0;
}

DWORD WINAPI threadesquivas(LPVOID data) {
	WaitForSingleObject(TrincoOfThreads, INFINITE);
	int i = (int)data;
	int aux;
	int x = 0, y = 1;
	int fire = 1.4*game.fireTime;
	game.invadeShips[i].tipo = SHIP_DODGE;
	game.invadeShips[i].vida = 3;
	do {
		srand((unsigned int)time(NULL));
		x = rand() % ((game.nRows) - 1) + 1;
	} while (hasColision(x, y, 1) == true);
	game.invadeShips[i].x = x;
	game.invadeShips[i].y = 1;
	drawBlock(x, y, 1, BLOCK_ENEMYSHIP);
	ReleaseMutex(TrincoOfThreads);

	while (1) {
		WaitForSingleObject(TrincoOfThreads, INFINITE);
		Sleep(SHIP_SPEED * 11);
		drawBlock(game.invadeShips[i].x, game.invadeShips[i].y, 1, BLOCK_EMPTY);
		do {


			aux = rand() % 4;
			x = 0, y = 0;
			switch (aux)
			{
			case(0):
				y = 1; break;
			case(1):
				y = -1; break;
			case(2):
				x = 1; break;
			case(3):
				x = -1; break;
			}

		} while (hasColision(x + game.invadeShips[i].x, y + game.invadeShips[i].y, 1) == true);
		_tprintf(TEXT("passou"));

		game.invadeShips[i].x = x + game.invadeShips[i].x;
		game.invadeShips[i].y = y + game.invadeShips[i].y;
		drawBlock(game.invadeShips[i].x, game.invadeShips[i].y, 1, BLOCK_ENEMYSHIP);
		if (fire == 0) {
			fire = 1.4*game.fireTime;
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)EnemyFire, &game.invadeShips[i], 0, &threadIds);

		}
		else
			fire--;
		ReleaseMutex(TrincoOfThreads);
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
	dataGame.nColumns = 30;
	dataGame.nRows = 30;
	dataGame.direction = 10;
	dataGame.fireTime = 5;
	dataGame.gameObjects = 4;
	dataGame.lifes = 3;
	dataGame.objectsDuration = 5;
	dataGame.powerUpTime = 10;
	dataGame.fireTime = 5;
	initGame(dataGame);
}


int initRandomObject() {
	srand((unsigned int)time(NULL));
	int nGenerated = rand() % 100 + 1;

	if (nGenerated < 25) {
		return BLOCK_MORE;
	}
	else if (nGenerated < 50 && nGenerated > 25) {
		return BLOCK_SHIELD;
	}
	else if (nGenerated < 65 && nGenerated > 50) {
		return BLOCK_ALCOOL;
	}
	else if (nGenerated < 80 && nGenerated > 65) {
		return BLOCK_ICE;
	}
	else if (nGenerated < 95 && nGenerated > 80) {
		return BLOCK_BATTERY;
	}
	else if (nGenerated > 95) {
		return BLOCK_LIFE;
	}
	else {
		_tprintf(_T("Error creating object"));
	}
	return -1; //ERROR
}


//START ALL CLIENTS WITH NULL
void StartPlayerShips() {
	for (int i = 0; i < MAXCLIENTS; i++) {
		users[i] = NULL;
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
	} while (hasColision(x, y, 1));
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
		handleThreadsNavesInimigas[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadbasicas, (LPVOID)i, 0, &threadIds);
	}

	for (i = 0; i < navesInimigasEsquivas; i++) {
		invadeShipId = i;
		handleThreadsNavesInimigas[i + navesInimigasBasicas] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)threadesquivas, (LPVOID)i, 0, &threadIds);
	}
	_tprintf(_T("Naves inimigas criadas com sucesso\n"));
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PowerUpTimeWait, NULL, 0, NULL);

}


DWORD WINAPI listenClientSharedMemory(LPVOID params) {
	data(*getData)();
	data dataGame;
	HANDLE canWrite, canRead;
	eventReader = CreateEvent(NULL, TRUE, FALSE, TEXT("Global\eventReader"));
	eventWriter = CreateEvent(NULL, TRUE, FALSE, TEXT("Global\eventWriter"));

	canWrite = startSemaphore(L"writeSemaphore");
	canRead = startSemaphore(L"readSemaphore");

	getData = (data(*)()) GetProcAddress(DLL, "readBuffer");
	if (getData == NULL) {
		_tprintf(TEXT("[SHM ERROR] Loading getDataSHM function from DLL (%d)\n"), GetLastError());
		return NULL;
	}
	do {

		WaitForSingleObject(canRead, INFINITE);

		//GETDATA IN CORRECT POSITION

		dataGame = getData();
		manageCommands(dataGame);
		_tprintf(TEXT("Lido: %d"), dataGame.op);
		ReleaseSemaphore(canWrite, 1, NULL);
	} while ((dataGame.op != -1));
	return 0;
}

void sendGameInfo(GameInfo gi) {
	setGameInfo(gi);
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


LRESULT CALLBACK DialogConfig(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	TCHAR aux[24];
	switch (Msg)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		switch (wParam)
		{
		case IDOK:
			GetDlgItemText(hWndDlg, IDC_linhas, aux, 20);
			game.nRows = _wtoi(aux);
			GetDlgItemText(hWndDlg, IDC_colunas, aux, 20);
			game.nColumns = _wtoi(aux);
			GetDlgItemText(hWndDlg, IDC_basicas, aux, 20);
			game.nInvadesBasic = _wtoi(aux);
			GetDlgItemText(hWndDlg, IDC_esquivas, aux, 20);
			game.nInvadesDodge = _wtoi(aux);
			GetDlgItemText(hWndDlg, IDC_dificuldade, aux, 20);
			game.difficult = _wtoi(aux);
			GetDlgItemText(hWndDlg, IDC_vidas, aux, 20);
			game.lifes = _wtoi(aux);
			GetDlgItemText(hWndDlg, IDC_poweruptempo, aux, 20);
			game.powerUpTime = _wtoi(aux);
			GetDlgItemText(hWndDlg, IDC_velDisparo, aux, 20);
			game.fireTime = _wtoi(aux);



			EndDialog(hWndDlg, 0);
			return TRUE;
		}
		break;
	}

	return FALSE;
}

int _tmain() {


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
	startMutex = (HANDLE(*)())GetProcAddress(DLL, "startSyncMutex");
	setGameInfo = (void(*)(GameInfo gi))GetProcAddress(DLL, "setInfoSHM");
	startSemaphore = (HANDLE(*)(LPCWSTR semaphoreName))GetProcAddress(DLL, "startSyncSemaphore");

	if (writeB == NULL || readB == NULL || startgame == NULL || setGameInfo == NULL || startSemaphore == NULL) {
		_tprintf(TEXT("[SHM ERROR] Loading functions from DLL (%d)\n"), GetLastError());
		return 0;
	}

	startgame();
	initializeSharedMemory();
	//WaitForSingleObject(hThreadSharedMemory, INFINITE);


	auxGameForNow();

	gerarNavesInimigas();
	Sleep(3000);
	TrincoOfThreads = CreateMutex(NULL, FALSE, NULL);

	HANDLE hGameThread = CreateThread(
		NULL,
		0,
		gameThread,
		NULL,
		0,
		0);

	if (hGameThread == NULL) {
		_tprintf(TEXT("[ERROR] Creating Shared Memory Thread... (%d)"), GetLastError());
	}
	WaitForSingleObject(hGameThread, INFINITE);

	return 0;
}



#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>
#include "../DLL/DLL.h"
#include "servidor.h"
#include "resource.h"

// CONFIG 
TCHAR szProgName[] = TEXT("SPACE_PROG");
int users[MAXCLIENTS];
HINSTANCE hThisInst;
int windowMode = 0;
HWND hWnd;
HMODULE DLL;
HWND janelaglobal;
// GAME
Game game;
GameInfo gameInfo;

int invadeShipId;
//DLL FUNCTIONS
BOOL(*startgame) ();
BOOL(*writeB)(MSGdata data);
MSGdata(*readB) ();
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
HANDLE eventReaderFromBuffer;
HANDLE eventWriterFromBuffer;
HANDLE canWrite, canRead;
HANDLE PlayersJoin;
// FUNCTIONS
void GameInfoSend() {
	WaitForSingleObject(TrincoOfThreads, INFINITE);
	gameInfo.Id = ALL_PLAYERS;	// For all players see
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
	gameInfo.commandId = REFRESH_GAME;
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
	}
		drawBlock(game.playerShips[i].x, game.playerShips[i].y, 1, BLOCK_EMPTY);
		switch (dir) {

		case RIGHT:
			game.playerShips[i].x++;
			break;
		case LEFT:
			game.playerShips[i].x--;
			break;
		case UP:
			game.playerShips[i].y--;
			break;
		case DOWN:
			game.playerShips[i].y++;
			break;
		}
		drawBlock(game.playerShips[i].x, game.playerShips[i].y, 1, BLOCK_DEFENCESHIP);
	
}

void manageCommands(MSGdata data) {
	switch (data.type) {
	case EXIT:
		_tprintf(TEXT("Goodbye.."));
		break;

		break;
	case JOIN_GAME:
	//	if (game.Created && !game.running) {
			joinGame(data);

	//	}
	//	else {
	//		gameInfo.commandId = ERROR_CANNOT_JOIN_GAME;
	//		gameInfo.Id = dataGame.playerId;
	//		setGameInfo(gameInfo);
	//	}
		break;
	case SCORES:
		break;
	case MOVE:
		moveShip(data.id, data.command);
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
	else if (type == 3) {

		for (int i = x; i < x + 3; i++)
			for (int j = y; j < y + 3; j++)
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
	MSGdata option;
	while (1) {

		option = readB();
		_tprintf(TEXT("\n chegou do gateway: %d\n"), option.type);
	}
}


void initGame(Game dataGame) {
	game.nRows = dataGame.nRows;
	game.nColumns = dataGame.nColumns;
	game.Created = TRUE;
	game.running = FALSE;
	game.nPlayers = 0;
	game.difficult = dataGame.difficult;
	game.lifes = dataGame.lifes;
	game.fireTime = dataGame.fireTime;
	game.powerUpTime = dataGame.powerUpTime;
	game.nInvadesBasic = 2;
	game.nInvadesDodge = 1;
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
	Game dataGame;
	dataGame.difficult = 1;
	dataGame.nColumns = 30;
	dataGame.nRows = 30;
	dataGame.fireTime = 5;
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
void joinDefendShip() {
	for (int i = 0; i < MAXCLIENTS; i++) {
		users[i] = -1;
	}

}

void createDefends() {
	for (int i = 0; i < MAXCLIENTS; i++) {
		if (users[i] != -1) {
			game.playerShips[game.nPlayers] = initShip(users[i]);
			game.nPlayers++;
		}
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

void joinGame(MSGdata data) {
	for (int i = 0; i < MAXCLIENTS; i++) {
		if (users[i] == -1) {
			users[i] = data.id;
			_tprintf(TEXT("Player : %d connected."), data.id);
			break;
		}
	}
}

DefenceShip initShip(int id) {
	DefenceShip defenceShip;
	int x, y;
	do {
		srand((unsigned int)time(NULL));
		x = rand() % ((game.nRows) - 1) + 1;
		srand((unsigned int)time(NULL));
		y = game.nRows - 4;

	} while (hasColision(x, y, 1) == true);
	drawBlock(x, y, 1, BLOCK_DEFENCESHIP);
	defenceShip.id = id;
	defenceShip.alive = TRUE;
	defenceShip.x = x;
	defenceShip.y = y;

	defenceShip.speed = SHIP_SPEED;
	defenceShip.effect = NO_EFFECT;
	return defenceShip;
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
	MSGdata(*getData)();
	MSGdata data;
	eventReader = CreateEvent(NULL, TRUE, FALSE, TEXT("Global\eventReader"));
	eventWriter = CreateEvent(NULL, TRUE, FALSE, TEXT("Global\eventWriter"));

	canWrite = startSemaphore(L"writeSemaphore");
	canRead = startSemaphore(L"readSemaphore");
	eventReaderFromBuffer = CreateEvent(NULL, TRUE, FALSE, TEXT("Global\eventReaderFromBuff"));
	eventWriterFromBuffer = CreateEvent(NULL, TRUE, FALSE, TEXT("Global\eventWriterFromBuff"));;
	//ReleaseSemaphore(canWrite, 1, NULL);
	getData = (MSGdata(*)()) GetProcAddress(DLL, "readBuffer");
	if (getData == NULL) {
		_tprintf(TEXT("[SHM ERROR] Loading getDataSHM function from DLL (%d)\n"), GetLastError());
		return NULL;
	}
	do {

		WaitForSingleObject(canRead, INFINITE);

		//GETDATA IN CORRECT POSITION

		data = getData();
		manageCommands(data);
		_tprintf(TEXT("Lido: %d"), data.type);
		ReleaseSemaphore(canWrite, 1, NULL);
	} while ((data.type != -1));
	return 0;
}

void sendGameInfo(GameInfo gi) {
	setGameInfo(gi);
}
void initializeSharedMemory() {

	BOOL(*createFileMap)();
	WaitForSingleObject(PlayersJoin, INFINITE);

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
		TCHAR text[100];
		_itot_s(10, text, 100, 10);
		_stprintf_s(text, TEXT("%d"), 30);
		SendMessage(GetDlgItem(hWndDlg, IDC_linhas), WM_SETTEXT, 0, (LPARAM)text);
		SendMessage(GetDlgItem(hWndDlg, IDC_colunas), WM_SETTEXT, 0, (LPARAM)text);
		_stprintf_s(text, TEXT("%d"), 1);
		SendMessage(GetDlgItem(hWndDlg, IDC_basicas), WM_SETTEXT, 0, (LPARAM)text);
		SendMessage(GetDlgItem(hWndDlg, IDC_esquivas), WM_SETTEXT, 0, (LPARAM)text);
		_stprintf_s(text, TEXT("%d"), 3);
		SendMessage(GetDlgItem(hWndDlg, IDC_dificuldade), WM_SETTEXT, 0, (LPARAM)text);
		SendMessage(GetDlgItem(hWndDlg, IDC_vidas), WM_SETTEXT, 0, (LPARAM)text);
		_stprintf_s(text, TEXT("%d"), 5);
		SendMessage(GetDlgItem(hWndDlg, IDC_poweruptempo), WM_SETTEXT, 0, (LPARAM)text);
		_stprintf_s(text, TEXT("%d"), 10);
		SendMessage(GetDlgItem(hWndDlg, IDC_velDisparo), WM_SETTEXT, 0, (LPARAM)text);

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
			initGame(game);


			EndDialog(hWndDlg, 0);
			return TRUE;
		}
		break;
	}

	return FALSE;
}



void sendMsg(TCHAR  *t) {
	SendDlgItemMessage(
		hWnd, IDC_LIST, LB_ADDSTRING,
		(WPARAM)0, (LPARAM)(t));
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {
	HDC hdc;
	HANDLE listBox = NULL;
	PAINTSTRUCT ps;
	switch (messg) {
	case WM_CREATE:
		CreateWindow(TEXT("button"), TEXT("StartGame"),
			WS_VISIBLE | WS_CHILD,
			20, 50, 80, 25,
			hWnd, (HMENU)1, NULL, NULL);

		CreateWindow(TEXT("button"), TEXT("Config"),
			WS_VISIBLE | WS_CHILD,
			120, 50, 80, 25,
			hWnd, (HMENU)2, NULL, NULL);
		CreateWindow(TEXT("button"), TEXT("Quit"),
			WS_VISIBLE | WS_CHILD,
			220, 50, 80, 25,
			hWnd, (HMENU)3, NULL, NULL);
		listBox = CreateWindow(L"LISTBOX", L"",
			WS_CHILD | WS_VISIBLE | LBS_NOSEL |
			WS_SIZEBOX | LBS_HASSTRINGS | LBS_STANDARD,
			500, 50, 500, 300,
			hWnd, (HMENU)IDC_LIST, NULL, NULL);
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == 1) {

			gameInfo.commandId = GAME_STARTED;
			sendGameInfo(gameInfo);
			SetEvent(eventReader);

			Sleep(1000);
			auxGameForNow();
			gerarNavesInimigas();
			createDefends();
			TrincoOfThreads = CreateMutex(NULL, FALSE, NULL);

			Sleep(3000);
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


			TCHAR buffer[1024] = TEXT("");
			int i = 0;
			_stprintf_s(buffer, 1024, _T("Jogo Criado com sucesso."));
			sendMsg(buffer);
		}

		if (LOWORD(wParam) == 2) {
			DialogBox(hThisInst, (LPCWSTR)IDD_CONFIG, hWnd, (DLGPROC)DialogConfig);
			TCHAR buffer[1024] = TEXT("");
			int i = 0;
			
			_stprintf_s(buffer, 1024, _T("Configuração efectuada com sucesso!"), i, i);
			sendMsg(buffer);
		}
		if (LOWORD(wParam) == 3) {
			PostQuitMessage(0);
		}
	default:
		// Neste exemplo, para qualquer outra mensagem (p.e. "minimizar","maximizar","restaurar")
		// não é efectuado nenhum processamento, apenas se segue o "default" do Windows
		return(DefWindowProc(hWnd, messg, wParam, lParam));
		break;
	}
	return(0);}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {

	MSG lpMsg; 
	WNDCLASSEX wcApp; 
	wcApp.cbSize = sizeof(WNDCLASSEX); 
	wcApp.hInstance = hInst;
						
					
	wcApp.lpszClassName = szProgName; 
	wcApp.lpfnWndProc = WndProc; 
							
									
	wcApp.style = CS_HREDRAW | CS_VREDRAW;
										
	wcApp.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcApp.hIconSm = LoadIcon(NULL, IDI_INFORMATION);// "hIconSm" = handler do ícon pequeno
	wcApp.hCursor = LoadCursor(NULL, IDC_ARROW); // "hCursor" = handler do cursor (rato)
	wcApp.lpszMenuName = (LPCTSTR)(NULL); // Classe do menu que a janela pode ter
	wcApp.cbClsExtra = 0; 
	wcApp.cbWndExtra = 0; 
	wcApp.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

	if (!RegisterClassEx(&wcApp))
		return(0);

	hWnd = CreateWindow(
		szProgName, 
		TEXT("SpaceGame"),
		WS_OVERLAPPEDWINDOW, 
		CW_USEDEFAULT, 
		CW_USEDEFAULT, 
		CW_USEDEFAULT, 
		CW_USEDEFAULT,
		(HWND)HWND_DESKTOP,
		(HMENU)NULL, 
		(HINSTANCE)hInst, 		
		0); 
	ShowWindow(hWnd, nCmdShow); // "hWnd"= handler da janela, devolvido por
	UpdateWindow(hWnd); // Refrescar a janela (Windows envia à janela uma


	DLL = LoadLibrary(_T("DLL"));
	if (DLL == NULL) {
		_tprintf(_T("[ERROR] Loading DLL!!"));
		return 0;
	}
	else
		_tprintf(_T("DLL lida com sucesso!\n"));

	writeB = (BOOL(*)(MSGdata data))GetProcAddress(DLL, "writeBuffer");
	readB = (MSGdata(*)())GetProcAddress(DLL, "readBuffer");
	startgame = (BOOL(*)())GetProcAddress(DLL, "createGame");
	startMutex = (HANDLE(*)())GetProcAddress(DLL, "startSyncMutex");
	setGameInfo = (void(*)(GameInfo gi))GetProcAddress(DLL, "setInfoSHM");
	startSemaphore = (HANDLE(*)(LPCWSTR semaphoreName))GetProcAddress(DLL, "startSyncSemaphore");
	PlayersJoin = startMutex();

	if (writeB == NULL || readB == NULL || startgame == NULL || setGameInfo == NULL || startSemaphore == NULL) {
		_tprintf(TEXT("[SHM ERROR] Loading functions from DLL (%d)\n"), GetLastError());
		return 0;
	}

	startgame();
	initializeSharedMemory();
	joinDefendShip();
	ReleaseSemaphore(canWrite, 1, NULL);


//	WaitForSingleObject(hGameThread, INFINITE);


	while (GetMessage(&lpMsg, NULL, 0, 0)) {
		TranslateMessage(&lpMsg); // Pré-processamento da mensagem (p.e. obter código
								  // ASCII da tecla premida)
		DispatchMessage(&lpMsg); // Enviar a mensagem traduzida de volta ao Windows, que
								 // aguarda até que a possa reenviar à função de
								 // tratamento da janela, CALLBACK TrataEventos (abaixo)
	}
	InvalidateRect(janelaglobal, NULL, 0);

	// ============================================================================
	// 6. Fim do programa
	// ============================================================================
	return((int)lpMsg.wParam); // Retorna sempre o parâmetro wParam da estrutura lpMsg
}

//int _tmain() {
//
//
//	DLL = LoadLibrary(_T("DLL"));
//	if (DLL == NULL) {
//		_tprintf(_T("[ERROR] Loading DLL!!"));
//		return 0;
//	}
//	else
//		_tprintf(_T("DLL lida com sucesso!\n"));
//
//	writeB = (BOOL(*)(MSGdata data))GetProcAddress(DLL, "writeBuffer");
//	readB = (MSGdata(*)())GetProcAddress(DLL, "readBuffer");
//	startgame = (BOOL(*)())GetProcAddress(DLL, "createGame");
//	startMutex = (HANDLE(*)())GetProcAddress(DLL, "startSyncMutex");
//	setGameInfo = (void(*)(GameInfo gi))GetProcAddress(DLL, "setInfoSHM");
//	startSemaphore = (HANDLE(*)(LPCWSTR semaphoreName))GetProcAddress(DLL, "startSyncSemaphore");
//	PlayersJoin = startMutex();
//
//	if (writeB == NULL || readB == NULL || startgame == NULL || setGameInfo == NULL || startSemaphore == NULL) {
//		_tprintf(TEXT("[SHM ERROR] Loading functions from DLL (%d)\n"), GetLastError());
//		return 0;
//	}
//
//	startgame();
//	initializeSharedMemory();
//	joinDefendShip();
//	ReleaseSemaphore(canWrite, 1, NULL);
//	int opc;
//
//	while (1) {
//
//		_tprintf(TEXT("Insert a option: "));
//		_tscanf_s(TEXT("%d"), &opc);
//		if (opc == 1) {
//			gameInfo.commandId = GAME_STARTED;
//			sendGameInfo(gameInfo);
//			SetEvent(eventReader);
//			break;
//		}
//	}
//	auxGameForNow();
//	gerarNavesInimigas();
//	createDefends();
//	TrincoOfThreads = CreateMutex(NULL, FALSE, NULL);
//
//	Sleep(3000);
//	HANDLE hGameThread = CreateThread(
//		NULL,
//		0,
//		gameThread,
//		NULL,
//		0,
//		0);
//	if (hGameThread == NULL) {
//		_tprintf(TEXT("[ERROR] Creating Shared Memory Thread... (%d)"), GetLastError());
//	}
//	WaitForSingleObject(hGameThread, INFINITE);
//
//	return 0;
//}



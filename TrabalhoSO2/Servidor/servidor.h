#pragma once

#include <windows.h>
#include "../util.h"

typedef struct {
	int id;
	TCHAR user[24];
	int x;
	int	y;
	int	vidas;
	int effect;
	int speed;
	BOOL alive;
}DefenceShip;

typedef struct {
	int id;
	int x, y, vida, tipo;
	BOOL blocked;
	LPDWORD* threadId;
	BOOL alive;
}InvadeShip;


typedef struct Objects {
	int x, y;
	int duration;
	int block;

}Object, *pObject;



typedef struct Game {

	int  **boardGame;
	BOOL Created;
	BOOL running;
	int objectsDuration;
	int nObjects;
	int nPlayers;
	int nInvadesBasic;
	int nInvadesDodge;
	int lifes;
	int difficult;
	int nRows, nColumns;
	TCHAR user[24];
	DefenceShip playerShips[MAXCLIENTS];

	InvadeShip invadeShips[MAXSHIPS];
	Object *object;
	int fireTime;
	int powerUpTime;


} Game, *pGame;

//init
void initializeSharedMemory();
void gerarNavesInimigas();
int initRandomObject();
void StartPlayerShips();
DefenceShip initShip(int id);

//map/game
void moveShip(int id, int dir);
BOOL hasColision(int x, int y, int type);
void drawBlock(int x, int y, int type, int blockType);
void ObjectEffect(int block, int player);

//Interactions
void GameInfoSend();
void manageCommands(data dataGame);
void auxGameForNow();
void joinGame(data dataGame);
void sendGameInfo(GameInfo gi);

//THREADS
DWORD WINAPI gameThread(LPVOID params);
DWORD WINAPI threadPowerUp(LPVOID data);
DWORD WINAPI EnemyFire(LPVOID data);
DWORD WINAPI PowerUp(LPVOID data);
DWORD WINAPI threadbasicas(LPVOID data);
DWORD WINAPI threadesquivas(LPVOID data);
DWORD WINAPI awaitMessages(LPVOID dados);
DWORD WINAPI listenClientSharedMemory(LPVOID params);
#pragma once

#include <windows.h>

// Defines
#define MINIDPLAYER		1000
#define BUFFSIZE		1024
#define TCHARSIZE		30

#define TCHAR_SIZE 	60
#define WHO 		60
#define COMMANDSIZE 60
#define MAXCLIENTS  20
#define MAXSHIPS	20 
#define BUFFERSIZE	20

#define EXIT			100
#define CREATE_GAME		101
#define JOIN_GAME		102
#define SCORES			103
#define START_GAME		104
#define MOVE_SPACE		105
#define MOVE_SPACE2 	106
#define GAME_OVER		107
#define REFRESH_BOARD	108

//errors
#define ERROR_CANNOT_CREATE_GAME	99
#define ERROR_CANNOT_JOIN_GAME		98

//messages
#define SHM_ALL 600


#define LEFT  1
#define RIGHT 2
#define UP    3
#define DOWN  4


#define BLOCK_EMPTY				0
#define BLOCK_ICE				1
#define BLOCK_BATTERY			2
#define BLOCK_MORE				3
#define BLOCK_LIFE				4
#define BLOCK_ALCOOL			5
#define BLOCK_SHIELD			6
#define BLOCK_WALL				7
#define BLOCK_DEFENCESHIP		8
#define BLOCK_ENEMYSHIP			9

#define NO_EFFECT		0
#define EFFECT_ICE		1
#define EFFECT_BATTERY	2
#define EFFECT_MORE		3
#define EFFECT_ALCOOL	4
#define EFFECT_SHIELD	5

#define SHIP_SPEED 100

#define SHIP_BASIC 0
#define SHIP_DODGE 1

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


typedef struct GameInfo {
	int Id;											
	int commandId;						
	int **boardGame;			
	int nRows, nColumns;
} GameInfo, *pGameInfo;


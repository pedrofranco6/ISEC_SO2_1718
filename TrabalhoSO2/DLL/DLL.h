#pragma once
#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <windows.h>

#ifdef DLL_EXPORTS
#define DLL_IMP_API __declspec(dllexport)
#else
#define DLL_IMP_API __declspec(dllimport)
#endif

typedef struct {
	char nome[10];
	int id, duracao, raridade;
}Powerup;

typedef struct {
	int x, y;
}tiros;

typedef struct {
	int x, y, vidas;
}NaveDefensora;

typedef struct {
	int x, y, vida, tipo;
	LPDWORD* threadId;
}NaveInvasora;

typedef struct {
	int dimX, dimY;
	NaveDefensora navesDefensoras[2];
	NaveInvasora navesInimigasBasicas[2];
	NaveInvasora navesInimigasEsquiva[2];
}Jogo, *pJogo;


HANDLE mapFile;
pJogo viewMapFile;
TCHAR mapName[] = TEXT("MapFile");
HANDLE mutex;

// Cliente.cpp : Defines the entry point for the console application.
//
#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <time.h>
#include "resource.h"
#include "../Servidor/servidor.h"
Game game;
HINSTANCE hThisInst;
int windowMode = 0;
HWND hWnd;

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
			GetDlgItemText(hWndDlg, IDC_linhas, aux, 20 );
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

int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow
) {
	hThisInst = hInstance;
	hWnd = NULL;
	windowMode = nCmdShow;
	DialogBox(hThisInst, (LPCWSTR)IDD_CONFIG, hWnd, (DLGPROC)DialogConfig);
	return false;
}




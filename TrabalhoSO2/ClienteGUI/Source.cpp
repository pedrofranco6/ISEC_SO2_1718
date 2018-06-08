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
HWND hWnd; // hWnd � o handler da janela, gerado mais abaixo por CreateWindow()
HWND janelaglobal;

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

/* ===================================================== */
/* Programa base (esqueleto) para aplica��es Windows */
/* ===================================================== */
// Cria uma janela de nome "Janela Principal" e pinta fundo de branco
// Modelo para programas Windows:
// Composto por 2 fun��es:
// WinMain() = Ponto de entrada dos programas windows
// 1) Define, cria e mostra a janela
// 2) Loop de recep��o de mensagens provenientes do Windows
// TrataEventos()= Processamentos da janela (pode ter outro nome)
// 1) � chamada pelo Windows (callback)
// 2) Executa c�digo em fun��o da mensagem recebida
LRESULT CALLBACK TrataEventos(HWND, UINT, WPARAM, LPARAM);
// Nome da classe da janela (para programas de uma s� janela, normalmente este nome �
// igual ao do pr�prio programa) "szprogName" � usado mais abaixo na defini��o das
// propriedades do objecto janela
TCHAR szProgName[] = TEXT("SPACE_PROG");
// ============================================================================
// FUN��O DE IN�CIO DO PROGRAMA: WinMain()
// ============================================================================
// Em Windows, o programa come�a sempre a sua execu��o na fun��o WinMain()que desempenha
// o papel da fun��o main() do C em modo consola WINAPI indica o "tipo da fun��o" (WINAPI
// para todas as declaradas nos headers do Windows e CALLBACK para as fun��es de
// processamento da janela)
// Par�metros:
// hInst: Gerado pelo Windows, � o handle (n�mero) da inst�ncia deste programa
// hPrevInst: Gerado pelo Windows, � sempre NULL para o NT (era usado no Windows 3.1)
// lpCmdLine: Gerado pelo Windows, � um ponteiro para uma string terminada por 0
// destinada a conter par�metros para o programa
// nCmdShow: Par�metro que especifica o modo de exibi��o da janela (usado em
// ShowWindow()
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {
	
	MSG lpMsg; // MSG � uma estrutura definida no Windows para as mensagens
	WNDCLASSEX wcApp; // WNDCLASSEX � uma estrutura cujos membros servem para
					  // definir as caracter�sticas da classe da janela
					  // ============================================================================
					  // 1. Defini��o das caracter�sticas da janela "wcApp"
					  // (Valores dos elementos da estrutura "wcApp" do tipo WNDCLASSEX)
					  // ============================================================================
	wcApp.cbSize = sizeof(WNDCLASSEX); // Tamanho da estrutura WNDCLASSEX
	wcApp.hInstance = hInst; // Inst�ncia da janela actualmente exibida
							 // ("hInst" � par�metro de WinMain e vem
							 // inicializada da�)
	wcApp.lpszClassName = szProgName; // Nome da janela (neste caso = nome do programa)
	wcApp.lpfnWndProc = TrataEventos; // Endere�o da fun��o de processamento da janela
									  // ("TrataEventos" foi declarada no in�cio e
									  // encontra-se mais abaixo)
	wcApp.style = CS_HREDRAW | CS_VREDRAW;// Estilo da janela: Fazer o redraw se for
										  // modificada horizontal ou verticalmente
	wcApp.hIcon = LoadIcon(NULL, IDI_APPLICATION);// "hIcon" = handler do �con normal
												  //"NULL" = Icon definido no Windows
												  // "IDI_AP..." �cone "aplica��o"
	wcApp.hIconSm = LoadIcon(NULL, IDI_INFORMATION);// "hIconSm" = handler do �con pequeno
													//"NULL" = Icon definido no Windows
													// "IDI_INF..." �con de informa��o
	wcApp.hCursor = LoadCursor(NULL, IDC_ARROW); // "hCursor" = handler do cursor (rato)
												 // "NULL" = Forma definida no Windows
												 // "IDC_ARROW" Aspecto "seta"
	wcApp.lpszMenuName = (LPCTSTR)(IDD_MENU); // Classe do menu que a janela pode ter
							   // (NULL = n�o tem menu)
	wcApp.cbClsExtra = 0; // Livre, para uso particular
	wcApp.cbWndExtra = 0; // Livre, para uso particular
	wcApp.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	// "hbrBackground" = handler para "brush" de pintura do fundo da janela. Devolvido por
	// "GetStockObject".Neste caso o fundo ser� branco
	// ============================================================================
	// 2. Registar a classe "wcApp" no Windows
	// ============================================================================
	if (!RegisterClassEx(&wcApp))
		return(0);
	// ============================================================================
	// 3. Criar a janela
	// ============================================================================
	hWnd = CreateWindow(
		szProgName, // Nome da janela (programa) definido acima
		TEXT("SpaceGame"),// Texto que figura na barra do t�tulo
		WS_OVERLAPPEDWINDOW, // Estilo da janela (WS_OVERLAPPED= normal)
		CW_USEDEFAULT, // Posi��o x pixels (default=� direita da �ltima)
		CW_USEDEFAULT, // Posi��o y pixels (default=abaixo da �ltima)
		CW_USEDEFAULT, // Largura da janela (em pixels)
		CW_USEDEFAULT, // Altura da janela (em pixels)
		(HWND)HWND_DESKTOP, // handle da janela pai (se se criar uma a partir de
							// outra) ou HWND_DESKTOP se a janela for a primeira,
							// criada a partir do "desktop"
		(HMENU)NULL, // handle do menu da janela (se tiver menu)
		(HINSTANCE)hInst, // handle da inst�ncia do programa actual ("hInst" �
						  // passado num dos par�metros de WinMain()
		0); // N�o h� par�metros adicionais para a janela
			// ============================================================================
			// 4. Mostrar a janela
			// ============================================================================
	ShowWindow(hWnd, nCmdShow); // "hWnd"= handler da janela, devolvido por
								// "CreateWindow"; "nCmdShow"= modo de exibi��o (p.e.
								// normal/modal); � passado como par�metro de WinMain()
	UpdateWindow(hWnd); // Refrescar a janela (Windows envia � janela uma
						// mensagem para pintar, mostrar dados, (refrescar)�
						// ============================================================================
						// 5. Loop de Mensagens
						// ============================================================================
						// O Windows envia mensagens �s janelas (programas). Estas mensagens ficam numa fila de
						// espera at� que GetMessage(...) possa ler "a mensagem seguinte"
						// Par�metros de "getMessage":
						// 1)"&lpMsg"=Endere�o de uma estrutura do tipo MSG ("MSG lpMsg" ja foi declarada no
						// in�cio de WinMain()):
						// HWND hwnd handler da janela a que se destina a mensagem
						// UINT message Identificador da mensagem
						// WPARAM wParam Par�metro, p.e. c�digo da tecla premida
						// LPARAM lParam Par�metro, p.e. se ALT tamb�m estava premida
						// DWORD time Hora a que a mensagem foi enviada pelo Windows
						// POINT pt Localiza��o do mouse (x, y)
						// 2)handle da window para a qual se pretendem receber mensagens (=NULL se se pretendem
						// receber as mensagens para todas as janelas pertencentes � thread actual)
						// 3)C�digo limite inferior das mensagens que se pretendem receber
						// 4)C�digo limite superior das mensagens que se pretendem receber
						// NOTA: GetMessage() devolve 0 quando for recebida a mensagem de fecho da janela,
						// terminando ent�o o loop de recep��o de mensagens, e o programa
	while (GetMessage(&lpMsg, NULL, 0, 0)) {
		TranslateMessage(&lpMsg); // Pr�-processamento da mensagem (p.e. obter c�digo
								  // ASCII da tecla premida)
		DispatchMessage(&lpMsg); // Enviar a mensagem traduzida de volta ao Windows, que
								 // aguarda at� que a possa reenviar � fun��o de
								 // tratamento da janela, CALLBACK TrataEventos (abaixo)
	}
	InvalidateRect(janelaglobal, NULL, 0);
	// ============================================================================
	// 6. Fim do programa
	// ============================================================================
	return((int)lpMsg.wParam); // Retorna sempre o par�metro wParam da estrutura lpMsg
}
// ============================================================================
// FUN��O DE PROCESSAMENTO DA JANELA
// Esta fun��o pode ter um nome qualquer: Apenas � neces�rio que na inicializa��o da
// estrutura "wcApp", feita no in�cio de WinMain(), se identifique essa fun��o. Neste
// caso "wcApp.lpfnWndProc = WndProc"
//
// WndProc recebe as mensagens enviadas pelo Windows (depois de lidas e pr�-processadas
// no loop "while" da fun��o WinMain()
//
// Par�metros:
// hWnd O handler da janela, obtido no CreateWindow()
// messg Ponteiro para a estrutura mensagem (ver estrutura em 5. Loop...
// wParam O par�metro wParam da estrutura messg (a mensagem)
// lParam O par�metro lParam desta mesma estrutura
//
// NOTA:Estes par�metros est�o aqui acess�veis o que simplifica o acesso aos seus valores
//
// A fun��o EndProc � sempre do tipo "switch..." com "cases" que descriminam a mensagem
// recebida e a tratar. Estas mensagens s�o identificadas por constantes (p.e.
// WM_DESTROY, WM_CHAR, WM_KEYDOWN, WM_PAINT...) definidas em windows.h
//============================================================================

void sendMsg(TCHAR  *t) {
	SendDlgItemMessage(
		hWnd, IDC_LIST, LB_ADDSTRING,
		(WPARAM)0, (LPARAM)(t));
}
LRESULT CALLBACK TrataEventos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {
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
			WS_CHILD | WS_VISIBLE | LBS_STANDARD,
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
			TCHAR buffer[1024] = TEXT("");
			int i = 0;
			Beep(40, 50);
			_stprintf_s(buffer,1024, _T("%d %d"), i, i);
			sendMsg(buffer);
		}

		if (LOWORD(wParam) == 2) {
			DialogBox(hThisInst, (LPCWSTR)IDD_CONFIG, hWnd, (DLGPROC)DialogConfig);
		}
		if (LOWORD(wParam) == 3) {
			PostQuitMessage(0);
		}
	default:
		// Neste exemplo, para qualquer outra mensagem (p.e. "minimizar","maximizar","restaurar")
		// n�o � efectuado nenhum processamento, apenas se segue o "default" do Windows
		return(DefWindowProc(hWnd, messg, wParam, lParam));
		break;
	}
	return(0);}

/**int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow
) {
	hThisInst = hInstance;
	hWnd = NULL;
	windowMode = nCmdShow;
	DialogBox(hThisInst, (LPCWSTR)IDD_CONFIG, hWnd, (DLGPROC)DialogConfig);

	while (1);
	return false;
} **/




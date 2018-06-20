#include <process.h>
#include <windows.h> 
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <time.h>
#include "resource.h"
#include "../util.h"
#include "Cliente.h"
#include "../DLL/DLL.h"
#define GATEWAY_PIPE_NAME TEXT("\\\\.\\pipe\\gatewayPipe")
#define BUFSIZE 512
#define CONNECTING_STATE 0
#define READING_STATE 1 
#define WRITING_STATE 2

void sendCommand(MSGdata msg);
void startMainWindow();
LRESULT CALLBACK StartWindow(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MainWindow(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
void RefreshMap(GameInfo gameInfo);
//Game
HINSTANCE hThisInst;
int windowMode = 0;
HWND hWnd; // hWnd é o handler da janela, gerado mais abaixo por CreateWindow()
HWND janelaglobal;
TCHAR szProgName[] = TEXT("SPACE_CLIENT");
DWORD  cbToWrite, cbWritten, dwMode, cbRet, dwThreadId = 0;

HANDLE hEventWrite;
HDC hdc;
HDC memdc;

HBITMAP hbitEmpty;
HBITMAP hbitWall;


HBITMAP hbitEnemyShip;
HBITMAP hbitEnemyShot;
HBITMAP hbitDefenceShip;
HBITMAP hbitDefenceShot;
HBITMAP hbitIce;
HBITMAP hbitBattery;
HBITMAP hbitMore;
HBITMAP hbitLife;
HBITMAP hbitAlcool;
HBITMAP hbitShield;

HBRUSH hbrush;
INT_PTR InitialDialog;
HANDLE hPipe;
OVERLAPPED oOverlap;
GameInfo gi;
DWORD WINAPI ThreadReadGateway(void* data);

LRESULT CALLBACK TrataEventos(HWND, UINT, WPARAM, LPARAM);

//https://msdn.microsoft.com/en-us/library/windows/desktop/aa365592(v=vs.85).aspx

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {

	MSG lpMsg; // MSG é uma estrutura definida no Windows para as mensagens
	WNDCLASSEX wcApp; // WNDCLASSEX é uma estrutura cujos membros servem para
	wcApp.cbSize = sizeof(WNDCLASSEX); // Tamanho da estrutura WNDCLASSEX
	wcApp.hInstance = hInst; // Instância da janela actualmente exibida
	wcApp.lpszClassName = szProgName;
	wcApp.lpfnWndProc = TrataEventos;
	BOOL fSuccess = false;
	wcApp.style = CS_HREDRAW | CS_VREDRAW;

	wcApp.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcApp.hIconSm = LoadIcon(NULL, IDI_INFORMATION);
	wcApp.hCursor = LoadCursor(NULL, IDC_ARROW); // "hCursor" = handler do cursor (rato)
	wcApp.lpszMenuName = (LPCTSTR)(NULL); // Classe do menu que a janela pode ter
	wcApp.cbClsExtra = 0; // Livre, para uso particular
	wcApp.cbWndExtra = 0; // Livre, para uso particular
	wcApp.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

	if (!RegisterClassEx(&wcApp))
		return(0);
	// ============================================================================
	// 3. Criar a janela
	// ============================================================================
	hWnd = CreateWindow(
		szProgName, // Nome da janela (programa) definido acima
		TEXT("Space Client"),// Texto que figura na barra do título
		WS_OVERLAPPEDWINDOW, // Estilo da janela (WS_OVERLAPPED= normal)
		CW_USEDEFAULT, // Posição x pixels (default=à direita da última)
		CW_USEDEFAULT, // Posição y pixels (default=abaixo da última)
		CW_USEDEFAULT, // Largura da janela (em pixels)
		CW_USEDEFAULT, // Altura da janela (em pixels)
		(HWND)HWND_DESKTOP, // handle da janela pai (se se criar uma a partir de
							// outra) ou HWND_DESKTOP se a janela for a primeira,
							// criada a partir do "desktop"
		(HMENU)NULL, // handle do menu da janela (se tiver menu)
		(HINSTANCE)hInst, // handle da instância do programa actual ("hInst" é
						  // passado num dos parâmetros de WinMain()
		0); // Não há parâmetros adicionais para a janela
			// ============================================================================
			// 4. Mostrar a janela
			// ============================================================================
	windowMode = nCmdShow;

	ShowWindow(hWnd, nCmdShow); // "hWnd"= handler da janela, devolvido por
								  // "CreateWindow"; "nCmdShow"= modo de exibição (p.e.
								  // normal/modal); é passado como parâmetro de WinMain()
	UpdateWindow(hWnd); // Refrescar a janela (Windows envia à janela uma

	InvalidateRect(janelaglobal, NULL, 0);

	//============================================================================


	BOOL ret;
	TCHAR  message[BUFSIZE];
	TCHAR  chBuf[BUFSIZE];


	OVERLAPPED oOverlap; //S
	hPipe = CreateFile(GATEWAY_PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0 | FILE_FLAG_OVERLAPPED, NULL);

	if (hPipe == INVALID_HANDLE_VALUE) {
		_tprintf(TEXT("[ERRO] Nao foi possivel abrir o named pipe => %d\n"), GetLastError());
		return -1;
	}

	if (!WaitNamedPipe(GATEWAY_PIPE_NAME, 20000)) {
		printf("[ERRO] Nao foi possivel abrir o named pipe passados 20 segundos\n");
		return -1;
	}

	dwMode = PIPE_READMODE_MESSAGE;
	fSuccess = SetNamedPipeHandleState(hPipe, &dwMode, NULL, NULL);

	if (!fSuccess) {
		_tprintf(TEXT("[ERRO] SetNamedPipeHandleState falhou => %d\n"), GetLastError());
		return -1;
	}

	HANDLE tPipe = CreateThread(NULL, 0, ThreadReadGateway, NULL, 0, &dwThreadId);
	if (tPipe == NULL) {
		printf("[ERRO] Criar thread para os pipes => %d\n", GetLastError());
		return -1;
	}

	hEventWrite = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (hEventWrite == NULL) {
		_tprintf(TEXT("[ERRO] Criar evento de leitura... (%d)\n"), GetLastError());
		return -1;
	}
	InitialDialog = DialogBox(hThisInst, (LPCWSTR)IDD_STARTDIALOG, hWnd, (DLGPROC)StartWindow);
	while ((ret = GetMessage(&lpMsg, NULL, 0, 0)) != 0) {
		if (ret != -1) {
			TranslateMessage(&lpMsg);	// Pré-processamento da mensagem (p.e. obter código 
										// ASCII da tecla premida)
			DispatchMessage(&lpMsg);	// Enviar a mensagem traduzida de volta ao Windows, que
										// aguarda até que a possa reenviar à função de 
										// tratamento da janela, CALLBACK TrataEventos (abaixo)
		}
	}

	// ============================================================================
	// 6. Fim do programa
	// ============================================================================

	// Retorna sempre o parâmetro wParam da estrutura lpMsg



	printf("\n<Fim da conversa, carrega ENTER para terminar a coneccao e sair>\n");
	_getch();

	TerminateThread(tPipe, NULL);
	CloseHandle(tPipe);
	CloseHandle(hPipe);

	return((int)lpMsg.wParam);
}
LRESULT CALLBACK StartWindow(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {

	switch (messg) {

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_JOINGAME:
			MSGdata msgToBuffer;
			msgToBuffer.type = JOIN_GAME;
			sendCommand(msgToBuffer);
			EndDialog(hWnd, 0);
			break;

		case IDC_QUIT:
			PostQuitMessage(0);
			break;
		}


	case WM_DESTROY:
		EndDialog(hWnd, 0);
		return 0;
	
	}
	return (0);
}


LRESULT CALLBACK TrataEventos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {

//	HDC device, auxDC;
	PAINTSTRUCT pt;
	MSGdata data;
	switch (messg) {
	case WM_CREATE:

		hdc = GetDC(hWnd);
		memdc = CreateCompatibleDC(hdc);

		hbitEmpty = CreateCompatibleBitmap(hdc, 800, 650);
		SelectObject(memdc, hbitEmpty);
		hbitEmpty = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_EMPTY));

		hbitWall = CreateCompatibleBitmap(hdc, 800, 650);
		SelectObject(memdc, hbitWall);
		hbitWall = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_WALL));

		hbitEnemyShip = CreateCompatibleBitmap(hdc, 800, 650);
		SelectObject(memdc, hbitEnemyShip);
		hbitEnemyShip = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_ENEMYSHIP));

		hbitEnemyShot = CreateCompatibleBitmap(hdc, 800, 650);
		SelectObject(memdc, hbitEnemyShot);
		hbitEnemyShot = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_ENEMYSHOT));

		hbitDefenceShip = CreateCompatibleBitmap(hdc, 800, 650);
		SelectObject(memdc, hbitDefenceShip);
		hbitDefenceShip = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_FRIENDLYSHIP));

		hbitDefenceShot = CreateCompatibleBitmap(hdc, 800, 650);
		SelectObject(memdc, hbitDefenceShot);
		hbitDefenceShot = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_FRIENDLYSHOT));

		hbitAlcool = CreateCompatibleBitmap(hdc, 800, 650);
		SelectObject(memdc, hbitAlcool);
		hbitAlcool = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_ALCOOL));

		hbitBattery = CreateCompatibleBitmap(hdc, 800, 650);
		SelectObject(memdc, hbitBattery);
		hbitBattery = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BATTERY));

		hbitIce = CreateCompatibleBitmap(hdc, 800, 650);
		SelectObject(memdc, hbitIce);
		hbitIce = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_ICE));

		hbitLife = CreateCompatibleBitmap(hdc, 800, 650);
		SelectObject(memdc, hbitLife);
		hbitLife = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_LIFE));

		hbitShield = CreateCompatibleBitmap(hdc, 800, 650);
		SelectObject(memdc, hbitShield);
		hbitShield = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_SHIELD));

		hbitMore = CreateCompatibleBitmap(hdc, 800, 650);
		SelectObject(memdc, hbitMore);
		hbitMore = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_MORE));

		hbrush = (HBRUSH) GetStockObject(WHITE_BRUSH);
		SelectObject(memdc, hbrush);
		PatBlt(memdc, 0, 0, 800, 650, PATCOPY);

		ReleaseDC(hWnd, hdc);

		break;

	case WM_PAINT:
		PAINTSTRUCT ps;
		hdc = BeginPaint(hWnd, &ps);
		//device = BeginPaint(hWnd, &ps);
		//auxDC = CreateCompatibleDC(device);
		//SelectObject(auxDC, hbitEmpty);
		//for (int i = 0; i < 10; i++) {
		//	for (int j = 0; j < 10; j++) {
		//		BitBlt(device, 5 + 40 * j, 5 + 40 * i, 40, 40, auxDC, 0, 0, SRCCOPY);
		//	}
		//}
		BitBlt(hdc, 0, 0, 800, 650, memdc, 0, 0, SRCCOPY);
		EndPaint(hWnd, &ps);
		break;

		// SetStretchBltModeauxDC, BLACKONWHITE);
		// StretchBlt(auxDC, x, y, 100, 60, hdcNave, 0, 0, bmNave.bmWidth, bmNave.bmHeight, SRCCOPY);
		// EX1...


	case WM_DESTROY: // Destruir a janela e terminar o programa
					 // "PostQuitMessage(Exit Status)"
		PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
	{
		
		data.type = MOVE;
		switch (wParam) {

		case VK_LEFT:
			data.command = LEFT;
			break;

		case VK_RIGHT:
			data.command = RIGHT;
			break;

		case VK_UP:
			data.command = UP;
			break;

		case VK_DOWN:
			data.command = DOWN;
			break;
		default:
			if(wParam == TEXT('C'))
			data.type = JOIN_GAME;
		}
		sendCommand(data);
		break;

	}
	default:
		// Neste exemplo, para qualquer outra mensagem (p.e. "minimizar","maximizar","restaurar")
		// não é efectuado nenhum processamento, apenas se segue o "default" do Windows


		return(DefWindowProc(hWnd, messg, wParam, lParam));
		break;
	}
		
	return(0);
}


void sendCommand(MSGdata msg) {
	BOOL   fSuccess = FALSE;
	cbToWrite = sizeof(MSGdata);

	ZeroMemory(&oOverlap, sizeof(oOverlap));
	ResetEvent(hEventWrite);
	oOverlap.hEvent = hEventWrite;

	fSuccess = WriteFile(hPipe, &msg, sizeof(MSGdata), &cbWritten, &oOverlap);

	WaitForSingleObject(hEventWrite, INFINITE);

	GetOverlappedResult(hPipe, &oOverlap, &cbWritten, FALSE);
	if (cbWritten < cbToWrite) {
		_tprintf(TEXT("[ERRO] Escrever no ficheiro do servidor => %d\n"), GetLastError());
		return;
	}

	if (!fSuccess) {
		_tprintf(TEXT("[ERRO] Funcao WriteFile falhou... (%d)\n", GetLastError()));
		return;
	}

}

DWORD WINAPI ThreadReadGateway(void* data) {

	HANDLE hEventRead;

	GameInfo  message;
	DWORD  msgSize = sizeof(GameInfo);
	BOOL   fSuccess = FALSE;
	DWORD  cbRead, cbRet;

	hEventRead = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (hEventRead == NULL) {
		_tprintf(TEXT("[ERRO] Evento para ler falhou... => %d\n"));
		return -1;
	}
	_tprintf(TEXT("-\nThread Reader - pronta para receber mensagens...\n[WRITE] Frase: "));

	while (1) {

		ZeroMemory(&oOverlap, sizeof(oOverlap));

		ResetEvent(hEventRead);
		oOverlap.hEvent = hEventRead;

		fSuccess = ReadFile(hPipe, &message, msgSize, &cbRead, &oOverlap);

		WaitForSingleObject(hEventRead, INFINITE);


		if (!fSuccess || cbRead < msgSize) {
			fSuccess = GetOverlappedResult(hPipe, &oOverlap, &cbRead, FALSE);
			if (!fSuccess) {
				_tprintf(TEXT("[ERRO] Ler do ficheiro do servidor... => %d\n"), GetLastError());
			}
		}
		if (message.commandId == REFRESH_GAME) {
			RefreshMap(message);

		}
		if (message.commandId == GAME_STARTED) {
			SendMessage(hWnd, WM_COMMAND, IDC_QUIT, NULL);
			startMainWindow();
			RefreshMap(message);
		}
		if(message.commandId == ERROR_CANNOT_JOIN_GAME) {
			SendDlgItemMessage(
				hWnd, IDC_text, LB_ADDSTRING,
				(WPARAM)0, (LPARAM)(TEXT("Game Already Started.")));
		}
		else if(message.commandId == WAIT_GAME) {
			startMainWindow();
			SendDlgItemMessage(
				hWnd, IDC_text, LB_ADDSTRING,
				(WPARAM)0, (LPARAM)(TEXT("Waiting for server to start game.")));
		}
	}

	return 0;
}

//----------------------------------------------------
// DRAW BITMAPS
//----------------------------------------------------


LRESULT CALLBACK MainWindow(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {
	MSGdata data;
	HDC auxmemdc;					// handler para Device Context auxiliar em mem�ria
									// que vai conter o bitmap 
	PAINTSTRUCT ps;				// Ponteiro para estrutura de WM_PAINT

								// Double Buffer
	HDC hdcDB;
	HBITMAP hdDB;

	switch (messg) {
	case WM_CREATE:
	{
		//SetRect(&rectangle, 1, 1, 20, 20);

		HBITMAP hbitEmpty;
		HBITMAP hbitIce;
		HBITMAP hbitBattery;
		HBITMAP hbitEnemyShip;
		HBITMAP hbitDefenceShip;
		HBITMAP hbitWall;
		HBITMAP hbitMore;
		HBITMAP hbitLife;
		HBITMAP hbitAlcool;
		HBITMAP hbitShield;

		hdc = GetDC(hWnd);
		memdc = CreateCompatibleDC(hdc);
		hbitEmpty = CreateCompatibleBitmap(hdc, 800, 650);
		SelectObject(memdc, hbitEmpty);

		hbitIce = CreateCompatibleBitmap(hdc, 800, 650);
		SelectObject(memdc, hbitIce);

		hbitBattery = CreateCompatibleBitmap(hdc, 800, 650);
		SelectObject(memdc, hbitBattery);

		hbitEnemyShip = CreateCompatibleBitmap(hdc, 800, 650);
		SelectObject(memdc, hbitEnemyShip);

		hbitDefenceShip = CreateCompatibleBitmap(hdc, 800, 650);
		SelectObject(memdc, hbitDefenceShip);

		hbitWall = CreateCompatibleBitmap(hdc, 800, 650);
		SelectObject(memdc, hbitWall);

		hbitMore = CreateCompatibleBitmap(hdc, 800, 650);
		SelectObject(memdc, hbitMore);

		hbitLife = CreateCompatibleBitmap(hdc, 800, 650);
		SelectObject(memdc, hbitLife);

		hbitAlcool = CreateCompatibleBitmap(hdc, 800, 650);
		SelectObject(memdc, hbitAlcool);

		hbitShield = CreateCompatibleBitmap(hdc, 800, 650);
		SelectObject(memdc, hbitShield);

		PatBlt(memdc, 0, 0, 800, 650, PATCOPY);

		ReleaseDC(hWnd, hdc);

		hbitEmpty = LoadBitmap(hThisInst, MAKEINTRESOURCE(IDB_EMPTY));
		hbitWall = LoadBitmap(hThisInst, MAKEINTRESOURCE(IDB_WALL));
		break;
	}

	case WM_KEYUP:
	{
		break;
	}

	case WM_COMMAND:
	{
		switch (LOWORD(wParam)) {
			//	case ID_FILE_EXIT:
					//closeEverything();
			break;
		}

	case WM_DESTROY:
	{
		// Destruir a janela e terminar o programa 
		// "PostQuitMessage(Exit Status)"		
	//	closeEverything();
		break;
	}

	case WM_PAINT:
	{

		hdc = BeginPaint(hWnd, &ps);

		BitBlt(hdc, 0, 0, 800, 650, memdc, 0, 0, SRCCOPY);

		EndPaint(hWnd, &ps);
		break;
	}

	case WM_SIZE:
	{
		break;
	}

	case WM_ERASEBKGND:
		break;

	default:
	{
		// Neste exemplo, para qualquer outra mensagem (p.e. "minimizar","maximizar","restaurar") // não é efectuado nenhum processamento, apenas se segue o "default" do Windows			
		return(DefWindowProc(hWnd, messg, wParam, lParam));
	}

	}

	return(0);
	}
}



void bitmap(int left, int right, int top, int bot, HBITMAP hbit) {
	hdc = GetDC(hWnd);
	HDC auxmemdc = CreateCompatibleDC(hdc);

	SelectObject(auxmemdc, hbit);
	//BitBlt(hdc, left, top, right, bot, auxmemdc, 0, 0, SRCCOPY);
	ReleaseDC(hWnd, hdc);
	BitBlt(memdc, left, top, right, bot, auxmemdc, 0, 0, SRCCOPY);
	DeleteDC(auxmemdc);

}


void RefreshMap(GameInfo gameInfo) {

			hdc = GetDC(hWnd);
			int x = 0, y = 0;
			for (int l = 0; l < gameInfo.nRows; l++) {
				for (int c = 0; c < gameInfo.nColumns; c++) {

					switch (gameInfo.boardGame[c][l]) {
					case BLOCK_EMPTY:
						bitmap(x, x + 20, y, y + 20, hbitEmpty);
						break;
					case BLOCK_WALL:
						bitmap(x, x + 20, y, y + 20, hbitWall);
						break;
					case BLOCK_ENEMYSHIP:
						bitmap(x, x + 20, y, y + 20, hbitEnemyShip);

						break;
					case BLOCK_ENEMYSHOT:
						bitmap(x, x + 20, y, y + 20, hbitEnemyShot);
						break;
					case BLOCK_DEFENCESHIP:
						bitmap(x, x + 20, y, y + 20, hbitDefenceShip);
						break;
					case BLOCK_FRIENDLYSHOT:
						bitmap(x, x + 20, y, y + 20, hbitDefenceShot);
						break;
					case BLOCK_SHIELD:
						bitmap(x, x + 20, y, y + 20, hbitShield);
						break;
					case BLOCK_ALCOOL:
						bitmap(x, x + 20, y, y + 20, hbitAlcool);
						break;
					case BLOCK_BATTERY:
						bitmap(x, x + 20, y, y + 20, hbitBattery);
						break;
					case BLOCK_MORE:
						bitmap(x, x + 20, y, y + 20, hbitMore);
						break;
					case BLOCK_ICE:
						bitmap(x, x + 20, y, y + 20, hbitIce);
						break;
					case BLOCK_LIFE:
						bitmap(x, x + 20, y, y + 20, hbitLife);
						break;
					}

					x += 20;
				}
				x = 0;
				y += 20;
			}
			ReleaseDC(hWnd, hdc);
			InvalidateRect(NULL, NULL, TRUE);


	
}

void startMainWindow() {
	ShowWindow(hWnd, windowMode); // "hWnd"= handler da janela, devolvido por
								// "CreateWindow"; "nCmdShow"= modo de exibição (p.e.
								// normal/modal); é passado como parâmetro de WinMain()
	UpdateWindow(hWnd); // Refrescar a janela (Windows envia à janela uma

	InvalidateRect(janelaglobal, NULL, 0);

	return;
}

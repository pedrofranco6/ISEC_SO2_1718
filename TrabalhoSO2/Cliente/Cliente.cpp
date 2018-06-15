#include <process.h>
#include <windows.h> 
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <time.h>

#include "../util.h"

#define GATEWAY_PIPE_NAME TEXT("\\\\.\\pipe\\gatewayPipe")
#define BUFSIZE 512
#define CONNECTING_STATE 0
#define READING_STATE 1 
#define WRITING_STATE 2

HANDLE hPipe;
OVERLAPPED oOverlap;
GameInfo gi;
DWORD WINAPI ThreadReadGateway(void* data);

//https://msdn.microsoft.com/en-us/library/windows/desktop/aa365592(v=vs.85).aspx

int _tmain() {

	HANDLE hEventWrite;

	TCHAR  message[BUFSIZE];
	TCHAR  chBuf[BUFSIZE];
	BOOL   fSuccess = FALSE;
	DWORD  cbToWrite, cbWritten, dwMode, cbRet, dwThreadId = 0;
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

	while (1) {
		_tprintf(TEXT("[WRITE] Frase: "));
		_fgetts(message, BUFSIZE, stdin);
		message[_tcslen(message) - 1] = '\0';
		cbToWrite = (lstrlen(message) + 1) * sizeof(TCHAR);

		if (_tcscmp(TEXT("fim"), message) == 0)
			break;

		ZeroMemory(&oOverlap, sizeof(oOverlap));
		ResetEvent(hEventWrite);
		oOverlap.hEvent = hEventWrite;

		fSuccess = WriteFile(hPipe, &message, cbToWrite, &cbWritten, &oOverlap);

		WaitForSingleObject(hEventWrite, INFINITE);

		GetOverlappedResult(hPipe, &oOverlap, &cbWritten, FALSE);
		if (cbWritten < cbToWrite) {
			_tprintf(TEXT("[ERRO] Escrever no ficheiro do servidor => %d\n"), GetLastError());
			return -1;
		}

		if (!fSuccess) {
			_tprintf(TEXT("[ERRO] Funcao WriteFile falhou... (%d)\n", GetLastError()));
			return -1;
		}

		_tprintf(TEXT("[INFO] Mensagem enviada com sucesso\n"));
	}

	printf("\n<Fim da conversa, carrega ENTER para terminar a coneccao e sair>\n");
	_getch();

	TerminateThread(tPipe, NULL);
	CloseHandle(tPipe);
	CloseHandle(hPipe);

	return 0;
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

		_tprintf(TEXT("-\n"));

		if (!fSuccess || cbRead < msgSize) {
			fSuccess = GetOverlappedResult(hPipe, &oOverlap, &cbRead, FALSE);
			if (!fSuccess) {
				_tprintf(TEXT("[ERRO] Ler do ficheiro do servidor... => %d\n"), GetLastError());
			}
		}
		if (message.Id == 1) {
			system("cls");
			for (int j = 0; j < message.nRows; j++) {
				for (int i = 0; i < message.nColumns; i++) {
					_tprintf(_T("%d"), message.boardGame[i][j]);
				}
				_tprintf(_T("\n"));
			}
		}


	}

	return 0;
}
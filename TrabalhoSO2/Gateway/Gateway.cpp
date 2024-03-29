#include "Gateway.h"
#include "../DLL/DLL.h"
#include <strsafe.h>
#include "../util.h"

BOOL(*writeB) (MSGdata msg);
BOOL(*openMap) ();
HANDLE(*startSemaphore)(LPCWSTR semaphoreName);

HMODULE DLL;
HANDLE eventWriter, eventReader;
HANDLE canWrite, canRead, canReadFromBuffer, canWriterFromBuffer;
GameInfo GInfo;

//PIPES
#define GATEWAY_PIPE_NAME TEXT("\\\\.\\pipe\\gatewayPipe")
#define INSTANCES 5
#define PIPE_TIMEOUT 5000
#define BUFSIZE 4096
#define CONNECTING_STATE 0 
#define READING_STATE 1 
#define WRITING_STATE 2 

typedef struct {
	OVERLAPPED oOverlap;
	HANDLE hPipeInst;
	MSGdata msg;
	DWORD cbRead;
	GameInfo gameInfo;
	DWORD cbToWrite;
	DWORD dwState;
	BOOL fPendingIO;
} PIPEINST, *LPPIPEINST;

PIPEINST Pipe[INSTANCES];
HANDLE hEvents[INSTANCES];

DWORD WINAPI ThreadPipesClientes(void* data);
DWORD WINAPI ThreadBroadcastToClientes(void* data);
BOOL ConnectToNewClient(HANDLE hPipe, LPOVERLAPPED lpo);
VOID DisconnectAndReconnect(DWORD i);
VOID SendPing(LPPIPEINST pipe);

// CLIENT - GATEWAY 



DWORD WINAPI ThreadPipesClientes(void* threaddata) {
	DWORD i, dwWait, cbRet, dwErr;
	BOOL fSuccess;

	for (i = 0; i < INSTANCES; i++) {

		hEvents[i] = CreateEvent(NULL, TRUE, TRUE, NULL);

		if (hEvents[i] == NULL) {
			printf("[ERRO] Criar evento falhou => %d\n", GetLastError());
			return 0;
		}

		Pipe[i].oOverlap.hEvent = hEvents[i];

		Pipe[i].hPipeInst = CreateNamedPipe(GATEWAY_PIPE_NAME, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, INSTANCES,
			BUFSIZE * sizeof(TCHAR), BUFSIZE * sizeof(TCHAR), PIPE_TIMEOUT, NULL);

		if (Pipe[i].hPipeInst == INVALID_HANDLE_VALUE) {
			printf("[ERRO] Criar named pipe falhou => %d\n", GetLastError());
			return 0;
		}

		Pipe[i].fPendingIO = ConnectToNewClient(Pipe[i].hPipeInst, &Pipe[i].oOverlap);
		Pipe[i].dwState = Pipe[i].fPendingIO ? CONNECTING_STATE : READING_STATE;
	}

	while (1) {

		dwWait = WaitForMultipleObjects(INSTANCES, hEvents, FALSE, INFINITE);

		i = dwWait - WAIT_OBJECT_0;
		if (i < 0 || i >(INSTANCES - 1)) {
			printf("Index out of range.\n");
			return 0;
		}

		if (Pipe[i].fPendingIO) {
			fSuccess = GetOverlappedResult(Pipe[i].hPipeInst, &Pipe[i].oOverlap, &cbRet, FALSE);

			switch (Pipe[i].dwState) {
			case CONNECTING_STATE:
				if (!fSuccess) {
					printf("[ERRO] Conectar com o cliente %d => %d\n", Pipe[i].hPipeInst, GetLastError());
					return 0;
				}
				Pipe[i].dwState = READING_STATE;
				break;

			case READING_STATE:
				if (!fSuccess || cbRet == 0) {
					DisconnectAndReconnect(i);
					continue;
				}
				Pipe[i].cbRead = cbRet;
				printf("%d", Pipe[i].cbRead);
				_tprintf(TEXT("\nCliente[%d]: %d\n"), Pipe[i].hPipeInst, Pipe->msg.type);
				Pipe[i].fPendingIO = FALSE;
				Pipe->msg.id = (int)Pipe[i].hPipeInst;
				WaitForSingleObject(canWrite, INFINITE);
				if (writeB(Pipe->msg) == FALSE) {
					_tprintf(TEXT("[ERROR] - %d"), GetLastError());
					return 0;
				}
				ReleaseSemaphore(canRead, 1, NULL);
				break;

			case WRITING_STATE:
				continue;

			default: {
				printf("[ERRO] Estado do pipe ivalido - 1\n");
				return 0;
			}
			}
		}

		switch (Pipe[i].dwState) {
		case READING_STATE:

			fSuccess = ReadFile(Pipe[i].hPipeInst, &Pipe->msg, sizeof(MSG), &Pipe[i].cbRead, &Pipe[i].oOverlap);



			if (fSuccess) {
				ReleaseSemaphore(canWrite, 1, NULL);
				Pipe[i].fPendingIO = FALSE;
				Pipe->msg.id = (int)Pipe[i].hPipeInst;
				WaitForSingleObject(canWrite, INFINITE);
				if (writeB(Pipe->msg) == FALSE) {
					_tprintf(TEXT("[ERROR] - %d"), GetLastError());
					return 0;
				}
				ReleaseSemaphore(canRead, 1, NULL);
				continue;
			}
			dwErr = GetLastError();


			if (!fSuccess && (dwErr == ERROR_IO_PENDING)) {
				Pipe[i].fPendingIO = TRUE;
				continue;
			}

			DisconnectAndReconnect(i);
			break;

		case WRITING_STATE:
			continue;

		default: {
			printf("[ERRO] Estado do pipe ivalido - 2\n");
			return 0;
		}
		}
	}

	return 0;
}

DWORD WINAPI ThreadBroadcastToClientes(void* data) {
	DWORD i, cbRet, dwErr;
	BOOL fSuccess;

	while (1) {

		//WaitForSingleObject DA SHARED MEMORY E ENVIA PARA TODOS OS CLIENTES LIGADOS

		Sleep(1000);

		for (i = 0; i < INSTANCES; i++) {
			//envia mensagem que vem da memoria partilhada para todos os clientes
			if (!Pipe[i].fPendingIO || (Pipe[i].dwState != CONNECTING_STATE)) {

				Pipe[i].dwState = WRITING_STATE;
				SendPing(&Pipe[i]);
				fSuccess = WriteFile(Pipe[i].hPipeInst, &Pipe->gameInfo, Pipe[i].cbToWrite, &cbRet, &Pipe[i].oOverlap);
				printf("\n[INFO] Ping enviado\n", GetLastError());

				dwErr = GetLastError();
				if (!fSuccess && (dwErr != ERROR_IO_PENDING)) {
					printf("\n[ERRO] WriteFile falhou => %d\n", GetLastError());
				}

				WaitForSingleObject(hEvents[i], INFINITE);

				fSuccess = GetOverlappedResult(Pipe[i].hPipeInst, &Pipe[i].oOverlap, &cbRet, FALSE);
				if (!fSuccess)
					printf("\n[ERRO] GetOverlappedResult falhou => %d\n", GetLastError());
				else {
					Pipe[i].dwState = READING_STATE;
					Pipe[i].fPendingIO = FALSE;
				}
			}
		}
	}

	return 0;
}

BOOL ConnectToNewClient(HANDLE hPipe, LPOVERLAPPED lpo) {

	BOOL fConnected, fPendingIO = FALSE;
	fConnected = ConnectNamedPipe(hPipe, lpo);

	if (fConnected) {
		printf("[ERRO] Coneccao com o named pipe falhou => %d\n", GetLastError());
		return 0;
	}

	switch (GetLastError()) {
	case ERROR_IO_PENDING:
		fPendingIO = TRUE;
		break;
	case ERROR_PIPE_CONNECTED:
		if (SetEvent(lpo->hEvent))
			break;
	default: {
		printf("[ERRO] Coneccao com o named pipe falhou => %d\n", GetLastError());
		return 0;
	}
	}

	//printf("[INFO] Cliente (%d) entrou no servidor\n", hPipe);

	return fPendingIO;
}

VOID DisconnectAndReconnect(DWORD i) {

	if (!DisconnectNamedPipe(Pipe[i].hPipeInst)) {
		printf("DisconnectNamedPipe falhou => %d\n", GetLastError());
	}

	Pipe[i].fPendingIO = ConnectToNewClient(Pipe[i].hPipeInst, &Pipe[i].oOverlap);
	Pipe[i].dwState = Pipe[i].fPendingIO ? CONNECTING_STATE : READING_STATE;
}

VOID SendPing(LPPIPEINST pipe) {

	pipe->gameInfo = GInfo;
	pipe->cbToWrite = sizeof(GameInfo);
}
// GATEWAY - SERVER

DWORD WINAPI ThreadReaderSharedMemory(LPVOID PARAMS) {

	GameInfo(*getInfo)();


	while (1) {

		WaitForSingleObject(eventReader, INFINITE);
		//	WaitForSingleObject(canWrite, INFINITE);
		getInfo = (GameInfo(*)()) GetProcAddress(DLL, "getInfoSHM");
		if (getInfo == NULL) {
			_tprintf(TEXT("[SHM ERROR] Loading getDataSHM function from DLL (%d)\n"), GetLastError());
			return 0;
		}
		GInfo = getInfo();


		ResetEvent(eventReader);

	}

	return 1;
}

//DWORD WINAPI ThreadReaderSharedMemoryMessages(LPVOID PARAMS) {
//
//	MSGdata(*getData)();
//	MSGdata aux;
//	GameInfo gi;
//	DWORD bytes;
//	int i;
//	getData = (MSGdata(*)()) GetProcAddress(DLL, "readBuffer");
//	if (getData == NULL) {
//		_tprintf(TEXT("[SHM ERROR] Loading getDataSHM function from DLL (%d)\n"), GetLastError());
//		return 0;
//	}
//	while (1) {
//		
//		WaitForSingleObject(canReadFromBuffer, INFINITE);
//
//		aux = getData();
//		WaitForSingleObject(canWrite, INFINITE);
//		gi.commandId = aux.command;
//		for ( i = 0; i < sizeof(Pipe) / sizeof(Pipe[0]); i++) {
//			if (aux.id == (int)Pipe[i].hPipeInst)
//				break;
//		}
//
//		WriteFile(Pipe[i].hPipeInst, &gi, sizeof(GameInfo), &bytes, &Pipe[i].oOverlap);
//		if (sizeof(GameInfo) < bytes) {
//			_tprintf(TEXT("ERROR"));
//		}
//		ReleaseSemaphore(canRead, 1, NULL);
//		ResetEvent(canReadFromBuffer);
//	}
//
//	return 1;
//}


void ThreadWriteSharedMemory(LPVOID PARAMS) {
	int op;
	MSGdata newData;
	MSGdata data;
	int opc = 0;
	// DLL FUNCTIONS - FUNCTION POINTERS


	while (opc != -1) {

		_tprintf(TEXT("Insert a option: "));
		_tscanf_s(TEXT("%d"), &opc);
		WaitForSingleObject(canWrite, INFINITE);
		data.type = opc;
		if (writeB(data) == FALSE) {
			_tprintf(TEXT("[ERROR] - %d"), GetLastError());
			return;
		}
		ReleaseSemaphore(canRead, 1, NULL);
	}
	return;
}


int _tmain(int argc, LPTSTR argv[]) {

	DLL = LoadLibrary(_T("DLL"));
	if (DLL == NULL) {
		_tprintf(_T("[ERROR] Loading DLL!!"));
		return 0;
	}
	else
		_tprintf(_T("DLL lida com sucesso!\n"));

	startSemaphore = (HANDLE(*)(LPCWSTR semaphoreName))GetProcAddress(DLL, "startSyncSemaphore");
	writeB = (BOOL(*)(MSGdata msg))GetProcAddress(DLL, "writeBuffer");
	openMap = (BOOL(*)())GetProcAddress(DLL, "openGame");

	eventWriter = CreateEvent(NULL, TRUE, FALSE, TEXT("Global\eventWriter"));
	eventReader = CreateEvent(NULL, TRUE, FALSE, TEXT("Global\eventReader"));

	canWrite = startSemaphore(L"writeSemaphore");
	canRead = startSemaphore(L"readSemaphore");

	canReadFromBuffer = CreateEvent(NULL, TRUE, FALSE, TEXT("Global\eventReaderFromBuff"));
	canWriterFromBuffer = CreateEvent(NULL, TRUE, FALSE, TEXT("Global\eventWriterFromBuff"));

	if (writeB == NULL || openMap == NULL || startSemaphore == NULL) {
		_tprintf(TEXT("[SHM ERROR] Loading function from DLL (%d)\n"), GetLastError());
		return 0;
	}


	openMap();

	HANDLE tPipes = CreateThread(NULL, 0, ThreadPipesClientes, NULL, 0, NULL);
	if (tPipes == NULL) {
		printf("[ERRO] Criar thread para os pipes => %d\n", GetLastError());
		return 0;
	}

	HANDLE tBroadcastToClientes = CreateThread(NULL, 0, ThreadBroadcastToClientes, NULL, 0, NULL);
	if (tBroadcastToClientes == NULL) {
		printf("[ERRO] Criar thread para broadcast dos mapas/mensagens => %d\n", GetLastError());
		return 0;
	}
	//HANDLE tClients = CreateThread(NULL, 0, ThreadReaderSharedMemoryMessages, NULL, 0, NULL);
	//if (tClients == NULL) {
	//	printf("[ERRO] Criar thread para broadcast dos mapas/mensagens => %d\n", GetLastError());
	//	return 0;
	//}

	CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)ThreadReaderSharedMemory,
		NULL,
		0,
		0
	);
	HANDLE threadWrite = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)ThreadWriteSharedMemory,
		NULL,
		0,
		0
	);


	Sleep(1000);
	ReleaseSemaphore(canWrite, 1, NULL);

	WaitForSingleObject(threadWrite, INFINITE);
	return 0;
}
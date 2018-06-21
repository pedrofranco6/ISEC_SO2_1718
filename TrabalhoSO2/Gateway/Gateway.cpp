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
	BOOL waitingForCliente;
	HANDLE tClientIO;
} PIPEINST, *LPPIPEINST;

PIPEINST Pipe[INSTANCES];
HANDLE hEvents[INSTANCES];

DWORD WINAPI ThreadPipesClientes(void* data);
DWORD WINAPI ThreadBroadcastToClientes(void* data);
BOOL ConnectToNewClient(HANDLE hPipe, LPOVERLAPPED lpo);
VOID DisconnectAndReconnect(DWORD i);
VOID SendPing(LPPIPEINST pipe);
DWORD WINAPI ThreadHandleEachClient(void* pdata);
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

		Pipe[i].waitingForCliente = ConnectToNewClient(Pipe[i].hPipeInst, &Pipe[i].oOverlap);
	}

	while (1) {

		dwWait = WaitForMultipleObjects(INSTANCES, hEvents, FALSE, INFINITE);

		i = dwWait - WAIT_OBJECT_0;
		if (i < 0 || i >(INSTANCES - 1)) {
			printf("Index out of range.\n");
			return 0;
		}

		if (Pipe[i].waitingForCliente) {

			Pipe[i].tClientIO = CreateThread(NULL, 0, ThreadHandleEachClient, (LPVOID)i, 0, NULL);

			if (Pipe[i].tClientIO == NULL) {
				printf("[ERRO] Criar thread para o cliente %d => %d\n", Pipe[i].hPipeInst, GetLastError());
				return 0;
			}

			Pipe[i].waitingForCliente = FALSE;
		}
	}

	return 0;
}

DWORD WINAPI ThreadHandleEachClient(void* pdata) {

	int i = (int)pdata;
	DWORD cbRet;
	BOOL fSuccess;

	while (1) {
		WaitForSingleObject(hEvents[i], INFINITE);

		fSuccess = ReadFile(Pipe[i].hPipeInst, &Pipe->msg, sizeof(MSG), &Pipe[i].cbRead, &Pipe[i].oOverlap);
		if (!fSuccess && GetLastError() != ERROR_IO_PENDING) {
			printf("[ERRO] Ler do pipe do cliente %d => %d", Pipe[i].hPipeInst, GetLastError());
			DisconnectAndReconnect(i);
			break;
		}

		WaitForSingleObject(hEvents[i], INFINITE);
		fSuccess = GetOverlappedResult(Pipe[i].hPipeInst, &Pipe[i].oOverlap, &cbRet, FALSE);

		if (!fSuccess || cbRet == 0) {
			printf("[ERRO] Overlap falhou => %d", GetLastError());
			DisconnectAndReconnect(i);
			break;
		}

		WaitForSingleObject(canWrite, INFINITE);
		if (writeB(Pipe->msg) == FALSE) {
			_tprintf(TEXT("[ERROR] - %d"), GetLastError());
			return 0;
		}
		ReleaseSemaphore(canRead, 1, NULL);
	}

	return 0;
}

BOOL ConnectToNewClient(HANDLE hPipe, LPOVERLAPPED lpo) {

	BOOL fConnected, waitingForCliente = FALSE;
	fConnected = ConnectNamedPipe(hPipe, lpo);

	if (fConnected) {
		printf("[ERRO] Coneccao com o named pipe falhou => %d\n", GetLastError());
		return 0;
	}

	switch (GetLastError()) {
	case ERROR_IO_PENDING:
		waitingForCliente = TRUE;
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

	return waitingForCliente;
}

VOID DisconnectAndReconnect(DWORD i) {

	if (!DisconnectNamedPipe(Pipe[i].hPipeInst)) {
		printf("DisconnectNamedPipe falhou => %d\n", GetLastError());
	}

	Pipe[i].waitingForCliente = ConnectToNewClient(Pipe[i].hPipeInst, &Pipe[i].oOverlap);
}

DWORD WINAPI ThreadBroadcastToClientes(void* data) {
	DWORD i, cbRet, dwErr;
	BOOL fSuccess;

	while (1) {

		Sleep(1000);

		for (i = 0; i < INSTANCES; i++) {
			if (!Pipe[i].waitingForCliente) {

				SendPing(&Pipe[i]);
				fSuccess = WriteFile(Pipe[i].hPipeInst, &Pipe->gameInfo, Pipe[i].cbToWrite, &cbRet, &Pipe[i].oOverlap);
				printf("\n[INFO] Ping enviado\n", GetLastError());

				if (!fSuccess && GetLastError() != ERROR_IO_PENDING)
					printf("\n[ERRO] WriteFile falhou => %d\n", GetLastError());

				WaitForSingleObject(hEvents[i], INFINITE);

				fSuccess = GetOverlappedResult(Pipe[i].hPipeInst, &Pipe[i].oOverlap, &cbRet, FALSE);
				if (!fSuccess)
					printf("\n[ERRO] GetOverlappedResult falhou => %d\n", GetLastError());
			}
		}
	}

	return 0;
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
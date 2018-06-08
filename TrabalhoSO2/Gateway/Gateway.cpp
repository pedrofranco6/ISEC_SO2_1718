#include "Gateway.h"
#include "../DLL/DLL.h"

BOOL(*writeB) (data data);
BOOL(*openMap) ();
HANDLE(*startSemaphore)(LPCWSTR semaphoreName);

HMODULE DLL;
HANDLE eventWriter, eventReader;
HANDLE canWrite, canRead;


DWORD WINAPI ThreadReaderSharedMemory(LPVOID PARAMS) {

	GameInfo(*getInfo)();
	GameInfo aux;

	while (1) {

		WaitForSingleObject(eventReader, INFINITE);
		getInfo = (GameInfo(*)()) GetProcAddress(DLL, "getInfoSHM");
		if (getInfo == NULL) {
			_tprintf(TEXT("[SHM ERROR] Loading getDataSHM function from DLL (%d)\n"), GetLastError());
			return 0;
		}
		aux = getInfo();
		if (getInfo().Id == 1) {
			system("cls");
			for (int j = 0; j < aux.nRows; j++) {
				for (int i = 0; i < aux.nColumns; i++) {
					_tprintf(_T("%d"), aux.boardGame[i][j]);
				}
				_tprintf(_T("\n"));
			}
		}

		ResetEvent(eventReader);

	}

	return 1;
}


void ThreadWriteSharedMemory(LPVOID PARAMS) {
	int op;
	data newData;
	data data;
	int opc = 0;
	// DLL FUNCTIONS - FUNCTION POINTERS


			while (opc != -1) {
				WaitForSingleObject(canWrite, INFINITE);
				_tprintf(TEXT("Insert a option: "));
				_tscanf_s(TEXT("%d"), &opc);
				data.op = opc;
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
	writeB = (BOOL(*)(data data))GetProcAddress(DLL, "writeBuffer");
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
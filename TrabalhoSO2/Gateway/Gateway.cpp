#include "Gateway.h"
#include "../DLL/DLL.h"

BOOL(*writeB) (data data);
BOOL(*openMap) ();
HMODULE DLL;
HANDLE eventWriter, eventReader;



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


void testeFunc() {
	int op;
	data newData;

	// DLL FUNCTIONS - FUNCTION POINTERS
	void(*setData)(data);

	do {
		_tprintf(TEXT("insira um valor:"));
		_tscanf_s(TEXT("%d"), &op);

		setData = (void(*)(data)) GetProcAddress(DLL, "writeBuffer");
		if (setData == NULL) {
			_tprintf(TEXT("[SHM ERROR] GetProcAddress - setData()\n"));
			return;
		}
		newData.op = op;
		setData(newData);
		SetEvent(eventWriter);

	} while (op = !0);

}


int _tmain(int argc, LPTSTR argv[]) {

	DLL = LoadLibrary(_T("DLL"));
	if (DLL == NULL) {
		_tprintf(_T("[ERROR] Loading DLL!!"));
		return 0;
	}
	else
		_tprintf(_T("DLL lida com sucesso!\n"));
	eventReader = CreateEvent(NULL, TRUE, FALSE, TEXT("Global\eventWriter"));
	eventWriter = CreateEvent(NULL, TRUE, FALSE, TEXT("Global\eventReader"));

	writeB = (BOOL(*)(data data))GetProcAddress(DLL, "writeBuffer");
	openMap = (BOOL(*)())GetProcAddress(DLL, "openGame");

	if (writeB == NULL || openMap == NULL) {
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
	testeFunc();

	data data;
	int opc = 0;
	while (opc != -1) {
		_tprintf(TEXT("Insert a option: "));
		_tscanf_s(TEXT("%d"), &opc);
		data.op = opc;
		if (writeB(data) == FALSE) {
			_tprintf(TEXT("[ERROR] - %d"), GetLastError());
			return 0;
		}
	}

	return 0;
}
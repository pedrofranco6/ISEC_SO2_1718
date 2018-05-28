#include "Gateway.h"
#include "../DLL/DLL.h"

BOOL (*message) (int number);
BOOL(*startgame) ();
HMODULE DLL;

int _tmain(int argc, LPTSTR argv[]) {

	DLL = LoadLibrary(_T("DLL"));
	if (DLL == NULL) {
		_tprintf(_T("[ERROR] Loading DLL!!"));
		return 0;
	}
	else
		_tprintf(_T("DLL lida com sucesso!\n"));


	message = (BOOL(*)(int number))GetProcAddress(DLL, "writeString");
	startgame = (BOOL(*)())GetProcAddress(DLL, "startGame");

	if (message == NULL || startgame == NULL) {
		_tprintf(TEXT("[SHM ERROR] Loading function from DLL (%d)\n"), GetLastError());
		return 0;
	}

	startgame();

	int opc = 0;
	while (opc != -1) {
		_tprintf(TEXT("Insert a option: "));
		_tscanf_s(TEXT("%d"), &opc);
		if (message(opc) == FALSE) {
			_tprintf(TEXT("[ERROR] - %d"), GetLastError());
			return 0;
		}
	}

	return 0;
}
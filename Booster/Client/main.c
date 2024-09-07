#include <stdio.h>
#include <Windows.h>
#include "..\..\Booster\Booster\BoosterCommon.h"

int main(const int argc, const char* argv[]) {
	if (argc < 3) {
		printf("Usage: Boost <threadid> <priority>\n");
		return 1;
	}

	DWORD dwReturn = NULL;
	UINT iThreadId = atoi(argv[1]);
	UINT iPriority = atoi(argv[2]);
	ThreadData sThData = { .ThreadId = iThreadId, .Priority = iPriority };

	printf("[#] Press <Enter> to Load the Device Object!\n");
	getch();
	HANDLE hDevice = CreateFile(L"\\\\.\\Booster", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (!hDevice) {
		printf("[-] CreateFile failed with status: %d\n", GetLastError());
		return 1;
	}
	printf("[+] Done!\n\n");

	printf("[#] Press <Enter> to Call the Driver!\n");
	getch();
	if (!WriteFile(hDevice, &sThData, sizeof(sThData), &dwReturn, NULL)) {
		printf("[-] WriteFile failed with status: %d\n", GetLastError());
		goto _EndOfFunction;
	}
	printf("[i] Priority Changed Successfully!\n");
	printf("[+] Done!\n\n");

_EndOfFunction:
	if (hDevice)
		CloseHandle(hDevice);
	return 0;
}
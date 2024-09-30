#include <stdio.h>
#include <Windows.h>
#include "../Zero/ZeroCommon.h"


int main(const int argc, const char* argv[]) {
	HANDLE hDriver = NULL;
	BYTE bBuffer[64];
	BYTE bBuffer2[1024];
	DWORD dwBytes = NULL;
	ZeroStats sZeroStats;

	hDriver = CreateFileW(L"\\\\.\\Zero", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (!hDriver) {
		printf("[-] CreateFileW Failed with status: %d\n", GetLastError());
		return EXIT_FAILURE;
	}

	for (int i = 0; i < 64; i++)
		bBuffer[i] = i + 1;

	printf("[i] Test Read!\n");
	if (!ReadFile(hDriver, bBuffer, sizeof(bBuffer), &dwBytes, NULL) || sizeof(bBuffer) != dwBytes) {
		printf("[-] ReadFile Failed with status: %d\n", GetLastError());
		goto _EndOfFunction;
	}

	for(int i = 0; i < 64; i++)
		if (bBuffer[i] != 0) {
			printf("[-] Wrong Data on Buffer!\n");
			break;
		}


	printf("[i] Test Write!\n");
	if (!WriteFile(hDriver, bBuffer2, sizeof(bBuffer2), &dwBytes, NULL) || sizeof(bBuffer2) != dwBytes) {
		printf("[-] WriteFile Failed with status: %d\n", GetLastError());
		goto _EndOfFunction;
	}


	if (!DeviceIoControl(hDriver, IOCTL_ZERO_GET_STATS, NULL, 0, &sZeroStats, sizeof(sZeroStats), &dwBytes, NULL)) {
		printf("[-] DeviceIoControl Failed with status: %d\n", GetLastError());
		goto _EndOfFunction;
	}

	printf("[+] Number of Bytes Read: %d\tNumber Of Bytes Written: %d\n", sZeroStats.TotalRead, sZeroStats.TotalWrite);

_EndOfFunction:
	if (hDriver)
		CloseHandle(hDriver);

	return EXIT_SUCCESS;
}
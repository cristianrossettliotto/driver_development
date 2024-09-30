#include <ntddk.h>
#include <stdarg.h>
#include "BoosterMacros.h"


ULONG Log(LogLevel eLevel, PCSTR sFormat, ...) {
	va_list list;
	va_start(list, sFormat);
	return vDbgPrintExWithPrefix("Booster", DPFLTR_IHVDRIVER_ID, (ULONG)eLevel, sFormat, list);
}

ULONG LogInfo(PCSTR sFormat, ...) {
	va_list list;
	va_start(list, sFormat);
	return vDbgPrintEx(DPFLTR_IHVDRIVER_ID, (ULONG)(INFORMATION), sFormat, list);
}

ULONG LogError(PCSTR sFormat, ...) {
	va_list list;
	va_start(list, sFormat);
	return vDbgPrintEx(DPFLTR_IHVDRIVER_ID, (ULONG)(ERROR), sFormat, list);
}

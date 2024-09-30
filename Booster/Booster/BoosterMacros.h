#pragma once

typedef enum {
	ERROR = 0,
	WARNING,
	INFORMATION,
	DEBUG,
	VERBOSE
} LogLevel;


ULONG Log(LogLevel eLevel, PCSTR sFormat, ...);
ULONG LogInfo(PCSTR sFormat, ...);
ULONG LogError(PCSTR sFormat, ...);

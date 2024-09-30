#pragma once

#define DEVICE_ZERO 0x8022

#define IOCTL_ZERO_GET_STATS CTL_CODE(DEVICE_ZERO, 0X800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_ZERO_CLEAR_STATS CTL_CODE(DEVICE_ZERO, 0X801, METHOD_NEITHER, FILE_ANY_ACCESS)

typedef struct {
	unsigned long long TotalRead;
	unsigned long long TotalWrite;
}ZeroStats;
#include "pch.h"
#include "ZeroCommon.h"

#define DRIVER_PREFIX "Zero:"



VOID ZeroUnload(PDRIVER_OBJECT pDriverObject);
DRIVER_DISPATCH ZeroCreateClose, ZeroRead, ZeroWrite, ZeroDeviceControl;

LONG64 g_TotalRead = 0;
LONG64 g_TotalWrite = 0;

UNICODE_STRING sSymbolicName = RTL_CONSTANT_STRING(L"\\??\\Zero");

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT pDriverObject, _In_ PUNICODE_STRING RegistryPath) {
	UNREFERENCED_PARAMETER(RegistryPath);

	NTSTATUS ntStatus = STATUS_SUCCESS;
	PDEVICE_OBJECT pDeviceObject = NULL;
	UNICODE_STRING sDeviceName = RTL_CONSTANT_STRING(L"\\Device\\Zero");

	pDriverObject->DriverUnload = ZeroUnload;
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = pDriverObject->MajorFunction[IRP_MJ_CLOSE] = ZeroCreateClose;
	pDriverObject->MajorFunction[IRP_MJ_READ] = ZeroRead;
	pDriverObject->MajorFunction[IRP_MJ_WRITE] = ZeroWrite;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ZeroDeviceControl;

	do {

		ntStatus = IoCreateDevice(pDriverObject, 0, &sDeviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &pDeviceObject);
		if (!NT_SUCCESS(ntStatus)) {
			KdPrint((DRIVER_PREFIX " failed to create device (0x%08x)\n", ntStatus));
			break;
		}

		pDeviceObject->Flags |= DO_DIRECT_IO;

		ntStatus = IoCreateSymbolicLink(&sSymbolicName, &sDeviceName);
		if (!NT_SUCCESS(ntStatus)) {
			KdPrint((DRIVER_PREFIX " failed to create symbolik link (0x%08x)\n", ntStatus));
			break;
		}

	} while (FALSE);

	if (!NT_SUCCESS(ntStatus) && pDeviceObject)
		IoDeleteDevice(pDeviceObject);

	return ntStatus;
}

VOID ZeroUnload(PDRIVER_OBJECT pDriverObject) {
	IoDeleteSymbolicLink(&sSymbolicName);
	IoDeleteDevice(pDriverObject->DeviceObject);
}

NTSTATUS CompleteIrp(PIRP pIrp, NTSTATUS ntStatus, ULONG_PTR info) {
	pIrp->IoStatus.Status = ntStatus;
	pIrp->IoStatus.Information = info;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return ntStatus;
}


NTSTATUS ZeroCreateClose(PDEVICE_OBJECT pDeviceObject, PIRP pIrp) {
	UNREFERENCED_PARAMETER(pDeviceObject);
	return CompleteIrp(pIrp, STATUS_SUCCESS, 0);
}


NTSTATUS ZeroRead(PDEVICE_OBJECT pDeviceObject, PIRP pIrp) {
	UNREFERENCED_PARAMETER(pDeviceObject);
	PIO_STACK_LOCATION pStack = IoGetCurrentIrpStackLocation(pIrp);
	ULONG lStackSize = pStack->Parameters.Read.Length;
	if (lStackSize == 0)
		return CompleteIrp(pIrp, STATUS_INVALID_BUFFER_SIZE, 0);

	// Verify if Direct I/O was set
	NT_ASSERT(pIrp->MdlAddress);

	PVOID pUserBufferInKernelSpace = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
	if(!pUserBufferInKernelSpace)
		return CompleteIrp(pIrp, STATUS_INSUFFICIENT_RESOURCES, 0);

	memset(pUserBufferInKernelSpace, 0, lStackSize);
	InterlockedAdd64(&g_TotalRead, lStackSize);

	return CompleteIrp(pIrp, STATUS_SUCCESS, lStackSize);
}


NTSTATUS ZeroWrite(PDEVICE_OBJECT pDeviceObject, PIRP pIrp) {
	UNREFERENCED_PARAMETER(pDeviceObject);
	PIO_STACK_LOCATION pStack = IoGetCurrentIrpStackLocation(pIrp);
	ULONG lStackSize = pStack->Parameters.Read.Length;
	InterlockedAdd64(&g_TotalWrite, lStackSize);
	return CompleteIrp(pIrp, STATUS_SUCCESS, lStackSize);
}


NTSTATUS ZeroDeviceControl(PDEVICE_OBJECT pDeviceObject, PIRP pIrp) {
	UNREFERENCED_PARAMETER(pDeviceObject);
	ULONG_PTR pLength = 0;
	NTSTATUS ntStatus = STATUS_INVALID_DEVICE_REQUEST;
	PIO_STACK_LOCATION pStack = IoGetCurrentIrpStackLocation(pIrp);
	
	switch (pStack->Parameters.DeviceIoControl.IoControlCode) {
		case IOCTL_ZERO_GET_STATS:
			if (pStack->Parameters.DeviceIoControl.OutputBufferLength < sizeof(ZeroStats)) {
				ntStatus = STATUS_BUFFER_TOO_SMALL;
				break;
			}
			
			ZeroStats * pZeroStats = (ZeroStats*) pIrp->AssociatedIrp.SystemBuffer;
			if (!pZeroStats) {
				ntStatus = STATUS_INVALID_PARAMETER;
				break;
			}

			pZeroStats->TotalRead = g_TotalRead;
			pZeroStats->TotalWrite = g_TotalWrite;
			pLength = sizeof(ZeroStats);
			ntStatus = STATUS_SUCCESS;
			break;
		case IOCTL_ZERO_CLEAR_STATS:
			g_TotalRead = g_TotalWrite = 0;
			ntStatus = STATUS_SUCCESS;
			break;
	}

	return CompleteIrp(pIrp, ntStatus, pLength);
}


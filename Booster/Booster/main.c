#include <ntifs.h>
#include <ntddk.h> // ntddk have to be after ntifs otherwise can give compilation error
#include "BoosterCommon.h" // something that can be made is remove ntddk, because is imported in the ntifs
#include <TraceLoggingProvider.h>
#include <evntrace.h>
#include "BoosterMacros.h"


TRACELOGGIN_DEFINE_PROVIDER(g_Provider, "Booster", (6f16c787 - a4a4 - 4941 - b48f - d237e69a66e3));

VOID BoosterUnload(_In_ PDRIVER_OBJECT DriverObject);

NTSTATUS BoosterCreateClose(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);

NTSTATUS BoosterWrite(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath) {
	Log(INFORMATION, "Booster: DriverEntry called. Registry Path: %wZ\n", RegistryPath);
	NTSTATUS ntStatus = STATUS_SUCCESS;
	PDEVICE_OBJECT pDeviceObject = NULL;
	UNICODE_STRING sDeviceName = RTL_CONSTANT_STRING(L"\\Device\\Booster");
	UNICODE_STRING sSymbolicName = RTL_CONSTANT_STRING(L"\\??\\Booster");

	DriverObject->DriverUnload = BoosterUnload;

	DriverObject->MajorFunction[IRP_MJ_CREATE] = BoosterCreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = BoosterCreateClose;
	DriverObject->MajorFunction[IRP_MJ_WRITE] = BoosterWrite;

	
	ntStatus = IoCreateDevice(DriverObject, 0, &sDeviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &pDeviceObject);
	if (!NT_SUCCESS(ntStatus)) {
		LogError("[-] IoCreateDevice failed with status: (0x%08X)\n", ntStatus);
		return ntStatus;
	}

	NT_ASSERT(pDeviceObject);

	ntStatus = IoCreateSymbolicLink(&sSymbolicName, &sDeviceName);
	if (!NT_SUCCESS(ntStatus)) {
		LogError("[-] IoCreateDevice failed with status: (0x%08X)\n", ntStatus);
		IoDeleteDevice(pDeviceObject);
		return ntStatus;
	}

	NT_ASSERT(NT_SUCCESS(ntStatus));

	return STATUS_SUCCESS;
}

VOID BoosterUnload(_In_ PDRIVER_OBJECT DriverObject) {
	LogInfo("Booster unload called!\n");

	UNICODE_STRING sSymbolicName = RTL_CONSTANT_STRING(L"\\??\\Booster");

	IoDeleteSymbolicLink(&sSymbolicName);

	IoDeleteDevice(DriverObject->DeviceObject);
}

NTSTATUS BoosterCreateClose(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp) {
	Log(VERBOSE, "Booster Create/Close called!\n");
	UNREFERENCED_PARAMETER(DeviceObject);

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS BoosterWrite(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);
	NTSTATUS ntStatus = STATUS_SUCCESS;
	PIO_STACK_LOCATION pIrpStack = NULL;
	ThreadData* pThreadData = NULL;
	PETHREAD pEthread = NULL;
	KPRIORITY kPriority = 0;
	ULONG_PTR pInformation = 0;

	pIrpStack = IoGetCurrentIrpStackLocation(Irp);

	if (pIrpStack->Parameters.Write.Length < sizeof(ThreadData)) {
		ntStatus = STATUS_BUFFER_TOO_SMALL;
		LogError("[-] Buffer size is less than expected!\n");
		goto _EndOfFunction;
	}

	pThreadData = (ThreadData*)Irp->UserBuffer;
	if (!pThreadData || pThreadData->Priority < 1 || pThreadData->Priority > 31) {
		ntStatus = STATUS_INVALID_PARAMETER;
		LogError("[-] The parameter is NULL or his value is not between 0 and 31!\n");
		goto _EndOfFunction;
	}

	ntStatus = PsLookupThreadByThreadId(ULongToHandle(pThreadData->ThreadId), &pEthread);
	if (!NT_SUCCESS(ntStatus)) {
		LogError("[-] PsLookupThreadByThreadId/ULongToHandle failed with status: (0x%08X)\n", ntStatus);
		goto _EndOfFunction;
	}

	kPriority = KeSetPriorityThread(pEthread, pThreadData->Priority);

	LogInfo("[+] Priority Change for Thread %u from %d to %d\n", pThreadData->ThreadId, kPriority, pThreadData->Priority);

	ObDereferenceObject(pEthread);
	pInformation = sizeof(pThreadData);

_EndOfFunction:
	Irp->IoStatus.Status = ntStatus;
	Irp->IoStatus.Information = pInformation;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return ntStatus;
}
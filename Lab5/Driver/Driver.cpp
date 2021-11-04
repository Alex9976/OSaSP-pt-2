#include "Driver.h"

PDEVICE_OBJECT g_pDeviceObject;

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	UNICODE_STRING sPsGetProcessImageFileName = RTL_CONSTANT_STRING(L"PsGetProcessImageFileName");
	gPsGetProcessImageFileName = (GET_PROCESS_IMAGE_NAME)MmGetSystemRoutineAddress(&sPsGetProcessImageFileName);
	if (!gPsGetProcessImageFileName)
	{
		DbgPrintEx(0, 0, "PSGetProcessImageFileName not found\n");
		return STATUS_UNSUCCESSFUL;
	}

	UNICODE_STRING DeviceName, Win32Device;
	NTSTATUS status;
	PDEVICE_OBJECT DeviceObject = NULL;

	RtlInitUnicodeString(&DeviceName, L"\\Device\\Lab5"/*DRIVER_NAME*/);
	RtlInitUnicodeString(&Win32Device, L"\\DosDevices\\Lab5"/*DRIVER_NAME */ );

	unsigned i;
	for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
		DriverObject->MajorFunction[i] = DriverUnsupportedHandler;

	DriverObject->MajorFunction[IRP_MJ_CREATE] = DriverCreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = DriverCreateClose;
	DriverObject->MajorFunction[IRP_MJ_SHUTDOWN] = DriverShutdown;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DriverDispatchIoctl;
	DriverObject->DriverUnload = (PDRIVER_UNLOAD)DriverUnload;

	status = IoCreateDevice(DriverObject, sizeof(DriverVariables), &DeviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &DeviceObject);
	if (!NT_SUCCESS(status))
		return status;
	if (!DeviceObject)
		return STATUS_UNEXPECTED_IO_ERROR;

	DeviceObject->Flags |= DO_DIRECT_IO;
	DeviceObject->AlignmentRequirement = FILE_WORD_ALIGNMENT;
	status = IoCreateSymbolicLink(&Win32Device, &DeviceName);
	DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
	g_pDeviceObject = DeviceObject;
	
	PDriverVariables driverInformation = GetDeviceVariable();
	driverInformation->startInitialize();
	DbgPrintEx(0, 0, "\nDriver load\n");
	return STATUS_SUCCESS;
}

NTSTATUS DriverUnload(PDRIVER_OBJECT DriverObject)
{
	UnloadControlProcHandler();
	UNICODE_STRING Win32Device;
	RtlInitUnicodeString(&Win32Device, L"\\DosDevices\\Lab5");
	IoDeleteSymbolicLink(&Win32Device);
	IoDeleteDevice(DriverObject->DeviceObject);
	DbgPrintEx(0, 0, "\nDriver unload\n");
	return STATUS_SUCCESS;
}

void sCreateProcessNotifyRoutine(HANDLE ppid, HANDLE pid, BOOLEAN create)
{
	PDriverVariables driverInformation = GetDeviceVariable();
	driverInformation->processNotify.initProcess(ppid, pid, create);
	PIO_WORKITEM allocWorkItem = IoAllocateWorkItem(g_pDeviceObject);
	IoQueueWorkItem(allocWorkItem, WorkItem, DelayedWorkQueue, allocWorkItem);
}

NTSTATUS DriverUnsupportedHandler(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	DbgPrintEx(0, 0, "Not supported Handler\n");
	Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return Irp->IoStatus.Status;
}

NTSTATUS DriverCreateClose(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	DbgPrintEx(0, 0, "Create Close Handler\n");
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS DriverShutdown(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	DbgPrintEx(0, 0, "Shutdown\n");
	UnloadControlProcHandler();
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS DriverDispatchIoctl(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	DbgPrintEx(0, 0, "Dispatch IOctl\n");

	NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;
	PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);

	switch (irpStack->Parameters.DeviceIoControl.IoControlCode)
	{
	case IOCTL_DRIVER_CONTROLLER_SWITCH_CONTROL_PROC_STATE:
		ntStatus = SwitchMonitoringHandler(Irp);
		break;
	}

	Irp->IoStatus.Status = ntStatus;

	Irp->IoStatus.Information = (ntStatus == STATUS_SUCCESS) ? irpStack->Parameters.DeviceIoControl.OutputBufferLength : 0;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return ntStatus;
}

NTSTATUS SwitchMonitoringHandler(IN PIRP Irp)
{
	DbgPrintEx(0, 0, "Call SwitchMonitor\n");
	NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;
	PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);

	if (irpStack->Parameters.DeviceIoControl.InputBufferLength == sizeof(ActivateHandlerProc))
	{
		PActivateHandlerProc pActivateInfo = (PActivateHandlerProc)(Irp->AssociatedIrp.SystemBuffer);
		PDriverVariables driverInformation = GetDeviceVariable();

		if (driverInformation->activateProc.isActivate != pActivateInfo->isActivate)
		{
			CopyActivateProc(pActivateInfo);
			ntStatus = CreateProcessNotify(!pActivateInfo->isActivate);
		}
		else
			DbgPrintEx(0, 0, "Repeat NotifyProc state\n");
	}
	else
		DbgPrintEx(0, 0, "Activate Handler not equals size\n");

	return ntStatus;
}

PDriverVariables GetDeviceVariable()
{
	return (PDriverVariables)g_pDeviceObject->DeviceExtension;
}

NTSTATUS CreateProcessNotify(IN BOOLEAN isDisabled)
{
	DbgPrintEx(0, 0, "ProcessNotify %s\n", isDisabled ? "disabled" : "enabled");
	return PsSetCreateProcessNotifyRoutine(sCreateProcessNotifyRoutine, isDisabled);
}

VOID UnloadControlProcHandler()
{
	PDriverVariables driverInformation = GetDeviceVariable();

	if (driverInformation->activateProc.isActivate)
		CreateProcessNotify(TRUE);

	driverInformation->finalize();
}

VOID WorkItem(IN PDEVICE_OBJECT DeviceObject, IN OPTIONAL PVOID Context)
{
	if (IsControlProc())
	{
		if (!IsParentIdManager())
			SwitchControlProcXState();
		else
			DbgPrint("ParentId is current manager\n");
	}
	IoFreeWorkItem((PIO_WORKITEM)Context);
}

BOOLEAN IsControlProc()
{
	PDriverVariables driverInformation = GetDeviceVariable();
	PCSTR szProcessName = GetProcessFileNameById(driverInformation->processNotify.hProcessID);
	DbgPrint("WorkItem %s.\n", szProcessName);
	return RtlEqualMemory(szProcessName, driverInformation->activateProc.procName, strlen(szProcessName));
}

PCSTR GetProcessFileNameById(IN HANDLE handle)
{
	PEPROCESS Process;
	PsLookupProcessByProcessId(handle, &Process);

	return gPsGetProcessImageFileName(Process);
}

VOID SwitchControlProcXState()
{
	PDriverVariables driverInformation = GetDeviceVariable();
	DbgPrintEx(0, 0, "NotifyProc %s is %s\n", driverInformation->activateProc.procName,
		driverInformation->processNotify.isCreate ? "created" : "closed");
	if (driverInformation->processNotify.isCreate)
		driverInformation->syncObjects.setCreateEvent();
	else
		driverInformation->syncObjects.setCloseEvent();
}

BOOLEAN IsParentIdManager()
{
	PDriverVariables driverInformation = GetDeviceVariable();
	PCSTR szProcessName = GetProcessFileNameById(driverInformation->processNotify.hParentID);
	return RtlEqualMemory(szProcessName, driverInformation->activateProc.managerName, strlen(szProcessName));
}

VOID CopyActivateProc(IN PActivateHandlerProc pActivateInfo)
{
	PDriverVariables driverInformation = GetDeviceVariable();
	RtlCopyMemory(&driverInformation->activateProc, pActivateInfo, sizeof(driverInformation->activateProc));
	DbgPrintEx(0, 0, "Copy procName %s\n", (driverInformation->activateProc.procName == NULL ? "Null" : driverInformation->activateProc.procName));
}
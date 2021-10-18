#include "Driver.h"
#include <ntifs.h>

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	DriverObject->DriverUnload = DriverUnload;
	PsSetCreateProcessNotifyRoutine(sCreateProcessNotifyRoutine, FALSE);
	DbgPrintEx(0, 0, "\nDriver load\n");
	return STATUS_SUCCESS;
}

NTSTATUS DriverUnload(PDRIVER_OBJECT DriverObject)
{
	PsSetCreateProcessNotifyRoutine(sCreateProcessNotifyRoutine, TRUE);
	DbgPrintEx(0, 0, "\nDriver unload\n");
	return STATUS_SUCCESS;
}

void sCreateProcessNotifyRoutine(HANDLE ppid, HANDLE pid, BOOLEAN create)
{
	if (create)
	{
		PEPROCESS process = NULL;
		PUNICODE_STRING parentProcessName = NULL, processName = NULL;
		PsLookupProcessByProcessId(ppid, &process);
		SeLocateProcessImageName(process, &parentProcessName);
		PsLookupProcessByProcessId(pid, &process);
		SeLocateProcessImageName(process, &processName);
		DbgPrintEx(0, 0, "\n%d %wZ\n\t\t%d %wZ\n", ppid, parentProcessName, pid, processName);
	}
	else
	{
		DbgPrintEx(0, 0, "Process %d lost child %d", ppid, pid);
	}
}
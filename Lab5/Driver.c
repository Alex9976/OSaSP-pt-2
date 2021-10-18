#include "Driver.h"

wchar_t* trackedProcess = L"calc.exe";

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
	PEPROCESS process = NULL;
	PUNICODE_STRING parentProcessName = NULL, processName = NULL;
	PsLookupProcessByProcessId(ppid, &process);
	SeLocateProcessImageName(process, &parentProcessName);
	PsLookupProcessByProcessId(pid, &process);
	SeLocateProcessImageName(process, &processName);
	if (isSubstrInStr(parentProcessName, trackedProcess))
	{
		if (create)
		{
			DbgPrintEx(0, 0, "\nProcess %d %wZ open\n", ppid, parentProcessName);
		}
		else
		{
			DbgPrintEx(0, 0, "\nProcess %d %wZ closed\n", ppid, parentProcessName);
		}
	}
}

BOOLEAN isSubstrInStr(PUNICODE_STRING str, PUNICODE_STRING substr)
{
	int strLength = wcslen(str->Buffer);
	wchar_t buffer[255] = { 0 };
	wcscpy_s(buffer, strLength + 1, str->Buffer);
	if (wcsstr(buffer, trackedProcess))
	{
		return TRUE;
	}
	return FALSE;
}
#include "Driver.h"
//\Device\HarddiskVolume2\Windows\notepad.exe
wchar_t* trackedProcess = L"win32calc.exe";
HANDLES openProcessesTable[255] = { 0 };
int openProcessesCount;

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

	if (isSubstrInStr(processName, trackedProcess))
	{
		if (create)
		{
			
			openProcessesTable[openProcessesCount].trackedProc = pid;
			++openProcessesCount;
			DbgPrintEx(0, 0, "\nProcess %d %wZ open\n", pid, processName);
		}
		else
		{
			findAndCloseProcess(pid);
			DbgPrintEx(0, 0, "\nProcess %d %wZ closed\n", pid, processName);
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

void findAndCloseProcess(HANDLE pid)
{
	PEPROCESS process = NULL;
	int pos = -1;
	for (int i = 0; i < openProcessesCount; i++)
	{
		if (openProcessesTable[i].trackedProc == pid)
		{
			NTSTATUS status = PsLookupProcessByProcessId(openProcessesTable[i].concomitantProcess, &process);
			if (status != STATUS_SUCCESS)
			{
				KeAttachProcess(process);
				ZwTerminateProcess(NULL, 0);
				KeDetachProcess();
				ObDereferenceObject(process);
			}
			pos = i;
		}
	}
	if (pos != -1)
	{
		for (int i = pos; i < openProcessesCount - 1; i++)
		{
			openProcessesTable[i].trackedProc = openProcessesTable[i + 1].trackedProc;
			openProcessesTable[i].concomitantProcess = openProcessesTable[i + 1].concomitantProcess;
		}
		--openProcessesCount;
	}
}
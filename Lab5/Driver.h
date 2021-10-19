#pragma once
#include <ntddk.h>
#define PREPROCESS PREPROCESSntifs
#define PETHREAD PETHREADntifs
#define PEPROCESS PEPROCESSntifs
#include <ntifs.h>

typedef struct _HANDLES {
	HANDLE trackedProc;
	HANDLE concomitantProcess;
} *PHANDLES, HANDLES;

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);
NTSTATUS DriverUnload(PDRIVER_OBJECT DriverObject);
void sCreateProcessNotifyRoutine(HANDLE ppid, HANDLE pid, BOOLEAN create);
BOOLEAN isSubstrInStr(PUNICODE_STRING str, PUNICODE_STRING substr);
void findAndCloseProcess(HANDLE ppid);
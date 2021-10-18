#pragma once
#include <ntddk.h>
#define PREPROCESS PREPROCESSntifs
#define PETHREAD PETHREADntifs
#define PEPROCESS PEPROCESSntifs
#include <ntifs.h>



NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);
NTSTATUS DriverUnload(PDRIVER_OBJECT DriverObject);
void sCreateProcessNotifyRoutine(HANDLE ppid, HANDLE pid, BOOLEAN create);
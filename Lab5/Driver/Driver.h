#pragma once
#include <ntddk.h>
#define PREPROCESS PREPROCESSntifs
#define PETHREAD PETHREADntifs
#define PEPROCESS PEPROCESSntifs
#include <ntifs.h>
#include "GlobalVaribles.h"

typedef PCHAR(*GET_PROCESS_IMAGE_NAME) (PEPROCESS Process); 
GET_PROCESS_IMAGE_NAME gGetProcessImageFileName;
GET_PROCESS_IMAGE_NAME gPsGetProcessImageFileName;

typedef struct _DriverVariables
{
    inline void startInitialize()
    {
        RtlZeroMemory(activateProc.procName, sizeof(activateProc.procName));
        activateProc.isActivate = FALSE;
        syncObjects.initialize();
    }

    inline void finalize()
    {
        syncObjects.close();
    }

    typedef struct _Process_Notify
    {
        inline void initProcess(HANDLE hParentID, HANDLE hProcessID, BOOLEAN isCreate)
        {
            this->hParentID = hParentID;
            this->hProcessID = hProcessID;
            this->isCreate = isCreate;
        }
        HANDLE hParentID;
        HANDLE hProcessID;
        BOOLEAN isCreate;
    }Process_Notify;

    typedef struct _SyncObject
    {
        inline void initialize()
        {
            UNICODE_STRING uszProcessEventString;
            RtlInitUnicodeString(&uszProcessEventString, L"\\BaseNamedObjects\\CreateProcEvent"/*SYNC_CREATE_PROC_EVENT*/);
            createProcEvent = IoCreateNotificationEvent(&uszProcessEventString, &createProcHandle);
            KeClearEvent(createProcEvent);

            RtlInitUnicodeString(&uszProcessEventString, L"\\BaseNamedObjects\\CloseProcEvent"/*SYNC_CLOSE_PROC_EVENT*/);
            closeProcEvent = IoCreateNotificationEvent(&uszProcessEventString, &closeProcHandle);
            KeClearEvent(closeProcEvent);
        };

        inline void close()
        {
            ZwClose(createProcHandle);
            ZwClose(closeProcHandle);
        }

        inline void setCreateEvent()
        {
            peekEvent(createProcEvent);
        }

        inline void setCloseEvent()
        {
            peekEvent(closeProcEvent);
        }
    private:
        inline void peekEvent(PKEVENT pEvent)
        {
            KeSetEvent(pEvent, 0, FALSE);
            KeClearEvent(pEvent);
        }

        PKEVENT createProcEvent;
        HANDLE createProcHandle;
        PKEVENT closeProcEvent;
        HANDLE closeProcHandle;
    }SyncObject;

    SyncObject syncObjects;
    Process_Notify processNotify;
    ActivateHandlerProc activateProc;

}DriverVariables, * PDriverVariables;

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);
NTSTATUS DriverUnload(PDRIVER_OBJECT DriverObject);
void sCreateProcessNotifyRoutine(HANDLE ppid, HANDLE pid, BOOLEAN create);
NTSTATUS DriverUnsupportedHandler(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS DriverCreateClose(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS DriverShutdown(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS DriverDispatchIoctl(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS SwitchMonitoringHandler(IN PIRP Irp);
PDriverVariables GetDeviceVariable();
NTSTATUS CreateProcessNotify(IN BOOLEAN isDisabled);
VOID UnloadControlProcHandler();
VOID WorkItem(IN PDEVICE_OBJECT DeviceObject, IN OPTIONAL PVOID Context);
BOOLEAN IsControlProc();
PCSTR GetProcessFileNameById(IN HANDLE handle);
VOID SwitchControlProcXState();
BOOLEAN IsParentIdManager();
VOID CopyActivateProc(IN PActivateHandlerProc pActivateInfo);
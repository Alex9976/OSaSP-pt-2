#pragma once

#include <iostream>
#include <stdio.h>
#include <tchar.h>
#include <Windows.h>
#include <psapi.h>
#include <queue>
#pragma comment(lib, "Psapi.lib") 
#pragma comment(lib, "Advapi32.lib")
#include "../Driver/GlobalVaribles.h"

#define FILE_NAME 256

extern SC_HANDLE SC_ManagerHandle;
extern SC_HANDLE SC_ServiceHandle;
extern HANDLE controlDriver;
extern WCHAR file_Name[FILE_NAME] = { 0 };
extern ActivateHandlerProc activateHandlerProc;

void initializeActivateHandler();
std::string getProcessNameByID(DWORD processID);
std::string getProcessNameByHandle(HANDLE hProcess);
void clearActivateHandler();
bool GetProcXAndY();
void initializeManager();
void closeService();
void initializeService();
bool startService();
SC_HANDLE createService();
DWORD WINAPI CreateProc(LPVOID state);
DWORD WINAPI CloseProc(LPVOID state);
void waitExit();
void closeControl();
void syncWithDriver();
#include <Windows.h>
#include <iostream>
#include <string>
#include <psapi.h>

extern "C" __declspec(dllimport) void __stdcall ReplaceString(DWORD pid, const char* srcString, const char* resString);
typedef void __stdcall TReplaceString(DWORD, const char*, const char*);

void ReplaceStringDynamic(DWORD PID, const char* srcStr, const char* resStr);
void InjectToProcess(DWORD procID, char srcStr[], char resStr[]);

struct DataToSend {
	DWORD PID;
	char src[255];
	char res[255];
};

int main()
{
	DWORD PID = GetCurrentProcessId();
	DWORD attachProcessPID = 0;
	bool isExit = false;


	char src[255] = { 0 };
	char res[255] = { 0 };

	while (!isExit)
	{
		std::cout << "1 - Static import\n2 - Dynamic import\n3 - Dll injection\n4 - Exit\n";

		char action;

		std::cin >> action;

		switch (action)
		{
		case '1':
			std::cout << "Source string: ";
			std::cin >> src;
			std::cout << "New string: ";
			std::cin >> res;
			std::cout << "String before change: " << src << std::endl;
			ReplaceString(PID, src, res);
			std::cout << "String after changes: " << src << std::endl;
			break;
		case '2':
			std::cout << "Source string: ";
			std::cin >> src;
			std::cout << "New string: ";
			std::cin >> res;

			std::cout << "String before change: " << src << std::endl;
			ReplaceStringDynamic(PID, src, res);
			std::cout << "String after changes: " << src << std::endl;
			break;
		case '3':
			std::cout << "Pid: ";
			std::cin >> attachProcessPID;
			std::cout << "Source string: ";
			std::cin >> src;
			std::cout << "New string: ";
			std::cin >> res;

			InjectToProcess(attachProcessPID, src, res);
			break;
		case '4':
			isExit = true;
			break;
		default:
			break;
		}
	}
}

void ReplaceStringDynamic(DWORD PID, const char* srcStr, const char* resStr)
{
	HMODULE hDll = LoadLibrary(L"StringReplacement.dll");

	if (hDll)
	{
		TReplaceString* lpReplaceString = (TReplaceString*)GetProcAddress(hDll, "ReplaceString");

		if (lpReplaceString != NULL)
		{
			lpReplaceString(PID, srcStr, resStr);
		}

		FreeLibrary(hDll);
	}
}

void InjectToProcess(DWORD procID, char srcStr[], char resStr[])
{

	HANDLE hProc = OpenProcess(PROCESS_VM_READ | PROCESS_VM_OPERATION | PROCESS_VM_WRITE |
		PROCESS_CREATE_THREAD | PROCESS_CREATE_PROCESS,
		FALSE, procID);

	DataToSend data = { 0 };
	data.PID = procID;
	strncpy(data.src, srcStr, 255);
	strncpy(data.res, resStr, 255);

	if (hProc)
	{
		LPVOID baseAddress = VirtualAllocEx(hProc, NULL, strlen("StringReplacement.dll") + 1,
			MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

		if (baseAddress)
		{

			LPVOID dataAddress = VirtualAllocEx(hProc, NULL, sizeof(data),
				MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

			if (dataAddress)
			{

				WriteProcessMemory(hProc, dataAddress, &data,
					sizeof(data), NULL);

				WriteProcessMemory(hProc, baseAddress, "StringReplacement.dll",
					strlen("StringReplacement.dll") + 1, NULL);

				DWORD threadId;

				HANDLE hThread = CreateRemoteThread(hProc, NULL, NULL, (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(L"Kernel32"), "LoadLibraryA"), baseAddress, NULL, &threadId);

				if (hThread == NULL)
					std::cout << "Error" << std::endl;
				else
					WaitForSingleObject(hThread, INFINITE);

				DWORD hLibModule = 0;
				GetExitCodeThread(hThread, &hLibModule);
				if (hLibModule == STILL_ACTIVE)
				{
					return;
				}

				CloseHandle(hThread);

				HMODULE hDll = LoadLibraryA("StringReplacement.dll");
				DWORD offset = (DWORD)GetProcAddress(hDll, "Inject") - (DWORD)hDll;

				hThread = CreateRemoteThread(hProc, NULL, NULL, (LPTHREAD_START_ROUTINE)(hLibModule + offset + 0x00007fff00000000), dataAddress, NULL, &threadId);
				
				if (hThread == NULL)
					std::cout << "Error" << std::endl;
				else
					WaitForSingleObject(hThread, INFINITE);

				CloseHandle(hThread);
			}
		}

		CloseHandle(hProc);

	}
}
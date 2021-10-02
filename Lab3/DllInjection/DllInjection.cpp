#include <Windows.h>
#include <iostream>
#include <string>

extern "C" __declspec(dllimport) void __stdcall ReplaceString(DWORD pid, const char* srcString, const char* resString);
typedef void __stdcall TReplaceString(DWORD, const char*, const char*);

void ReplaceStringDynamic(DWORD PID, const char* srcStr, const char* resStr);
void InjectToProcess(DWORD procID, char srcStr[], char resStr[]);

struct DataToSend {
	bool injectOnAttach;
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

	if (hProc)
	{
		LPVOID baseAddress = VirtualAllocEx(hProc, NULL, strlen("StringReplacement.dll") + 1,
			MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

		if (baseAddress)
		{
			TCHAR lpTempPathBuffer[MAX_PATH];

			GetTempPath(MAX_PATH, lpTempPathBuffer);

			char pathToFile[MAX_PATH];
			wcstombs(pathToFile, lpTempPathBuffer, wcslen(lpTempPathBuffer) + 1);
			;				FILE* fp;
			if ((fp = fopen(strcat(pathToFile, "dataToSend.txt"), "w")) != NULL)
			{
				fprintf(fp, "%d %s %s", true, srcStr, resStr);
				fclose(fp);
			}


			WriteProcessMemory(hProc, baseAddress, "StringReplacement.dll",
				strlen("StringReplacement.dll") + 1, NULL);

			DWORD threadId;

			HANDLE hThread = CreateRemoteThread(hProc, NULL, NULL,
				(LPTHREAD_START_ROUTINE)LoadLibraryA, (LPVOID)baseAddress, NULL, &threadId);

			if (hThread == NULL)
				std::cout << "Error" << std::endl;
			else
				WaitForSingleObject(hThread, INFINITE);
		}

		CloseHandle(hProc);
	}
}
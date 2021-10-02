#include "pch.h"
#include <Windows.h>
#include <vector>
#include "StringReplacer.hpp"

extern "C" __declspec(dllexport) void __stdcall ReplaceString(DWORD pid, const char* srcStr, const char* resStr)
{
	HANDLE hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION, FALSE, pid);

	if (hProcess)
	{
		SYSTEM_INFO systemInfo;
		GetSystemInfo(&systemInfo);

		MEMORY_BASIC_INFORMATION mbInfo;
		std::vector<char> chunk;
		char* p = 0;

		while (p < systemInfo.lpMaximumApplicationAddress)
		{
			if (VirtualQueryEx(hProcess, p, &mbInfo, sizeof(mbInfo)) == sizeof(mbInfo))
			{
				if (mbInfo.State == MEM_COMMIT && mbInfo.AllocationProtect == PAGE_READWRITE)
				{
					p = (char*)mbInfo.BaseAddress;

					chunk.resize(mbInfo.RegionSize);
					SIZE_T bytesRead;

					try
					{
						if (ReadProcessMemory(hProcess, p, &chunk[0], mbInfo.RegionSize, &bytesRead))
						{
							for (size_t i = 0; i < (bytesRead - strlen(srcStr)); ++i)
							{
								if (memcmp(srcStr, &chunk[i], strlen(srcStr)) == 0)
								{
									char* ref = (char*)p + i;

									for (int j = 0; j < strlen(resStr); j++)
										ref[j] = resStr[j];

									ref[strlen(resStr)] = 0;
								}
							}
						}
					}
					catch (std::bad_alloc& e)
					{

					}
				}

				p += mbInfo.RegionSize;
			}
		}
	}
}
#include <Windows.h>

struct DataToSend {
	DWORD PID;
	char src[40];
	char res[40];
};

extern "C" void __declspec(dllexport) __stdcall ReplaceString(DWORD pid, const char* srcStr, const char* resStr);

extern "C" __declspec(dllexport) void __stdcall Inject(DataToSend data);
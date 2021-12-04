#include <Windows.h>

extern "C" void __declspec(dllexport) __stdcall ReplaceString(DWORD pid, const char* srcStr, const char* resStr);
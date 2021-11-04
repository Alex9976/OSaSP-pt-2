#pragma once

#include <stdio.h>
#include <tchar.h>
#include <Windows.h>
#include <psapi.h>
#pragma comment(lib, "Psapi.lib") 
#pragma comment(lib, "Advapi32.lib")

class ThreadMutex
{
private:
	CRITICAL_SECTION m_mutex;

public:
	ThreadMutex();
	~ThreadMutex();
	void Lock();
	bool TryLock();
	void Unlock();
};


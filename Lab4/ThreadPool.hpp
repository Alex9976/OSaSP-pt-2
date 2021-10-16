#pragma once
#include <Windows.h>
#include <queue>
#include "Task.hpp"

class ThreadPool
{
public:
	ThreadPool(DWORD numberOfThreads);	
	~ThreadPool();
	void Add(Task* task);
	void Wait();
	void Wait(DWORD timeout);
private:
	CRITICAL_SECTION queueCS;
	std::queue<Task*> *taskQueue;
	const DWORD numberOfThreads;
	HANDLE* threadsList;
	CONDITION_VARIABLE spinWait;
	static DWORD WINAPI TaskListener(ThreadPool* instance);
	volatile LONG isRunning;
};


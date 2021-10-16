#include "ThreadPool.hpp"

ThreadPool::ThreadPool(DWORD numberOfThreads) : numberOfThreads(numberOfThreads), isRunning(true)
{
	taskQueue = new std::queue<Task*>();

	InitializeCriticalSection(&queueCS);
	InitializeConditionVariable(&spinWait);

	threadsList = new HANDLE[numberOfThreads];

	for (int i = 0; i < numberOfThreads; i++)
	{
		do
		{
			threadsList[i] = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)TaskListener, this, 0, NULL);
		} while (threadsList[i] == NULL);
	}
}

ThreadPool::~ThreadPool()
{
	if (isRunning)
	{
		Wait();
	}

	for (int i = 0; i < numberOfThreads; i++)
	{
		CloseHandle(threadsList[i]);
	}
	delete[] threadsList;
	delete taskQueue;
	DeleteCriticalSection(&queueCS);
}

void ThreadPool::Add(Task* task)
{
	EnterCriticalSection(&queueCS);
	taskQueue->push(task);
	WakeConditionVariable(&spinWait);
	LeaveCriticalSection(&queueCS);
}

void ThreadPool::Wait()
{
	Wait(INFINITE);
}

void ThreadPool::Wait(DWORD timeout)
{
	if (InterlockedCompareExchange(&isRunning, FALSE, TRUE))
	{
		EnterCriticalSection(&queueCS);
		for (int i = 0; i < numberOfThreads; i++)
		{
			taskQueue->push(NULL);
		}
		WakeAllConditionVariable(&spinWait);
		LeaveCriticalSection(&queueCS);
		WaitForMultipleObjects(numberOfThreads, threadsList, TRUE, timeout);
	}
}

DWORD WINAPI ThreadPool::TaskListener(ThreadPool* instance)
{
	Task* task;
	do
	{
		EnterCriticalSection(&instance->queueCS);
		while (instance->taskQueue->empty())
		{
			SleepConditionVariableCS(&instance->spinWait, &instance->queueCS, INFINITE);
		}
		task = instance->taskQueue->front();
		instance->taskQueue->pop();
		LeaveCriticalSection(&instance->queueCS);

		if (task != NULL)
		{
			task->GetTask()(task->GetArgument());
		}
	} while (task != NULL);

	return 0;
}


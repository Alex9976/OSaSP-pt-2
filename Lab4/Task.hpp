#pragma once
#include <Windows.h>

class Task
{
public:
	Task(LPTHREAD_START_ROUTINE Task, LPVOID Argument);
	LPTHREAD_START_ROUTINE GetTask();
	LPVOID GetArgument();
private:
	const LPTHREAD_START_ROUTINE task;
	const LPVOID argument;
};


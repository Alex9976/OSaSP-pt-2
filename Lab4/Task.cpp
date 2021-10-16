#include "Task.hpp"

Task::Task(LPTHREAD_START_ROUTINE Task, LPVOID Argument) : task(Task), argument(Argument)
{
}

LPTHREAD_START_ROUTINE Task::GetTask()
{
	return task;
}

LPVOID Task::GetArgument()
{
	return argument;
}
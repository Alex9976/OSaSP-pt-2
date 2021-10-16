#include "Sorter.hpp"
#include "ThreadPool.hpp"
#include "Task.hpp"

void Sorting(LPSTR* lines, DWORD linesCount, DWORD numberOfThreads);
void MergeLines(LPSTR* lines, DWORD left, DWORD right);

typedef struct _SORT_DATA
{
	LPSTR* strings;
	DWORD count;
} SORT_DATA;

void Sort(LPSTR* lines, DWORD linesCount, DWORD numberOfThreads)
{
	Sorting(lines, linesCount, numberOfThreads);
	MergeLines(lines, 0, linesCount - 1);
}

int QsortStringComparator(const void* lpString1, const void* lpString2)
{
	return strcmp(*((LPSTR*)lpString1), *((LPSTR*)lpString2));
}

DWORD WINAPI SortRoutine(SORT_DATA* srdArgument)
{
	qsort(srdArgument->strings, srdArgument->count, sizeof(LPSTR), QsortStringComparator);
	return 0;
}

void Sorting(LPSTR* lines, DWORD linesCount, DWORD numberOfThreads)
{
	ThreadPool* threadPool = new ThreadPool(numberOfThreads);

	DWORD stringsPerThread = linesCount / numberOfThreads + (linesCount % numberOfThreads == 0 ? 0 : 1);

	if (stringsPerThread > 0)
	{
		int stringsForThread;
		Task** task = (Task**)calloc(0, sizeof(Task*)), **taskBuffer;
		DWORD taskCount = 0;
		DWORD processedStrings = 0;
		SORT_DATA* sortData;

		for (int i = 0; i < numberOfThreads; i++)
		{
			stringsForThread = min(stringsPerThread, linesCount - processedStrings);
			if (stringsForThread > 0)
			{
				sortData = (SORT_DATA*)calloc(1, sizeof(SORT_DATA));
				sortData->count = stringsForThread;
				sortData->strings = lines + processedStrings;

				do
				{
					taskBuffer = (Task**)realloc(task, (taskCount + 1) * sizeof(Task*));
				} while (taskBuffer == NULL);
				task = taskBuffer;
				task[taskCount] = new Task((LPTHREAD_START_ROUTINE)SortRoutine, sortData);
				threadPool->Add(task[taskCount++]);
				processedStrings += stringsForThread;
			}
		}

		threadPool->Wait();

		for (int i = 0; i < taskCount; i++)
		{
			free(task[i]->GetArgument());
			delete task[i];
		}
		delete[] task;
	}

	delete threadPool;
}

void MergeLines(LPSTR* lines, DWORD left, DWORD right)
{
	if (left == right)
	{
		return;
	}

	DWORD middle = (right + left) / 2;
	MergeLines(lines, left, middle);
	MergeLines(lines, middle + 1, right);

	DWORD dwBufferLength = right - left + 1;
	LPSTR* buffer = (LPSTR*)calloc(dwBufferLength, sizeof(LPSTR));
	DWORD leftElement = left, rightElement = middle + 1;
	DWORD insertIndex = 0;

	while ((leftElement <= middle) && (rightElement <= right))
	{
		if (strcmp(lines[leftElement], lines[rightElement]) < 0)
		{
			buffer[insertIndex++] = lines[leftElement++];
		}
		else
		{
			buffer[insertIndex++] = lines[rightElement++];
		}
	}
	while (leftElement <= middle)
	{
		buffer[insertIndex++] = lines[leftElement++];
	}
	while (rightElement <= right)
	{
		buffer[insertIndex++] = lines[rightElement++];
	}

	for (DWORD dwCopyIndex = 0; dwCopyIndex < dwBufferLength; ++dwCopyIndex)
	{
		lines[left + dwCopyIndex] = buffer[dwCopyIndex];
	}
}
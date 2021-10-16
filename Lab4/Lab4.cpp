#include <iostream>
#include <Windows.h>
#include "Sorter.hpp"

LPSTR* GetLinesFromFile(LPWSTR filename, LPDWORD linesCount);

int main()
{
	std::string fileName;
    int numberOfThreads = 0;
    std::cout << "File name:\n";
    std::cin >> fileName;
    std::cout << "Number of threads:\n";
    std::cin >> numberOfThreads;
	DWORD linesCount = 0;
	wchar_t wcFileName[100] = { 0 };
	mbstowcs(wcFileName, fileName.c_str(), fileName.length());
	LPSTR* lines = GetLinesFromFile(wcFileName, &linesCount);
	Sort(lines, linesCount, numberOfThreads);

	for (int i = 0; i < linesCount; i++)
	{
		printf("%s\n", lines[i]);
	}
}

LPSTR* GetLinesFromFile(LPWSTR filename, LPDWORD linesCount)
{
	HANDLE hFile = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return NULL;
	}

	DWORD lpFileSizeHigh = GetFileSize(hFile, NULL);
	if (lpFileSizeHigh == INVALID_FILE_SIZE)
	{
		CloseHandle(hFile);
		return NULL;
	}

	LPSTR lpBuffer = (LPSTR)calloc(lpFileSizeHigh + 1, 1);
	if (!ReadFile(hFile, lpBuffer, lpFileSizeHigh, NULL, NULL))
	{
		CloseHandle(hFile);
		free(lpBuffer);
		return NULL;
	}
	CloseHandle(hFile);

	LPCSTR lpsDelimiters = "\r\n";
	LPSTR* lpsResult = (LPSTR*)calloc(0, sizeof(LPSTR)), * lpsBuffer;
	LPSTR lpsContext = 0;
	DWORD dwLinesCount = 0;
	LPSTR lpsToken = strtok_s(lpBuffer, lpsDelimiters, &lpsContext);

	while (lpsToken != NULL)
	{
		do
		{
			lpsBuffer = (LPSTR*)realloc(lpsResult, (dwLinesCount + 1) * sizeof(LPSTR));
		} while (lpsBuffer == NULL);
		lpsResult = lpsBuffer;
		lpsResult[dwLinesCount++] = lpsToken;
		lpsToken = strtok_s(NULL, lpsDelimiters, &lpsContext);
	}

	*linesCount = dwLinesCount;
	return lpsResult;
}

#include <Windows.h>
#include <string>
#include <fstream>
#include "AppWindow.hpp"

using namespace std;

HDC hdcBack;
HBITMAP hbmBack;
RECT rcClient;

const LPCSTR PATH_TO_FILE = "text.txt";
const int NUM_OF_COLUMNS = 6;
const int NUM_OF_ROWS = 4;
char textList[NUM_OF_COLUMNS * NUM_OF_ROWS][514];

void AppWindow_InitializeBackBuffer(HWND hwnd, int width, int height)
{
	HDC hdcWindow;
	hdcWindow = GetDC(hwnd);
	hdcBack = CreateCompatibleDC(hdcWindow);
	hbmBack = CreateCompatibleBitmap(hdcWindow, width, height);
	ReleaseDC(hwnd, hdcWindow);

	SaveDC(hdcBack);
	SelectObject(hdcBack, hbmBack);

}

void AppWindow_FinalizeBackBuffer()
{
	if (hdcBack)
	{
		RestoreDC(hdcBack, -1);
		DeleteObject(hbmBack);
		DeleteDC(hdcBack);
		hdcBack = 0;
	}
}

const wchar_t* GetWC(const char* c)
{
	const size_t cSize = strlen(c) + 1;
	wchar_t* wc = new wchar_t[cSize];
	mbstowcs(wc, c, cSize);

	return wc;
}

void AppWindow_draw(HDC hdc)
{
	HBRUSH hbrWhite;
	RECT rect;

	SaveDC(hdc);

	hbrWhite = CreateSolidBrush(RGB(255, 255, 255));

	SelectObject(hdc, hbrWhite);
	Rectangle(hdc, -1, -1, 4000, 4000);

	int windowWidth = rcClient.right - rcClient.left;
	int windowHeight = rcClient.bottom - rcClient.top;

	int columnWidth = windowWidth / NUM_OF_COLUMNS;
	int tableHeight = 0;

	for (int i = 0; i < NUM_OF_ROWS; i++)
	{
		int currentMaxColumnHeight = 0;

		for (int j = 0; j < NUM_OF_COLUMNS; j++)
		{
			int lengthOfDataItem = strlen(textList[NUM_OF_COLUMNS * i + j]);

			SetRect(&rect, j * columnWidth + 1, tableHeight + 1, (j + 1) * columnWidth - 1, windowHeight);
			int strHeight = DrawText(hdc, GetWC(textList[NUM_OF_COLUMNS * i + j]), lengthOfDataItem, &rect, DT_WORDBREAK);

			if (strHeight > currentMaxColumnHeight)
				currentMaxColumnHeight = strHeight;

		}

		tableHeight += currentMaxColumnHeight;

		MoveToEx(hdc, 0, tableHeight, NULL);
		LineTo(hdc, windowWidth, tableHeight);
	}

	for (int i = 1; i < NUM_OF_COLUMNS; i++)
	{
		int columnPosition = i * columnWidth;
		MoveToEx(hdc, columnPosition, 0, NULL);
		LineTo(hdc, columnPosition, tableHeight);
	}


	DeleteObject(hbrWhite);
	RestoreDC(hdc, -1);
}

bool LoadData()
{
	int requiredNumOfLines = NUM_OF_COLUMNS * NUM_OF_ROWS;
	FILE* fp;
	int i = 0;

	fopen_s(&fp, "text.txt", "r");
	
	if (fp == NULL)
	{
		return false;
	}

	while (!feof(fp) && i < requiredNumOfLines) {
		fgets(textList[i++], 512, fp);
	}
	fclose(fp);
	return true;
}


LRESULT CALLBACK AppWindow_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		if (!LoadData())
		{
			MessageBox(hwnd, L"Can't load text", L"Error", MB_OK | MB_ICONERROR);
		}
		break;
	case WM_DESTROY:
		AppWindow_FinalizeBackBuffer();
		PostQuitMessage(0);
		break;
	case WM_SIZE:
		GetClientRect(hwnd, &rcClient);
		AppWindow_FinalizeBackBuffer();
		AppWindow_InitializeBackBuffer(hwnd, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);
		InvalidateRect(hwnd, NULL, FALSE);
		break;
	case WM_PAINT:
		PAINTSTRUCT ps;
		AppWindow_draw(hdcBack);
		HDC hdc = BeginPaint(hwnd, &ps);
		BitBlt(hdc, 0, 0, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, hdcBack, 0, 0, SRCCOPY);
		EndPaint(hwnd, &ps);
		break;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

ATOM AppWindow_RegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = NULL;
	wcex.lpfnWndProc = AppWindow_WndProc;
	wcex.cbClsExtra = NULL;
	wcex.cbWndExtra = 4;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = 0;//(HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"Main Window";
	wcex.hIconSm = NULL;
	return RegisterClassEx(&wcex);
}
#include <Windows.h>
#include <fstream>
#include <iostream>
#include <string>
#include "AppWindow.hpp"

const wchar_t* const MainWindow_ClassName = L"Lab 2";
HWND hwndAppWindow;

LRESULT CALLBACK MainWindow_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RECT rect = { 0 };
	switch (uMsg)
	{
	case WM_SIZE:
	case WM_SIZING:
		GetWindowRect(hwnd, &rect);
		SetWindowPos(hwndAppWindow, (HWND)NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPervInstance, PSTR lpCmdLine, INT nCmdShow)
{
	WNDCLASSEX wcex;
	HWND hWnd, hWndProgress, hWndProgress2;
	MSG msg;
	ATOM atomAppWindow, atomElementWindow;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = NULL;
	wcex.lpfnWndProc = MainWindow_WndProc;
	wcex.cbClsExtra = NULL;
	wcex.cbWndExtra = NULL;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(0, IDI_WINLOGO);
	wcex.hCursor = LoadCursor(NULL, IDC_HAND);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = MainWindow_ClassName;
	wcex.hIconSm = 0;
	RegisterClassEx(&wcex);

	atomAppWindow = AppWindow_RegisterClass(hInstance);

	hWnd = CreateWindowEx(NULL, MainWindow_ClassName, MainWindow_ClassName, (WS_OVERLAPPEDWINDOW | WS_VISIBLE),
		0, 0, 1000, 600, NULL, NULL, hInstance, NULL);

	hwndAppWindow = CreateWindowEx(NULL, (LPCWSTR)atomAppWindow, NULL, WS_CHILD | WS_VISIBLE,
		0, 0, 1000, 600, hWnd, NULL, hInstance, NULL);

	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}
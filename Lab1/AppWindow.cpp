#include <Windows.h>
#include <windowsx.h>
#include "resource.h"
#include "AppWindow.hpp"
#pragma comment(lib, "Msimg32.lib")

HDC hdcBack;
HBITMAP hbmBack;
RECT rcClient;

HBITMAP sprite;
COORD spritePosition = { 0, 0 };
RECT spriteSpeed = { 0, 0, 0, 0 };
const int VK_W = 0x57;
const int VK_A = 0x41;
const int VK_S = 0x53;
const int VK_D = 0x44;
int SPRITE_ACCEL = 5;
int SPRITE_TIMER_ACCEL = 5;
bool isUP = false, isLeft = false, isDown = false, isRight = false;

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

void AppWindow_PrintBitmap(HDC hdc)
{
	HBITMAP hOldbm;
	HDC hMemDC;
	BITMAP bm;
	POINT  ptSize, ptOrg;

	hMemDC = CreateCompatibleDC(hdc);

	hOldbm = (HBITMAP)SelectObject(hMemDC, sprite);

	if (hOldbm)
	{
		SetMapMode(hMemDC, GetMapMode(hdc));
		GetObject(sprite, sizeof(BITMAP), (LPSTR)&bm);

		ptSize.x = bm.bmWidth;
		ptSize.y = bm.bmHeight;

		DPtoLP(hdc, &ptSize, 1);
		ptOrg.x = 0;
		ptOrg.y = 0;
		DPtoLP(hMemDC, &ptOrg, 1);

		TransparentBlt(hdc, spritePosition.X, spritePosition.Y, ptSize.x, ptSize.y, hMemDC, 0, 0, ptSize.x, ptSize.y, RGB(255, 255, 255));
		SelectObject(hMemDC, hOldbm);
	}
	DeleteDC(hMemDC);
}

void AppWindow_draw(HDC hdc)
{
	HBRUSH hbrSkyBlue;

	SaveDC(hdc);

	hbrSkyBlue = CreateSolidBrush(RGB(2, 204, 254));

	SelectObject(hdc, hbrSkyBlue);
	Rectangle(hdc, -1, -1, 4000, 4000);
	AppWindow_PrintBitmap(hdc);

	DeleteObject(hbrSkyBlue);
	RestoreDC(hdc, -1);
}

void swap(LONG* a, LONG* b) {
	LONG temp = *a;
	*a = *b;
	*b = temp;
}

void UpdateSpritePosition(RECT windowRectSize, SIZE spriteSize)
{
	COORD newSpritePosition;

	SIZE windowSize;
	windowSize.cx = windowRectSize.right - windowRectSize.left;
	windowSize.cy = windowRectSize.bottom - windowRectSize.top;

	COORD spriteSteps = { 0 };

	spriteSteps.Y = -(spriteSpeed.top - spriteSpeed.bottom);
    spriteSteps.X = -(spriteSpeed.left - spriteSpeed.right);


	newSpritePosition.X = spritePosition.X + spriteSteps.X;
	if (newSpritePosition.X < 0)
	{
		swap(&spriteSpeed.left, &spriteSpeed.right);
		spriteSpeed.left = spriteSteps.X;
		newSpritePosition.X = 0 + spriteSteps.X;
	}
	else if (newSpritePosition.X + spriteSize.cx > windowSize.cx)
	{
		swap(&spriteSpeed.left, &spriteSpeed.right);
		newSpritePosition.X = (SHORT)(windowSize.cx - spriteSize.cx - spriteSteps.X);
	}

	newSpritePosition.Y = spritePosition.Y + spriteSteps.Y;
	if (newSpritePosition.Y < 0)
	{
		swap(&spriteSpeed.top, &spriteSpeed.bottom);
		newSpritePosition.Y = 0 + spriteSteps.X;
	}
	else if (newSpritePosition.Y + spriteSize.cy > windowSize.cy)
	{
		swap(&spriteSpeed.top, &spriteSpeed.bottom);
		newSpritePosition.Y = (SHORT)(windowSize.cy - spriteSize.cy - spriteSteps.X);
	}

	spritePosition = newSpritePosition;
}

void DoUp()
{
	spriteSpeed.top += SPRITE_ACCEL;
}

void DoRight()
{
	spriteSpeed.right += SPRITE_ACCEL;
}

void DoDown()
{
	spriteSpeed.bottom += SPRITE_ACCEL;
}

void DoLeft()
{
	spriteSpeed.left += SPRITE_ACCEL;
}

SIZE GetSpriteSize(HBITMAP hBitmap)
{
	BITMAP sprite;
	GetObject(hBitmap, sizeof(BITMAP), &sprite);
	SIZE spriteSize;
	spriteSize.cx = sprite.bmWidth;
	spriteSize.cy = sprite.bmHeight;
	return spriteSize;
}


LRESULT CALLBACK AppWindow_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int x, y;
	RECT rect = { 0 };
	GetClientRect(hwnd, &rect);

	switch (uMsg)
	{
	case AW_LOADBITMAP:
		sprite = LoadBitmap((HINSTANCE)lParam, MAKEINTRESOURCE(IDB_BITMAP1));
		break;
	case WM_DESTROY:
		AppWindow_FinalizeBackBuffer();
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
		if (wParam == VK_UP || wParam == VK_W || isUP)
		{
			DoUp();
			isUP = true;
		}
		if (wParam == VK_RIGHT || wParam == VK_D || isRight)
		{
			DoRight();
			isRight = true;
		}
		if (wParam == VK_DOWN || wParam == VK_S || isDown)
		{
			DoDown();
			isDown = true;
		}
		if (wParam == VK_LEFT || wParam == VK_A || isLeft)
		{
			DoLeft();
			isLeft = true;
		}
		UpdateSpritePosition(rect, GetSpriteSize(sprite));
		InvalidateRect(hwnd, NULL, FALSE);
		break;
	case WM_KEYUP:
		if (wParam == VK_UP || wParam == VK_W)
		{
			isUP = false;
		}
		if (wParam == VK_RIGHT || wParam == VK_D)
		{
			isRight = false;
		}
		if (wParam == VK_DOWN || wParam == VK_S)
		{
			isDown = false;
		}
		if (wParam == VK_LEFT || wParam == VK_A)
		{
			isLeft = false;
		}
		break;
	case WM_TIMER:
		if (spriteSpeed.top - SPRITE_TIMER_ACCEL > 0)
		{
			spriteSpeed.top -= SPRITE_TIMER_ACCEL;
		}
		else
		{
			spriteSpeed.top = 0;
		}

		if (spriteSpeed.bottom - SPRITE_TIMER_ACCEL > 0)
		{
			spriteSpeed.bottom -= SPRITE_TIMER_ACCEL;
		}
		else
		{
			spriteSpeed.bottom = 0;
		}

		if (spriteSpeed.left - SPRITE_TIMER_ACCEL > 0)
		{
			spriteSpeed.left -= SPRITE_TIMER_ACCEL;
		}
		else
		{
			spriteSpeed.left = 0;
		}

		if (spriteSpeed.right - SPRITE_TIMER_ACCEL > 0)
		{
			spriteSpeed.right -= SPRITE_TIMER_ACCEL;
		}
		else
		{
			spriteSpeed.right = 0;
		}
		UpdateSpritePosition(rect, GetSpriteSize(sprite));
		InvalidateRect(hwnd, NULL, FALSE);
		break;
	case WM_MOUSEWHEEL:
		if (GET_KEYSTATE_WPARAM(wParam) == MK_SHIFT)
		{
			if (GET_WHEEL_DELTA_WPARAM(wParam) > 0)
			{
				DoLeft();
			}
			else
			{
				DoRight();
			}
		}
		else
		{
			if (GET_WHEEL_DELTA_WPARAM(wParam) > 0)
			{
				DoUp();
			}
			else
			{
				DoDown();
			}
		}
		UpdateSpritePosition(rect, GetSpriteSize(sprite));
		InvalidateRect(hwnd, NULL, FALSE);
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
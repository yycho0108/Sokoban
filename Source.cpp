#ifdef Src1

#include <Windows.h>
#include "resource.h"
#include <functional>

void CALLBACK PlayerMove(HWND, UINT, UINT, DWORD);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void DrawBitmap(HDC& hdc, HBITMAP& src, int x, int y);


HINSTANCE g_hInst;
HWND hMainWnd;
HBITMAP Player[4], BOX;
LPCTSTR Title = L"SOKOBAN";
HPEN curPen, oldPen;

class User
{
private:
	int x, y, dx, dy; //left-up-right-down;

	friend void CALLBACK inertia(HWND, UINT, UINT, DWORD);
public:
	class StaticImage;
	const static StaticImage *SIM;
	char dir[4];
	char face;
	inline void move();
	User(int x, int y) :x{ x }, y{ y }, dx{ 0 }, dy{ 0 }
	{
		for (int i = 0; i < 4; i++)
		{
			dir[i] = false;
		}
	}
};
class User::StaticImage
{
private:
	friend void CALLBACK inertia(HWND, UINT, UINT, DWORD);
	HBITMAP image[4];
public:
	StaticImage()
	{
		for (int i = 0; i < 4; i++)
		{
			image[i] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP2 + i));
		}
	}
	~StaticImage()
	{
		for (auto &x : image) DeleteObject(x);
	}
};
const User::StaticImage* User::SIM = NULL;
class Box
{
private:
	int x, y;
	static HBITMAP image;
};
User* Self;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	g_hInst = hInstance;
	WNDCLASS WndClass;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = Title;
	WndClass.lpszMenuName = NULL;
	WndClass.style = CS_VREDRAW | CS_HREDRAW;

	RegisterClass(&WndClass);

	hMainWnd = CreateWindow(Title, Title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1200,1200, NULL, NULL, hInstance, NULL);
	ShowWindow(hMainWnd, nCmdShow);

	MSG msg;
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch (iMsg)
	{
	case WM_CREATE:
	{
		Self = new User(0, 0);
		User::SIM = new User::StaticImage();
		SetTimer(hWnd,(UINT_PTR)Self, 50, inertia);
		BOX = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP1));
		curPen = (HPEN)GetStockObject(NULL_PEN);
		break;
	}
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_LEFT:
		case VK_UP:
		case VK_RIGHT:
		case VK_DOWN:
			Self->dir[wParam - VK_LEFT] = true;
			Self->face = wParam - VK_LEFT;
		}
		break;
	}
	case WM_KEYUP:
	{
		switch (wParam)
		{
		case VK_LEFT:
		case VK_UP:
		case VK_RIGHT:
		case VK_DOWN:
			Self->dir[wParam - VK_LEFT] = false;
			for (int i = 0; i < 4; i++)
			{
				if (Self->dir[i])
				{
					Self->face = i;
					break;
				}
			}
		}
		break;
	}
	case WM_DESTROY:
	{
		delete Self;
		delete User::SIM;
		HDC hdc = GetDC(hWnd);
		SelectObject(hdc, oldPen);
		DeleteObject(curPen);
		DeleteObject(BOX);
		PostQuitMessage(0);
		break;
	}
	default:
		return DefWindowProc(hWnd, iMsg, wParam, lParam);
	}
	return 0;
}

void DrawBitmap(HDC& hdc, const HBITMAP& src, int x, int y)
{
	HDC MemDC;
	HBITMAP OldBit;
	MemDC = CreateCompatibleDC(hdc);
	OldBit = (HBITMAP)SelectObject(MemDC, src);
	BitBlt(hdc, x, y, 48, 48, MemDC, 0, 0, SRCCOPY); //KNOWING SIZE IS 48... cheat
	SelectObject(MemDC, OldBit);
	DeleteObject(MemDC);
}

void CALLBACK inertia(HWND hWnd, UINT iMSG, UINT_PTR CallerId, DWORD dwTime)
{
	((User*)CallerId)->move();
	HDC hdc = GetDC(hMainWnd);
	static int cnt = 2;
	if (1)
	{
		cnt = 2;
		if (((User*)CallerId)->dx > 0)
		{
			--((User*)CallerId)->dx;
			if (((User*)CallerId)->dx > 30)
				((User*)CallerId)->dx = 30;
		}
		else if (((User*)CallerId)->dx < 0)
		{
			++((User*)CallerId)->dx;
			if (((User*)CallerId)->dx < -30)
				((User*)CallerId)->dx = -30;
		}

		if (((User*)CallerId)->dy > 0)
		{
			--((User*)CallerId)->dy;
			if (((User*)CallerId)->dy > 30)
				((User*)CallerId)->dy = 30;
		}
		else if (((User*)CallerId)->dy < 0)
		{
			++((User*)CallerId)->dy;
			if (((User*)CallerId)->dy < -30)
				((User*)CallerId)->dy = -30;
		}
	}


	if (((User*)CallerId)->x < 0)
	{
		((User*)CallerId)->x = 0;
		((User*)CallerId)->dx = abs(((User*)CallerId)->dx);
	}
	else if (((User*)CallerId)->x > 800)
	{
		((User*)CallerId)->x = 800;
		((User*)CallerId)->dx = -abs(((User*)CallerId)->dx);
	}
	if (((User*)CallerId)->y < 0)
	{
		((User*)CallerId)->y = 0;
		((User*)CallerId)->dy = abs(((User*)CallerId)->dy);
	}
	else if (((User*)CallerId)->y>800)
	{
		((User*)CallerId)->y = 800;
		((User*)CallerId)->dy = -abs(((User*)CallerId)->dy);
	}
	//RECT R = { ((User*)CallerId)->x, ((User*)CallerId)->y, ((User*)CallerId)->x + 48, ((User*)CallerId)->y + 48 };

	oldPen = (HPEN)SelectObject(hdc, curPen);

	Rectangle(hdc, ((User*)CallerId)->x-3, ((User*)CallerId)->y-3, ((User*)CallerId)->x + 51, ((User*)CallerId)->y + 51);
	DrawBitmap(hdc, ((User*)CallerId)->SIM->image[((User*)CallerId)->face], ((User*)CallerId)->x += ((User*)CallerId)->dx, ((User*)CallerId)->y += ((User*)CallerId)->dy);
	
	ReleaseDC(hMainWnd, hdc);
}

inline void User::move()
{
	if (dir[0]) dx -= 3;
	if (dir[1]) dy -= 3;
	if (dir[2]) dx += 3;
	if (dir[3]) dy += 3;
}

#endif
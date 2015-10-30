#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include "resource.h"
#include <string>
#include <vector>
#include <fstream>
#include <list>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

// left, up, right, down --
//empty, box, wall

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void DrawBitmap(HDC& hdc, HBITMAP& src, int x, int y);
void LoadStage(int);
void DrawStage(HDC& hdc);
void Move(WPARAM);
void Undo(WPARAM, bool);
void Pull(WPARAM);
void TestEnd();

HINSTANCE g_hInst;
HWND hMainWnd;
HBITMAP Player[4];
HBITMAP BOX;
HBITMAP WALL;
HBITMAP GOAL;
HBITMAP AGOAL;
LPCTSTR Title = L"Sokoban";
std::vector<std::string> CurStage;
int x = -1, y = -1;
char face;
struct tag_moveInfo
{
	std::list<WPARAM> moves;
	std::list<bool> withpack;
} moveInfo;

std::list<WPARAM> moves;

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

	hMainWnd = CreateWindow(Title, Title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1200, 1200, NULL, NULL, hInstance, NULL);
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
	HDC hdc;	
	PAINTSTRUCT	ps;
	static bool shift;

	switch (iMsg)
	{
	case WM_CREATE:
	{
		for (int i = 0; i < 4; i++)
		{
			Player[i] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP2 + i));
		}
		BOX = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP1));
		WALL = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP6));
		GOAL = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP7));
		AGOAL = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP8));
		LoadStage(1);
		break;
	}
	case WM_KEYDOWN:
	{
		if (wParam != 0x5A) Move(wParam);
		else
		{
			if (!moveInfo.moves.empty())
			{
				Undo(moveInfo.moves.back(), moveInfo.withpack.back());
				moveInfo.moves.pop_back();
				moveInfo.withpack.pop_back();
			}
		}

	}
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		DrawStage(hdc);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		for (int i = 0; i < 4; i++)
		{
			DeleteObject(Player[i]);
		}
		DeleteObject(BOX);
		DeleteObject(WALL);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, iMsg, wParam, lParam);
	}
	return 0;
}

void DrawBitmap(HDC& hdc, HBITMAP& src, int x, int y)
{
	HDC MemDC = CreateCompatibleDC(hdc);
	HBITMAP oldBitmap = (HBITMAP)SelectObject(MemDC, src);
	BITMAP Bit;
	GetObject(src, sizeof(BITMAP), &Bit);
	BitBlt(hdc, x*48, y*48, Bit.bmWidth, Bit.bmHeight, MemDC, 0, 0, SRCCOPY);
	SelectObject(MemDC, oldBitmap);
	DeleteDC(MemDC);
}

void LoadStage(int stage)
{
	TCHAR num[22];
	TCHAR DIR[256];

	std::string stringbuf;
	wsprintf(num, L"\\stage\\stage%02d.txt", stage);

	GetCurrentDirectory(256, DIR);
	lstrcat(DIR, num);

	std::ifstream fsrc{DIR};

	while (std::getline(fsrc, stringbuf))
	{
		CurStage.push_back(stringbuf);
	}
	x = -1;
	y = -1; //indicates "reset";
}

void DrawStage(HDC& hdc)
{

	bool End = true;
	for (int i = 0; i < CurStage.size(); i++)
	{
		for (int j = 0; j < CurStage[i].size(); j++)
		{

			switch (CurStage[i][j])
			{
			case '1':DrawBitmap(hdc, BOX, j, i); End = false; break;
			case '2':DrawBitmap(hdc, WALL, j, i); break;
			case '7':End = false;
			case '3':
			case '8':
				DrawBitmap(hdc, Player[face], j,i);
				if (!(~x || ~y)){ x = j, y = i; }
				break;
			case '4':DrawBitmap(hdc, GOAL, j, i); End = false;
				break;
			case '5': DrawBitmap(hdc, AGOAL, j, i);
			}
		}
	}
	if(End) PostQuitMessage(0);
}

inline bool getDir(WPARAM& wParam, char& dx, char& dy)
{
	dx = 0, dy = 0;
	switch (wParam)
	{
	case VK_LEFT: dx = -1; return true;
	case VK_UP: dy = -1; return true;
	case VK_RIGHT: dx = 1; return true;
	case VK_DOWN: dy = 1; return true;
	default: return FALSE;
	}
}
void Move(WPARAM wParam)
{

	char dx, dy;
	if (!getDir(wParam, dx, dy)) return;

	face = wParam - VK_LEFT;
	switch (CurStage[y + dy][x + dx])
	{
	case '0'://empty // 3 = player...
	case '4':
	{
		moveInfo.moves.push_back(wParam);
		moveInfo.withpack.push_back(false);

		CurStage[y][x] -= 3;
		CurStage[y += dy][x += dx] += 3;
		RECT R{ 48 * min(x, x - dx), 48 * min(y, y - dy), 48 * (max(x, x - dx) + 1), 48 * (max(y, y - dy) + 1) };
		InvalidateRect(hMainWnd, &R, TRUE);
		break;
	}
	case '1'://Box
	case '5':
		if (CurStage[y + dy + dy][x + dx + dx] == '0' || CurStage[y + dy + dy][x + dx + dx] == '4') // 5 = ACTIVATED GOAL // PLAYER - BOX/AG - EMPTY/G
		{
			//PlaySound((LPCTSTR)SND_ALIAS_SYSTEMASTERISK, NULL, SND_ALIAS_ID);
			moveInfo.moves.push_back(wParam); //successful
			moveInfo.withpack.push_back(true);

			++CurStage[y + dy + dy][x + dx + dx];
			--CurStage[y += dy][x += dx]; //undo box
			CurStage[y][x] += 3; //move player on
			CurStage[y - dy][x - dx] -= 3; //undo player
			RECT R{ 48 * min(min(x, x + dx), x - dx), 48 * min(min(y, y + dy), y - dy), 48 * (max(max(x, x + dx), x - dx) + 1), 48 * (max(max(y, y + dy), y - dy) + 1) };
			InvalidateRect(hMainWnd, &R, TRUE);
			break;
		}//else go down...
	case '2': // Wall
		RECT R{ 48 * x, 48 * y, 48 * (x + 1), 48 * (y + 1) };
		InvalidateRect(hMainWnd, &R, TRUE);
		break;
	}
}
void Pull(WPARAM wParam)
{
	char dx, dy;

	if (!getDir(wParam, dx, dy)) return; //dx,dy teaches you the dir to go!
	if (CurStage[y - dy][x - dx] != '1' || CurStage[y - dy][x - dx] != '5') return; //no box to pull

	switch (CurStage[y + dy][x + dx])
	{
	case '0'://empty // 3 = player...
	case '4': // 4 = GOAL
	{
		--CurStage[y - dy][x - dx];
		CurStage[y][x] += 1 - 3;
		CurStage[y += dy][x += dx] += 3;
		RECT R{ 48 * min(x, x - dx - dx), 48 * min(y, y - dy - dy), 48 * (max(x, x - dx - dx) + 1), 48 * (max(y, y - dy - dy) + 1) };
		InvalidateRect(hMainWnd, &R, TRUE);
	}
	break;
	case '2': // 2 = wall
	case '1':
	case '5':
	{
		return;
	}
	}
}
void Undo(WPARAM wParam, bool withpack)
{
	face = wParam - VK_LEFT;
	char dx, dy;
	if (!getDir(wParam, dx, dy)) return;

	switch (CurStage[y + dy][x + dx])
	{
	case '0'://empty // 3 = player...
	case '4': // 4 = GOAL
	case '2': // 2 = wall
	{
		CurStage[y][x] -= 3;
		CurStage[y -= dy][x -= dx] += 3;
		RECT R{ 48 * min(x, x +dx), 48 * min(y, y + dy), 48 * (max(x, x + dx) + 1), 48 * (max(y, y +dy) + 1) };
		InvalidateRect(hMainWnd, &R, TRUE);
		break;
	}
	case '1'://Box
	case '5': //Activated Goal
	{
		if (withpack)
		{
			--CurStage[y + dy][x + dx]; // remove box
			CurStage[y][x] += -3 + 1; //remove player, add box
			CurStage[y -= dy][x -= dx] += 3; //move player on
		}
		else
		{
			CurStage[y][x] -= 3;
			CurStage[y -= dy][x -= dx] += 3;
		}
		RECT R{ 48 * min(min(x, x + dx), x + dx + dx), 48 * min(min(y, y + dy), y + dy + dy), 48 * (max(max(x, x + dx), x + dx + dx) + 1), 48 * (max(max(y, y + dy), y + dy + dy) + 1) };
		InvalidateRect(hMainWnd, NULL, TRUE);
		break;
	}
	}
}
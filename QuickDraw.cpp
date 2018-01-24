#include "QuickDraw.h"

#include <iostream>
#include <process.h>

#include <windowsx.h>

QuickDraw * currentQD;

bool QuickDrawReady = false;

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	//	PAINTSTRUCT ps;
	//	HDC hdc;

	//	std::cout << "win\n";
	switch (message)
	{
	case WM_KEYDOWN:
		currentQD->keypress = wParam;
		currentQD->SetKeyDown(wParam);

		break;
	case WM_KEYUP:
		currentQD->keypress = 0;
		currentQD->SetKeyUp(wParam);

		break;
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		currentQD->mousebutton = message;
		currentQD->mouseset = true;
		currentQD->mousex = GET_X_LPARAM(lParam); 
        currentQD->mousey = GET_Y_LPARAM(lParam); 
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		break;
	case WM_PAINT:
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void WindowProc (void * obj)
{
	QuickDraw * qd = (QuickDraw *) obj;

	HINSTANCE hInstance = 0;

	char t[500];
	GetConsoleTitleA (t, 500);     // retrieve the text of the title bar of the console window
	HWND hwndConsole = FindWindowA (NULL, t);  // FindWindowA actually
	hInstance = (HINSTANCE)GetWindowLong(hwndConsole, GWL_HINSTANCE);

	WNDCLASS wc = {0};
	wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wc.hCursor = LoadCursor (hInstance, IDC_ARROW);
	wc.hIcon = LoadIcon (hInstance, IDI_APPLICATION);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = TEXT("QuickDraw");
	wc.style = CS_HREDRAW | CS_VREDRAW;

	if (!RegisterClass (&wc))
	{
		std::cout << "Problem with WNDCLASS, QuickDraw." << std::endl;
		return;   // ERR, SO QUIT
	}

	qd->hWnd = CreateWindow (L"QuickDraw", L"QuickDraw", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	ShowWindow (qd->hWnd, SW_SHOWNORMAL);    // ShowWindow() on msdn
	UpdateWindow (qd->hWnd);

	qd->hdc = BeginPaint(qd->hWnd, &(qd->ps));
	qd->hdcFront = CreateCompatibleDC (qd->hdc); 
	RECT windowRect;
	GetClientRect(qd->hWnd, &windowRect); 
	qd->frontBuffer = CreateCompatibleBitmap (qd->hdc, windowRect.right, windowRect.bottom);
	SelectObject (qd->hdcFront, qd->frontBuffer); 

	MSG msg;

	QuickDrawReady = true;

	while (GetMessage(&msg, NULL, 0, 0))
	{
		//	std::cout << "Bounce\n";

		//		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

QuickDraw::QuickDraw(void)
{
	currentQD = this;
	
	memset(&m_keyboard, false, sizeof(bool)*NUM_KEYS);
	memset(&m_mouse, false, sizeof(bool)*NUM_MOUSE_BUTTONS);

	_beginthread (WindowProc, 0, this);

	while (!QuickDrawReady)
	{
	  Sleep (50);
	}
}


QuickDraw::~QuickDraw(void)
{
}

void QuickDraw::SetKeyDown(char ch)
{
	//std::unique_lock<std::mutex>	locked(m_key_lock);
	m_keyboard[ch] = true;
}

void QuickDraw::SetKeyUp(char ch)
{
	//std::unique_lock<std::mutex>	locked(m_key_lock);
	m_keyboard[ch] = false;
}

bool QuickDraw::isActive(char ch)
{
	//std::unique_lock<std::mutex>	locked(m_key_lock);
	return m_keyboard[ch];
}

void QuickDraw::clearScreen (int r, int g, int b)

{
	HBRUSH NewBrush = CreateSolidBrush(RGB(r, g, b));
	SelectObject(hdcFront, NewBrush); 
	HPEN NewPen = CreatePen (PS_SOLID, 1, RGB(r, g, b));
	SelectObject(hdcFront, NewPen); 

	RECT windowRect;
	GetClientRect(hWnd, &windowRect); 
	FillRect (hdcFront, &windowRect, NewBrush);

	DeleteObject(NewPen);
	DeleteObject(NewBrush);
}

void QuickDraw::drawLine (int x1, int y1, int x2, int y2, int r, int g, int b)

{
	HPEN NewPen = CreatePen (PS_SOLID, 1, RGB(r, g, b));

	SelectObject(hdcFront, NewPen); 
	MoveToEx (hdcFront, x1, y1, NULL);
	LineTo (hdcFront, x2, y2);

	DeleteObject(NewPen);
}

void QuickDraw::drawSolidBox (int x1, int y1, int x2, int y2, int r, int g, int b)
{
	HBRUSH NewBrush = CreateSolidBrush(RGB(r, g, b));

	RECT rect;
	rect.left = x1;
	rect.right = x2;
	rect.top = y1;
	rect.bottom = y2;
	FillRect (hdcFront, &rect, NewBrush);

	DeleteObject(NewBrush);
}

void QuickDraw::drawCircle (int x1, int y1, int radius, int r, int g, int b)
{
	HPEN NewPen = CreatePen (PS_SOLID, 1, RGB(r, g, b));

	SelectObject(hdcFront, (HBRUSH)GetStockObject(HOLLOW_BRUSH));
	SelectObject(hdcFront, NewPen); 
	Ellipse (hdcFront, x1 - radius, y1 - radius, x1 + radius, y1 + radius);

	DeleteObject(NewPen);
}

void QuickDraw::drawSolidCircle (int x1, int y1, int radius, int r, int g, int b)
{
	HBRUSH NewBrush = CreateSolidBrush(RGB(r, g, b));
	SelectObject(hdcFront, NewBrush); 
	HPEN NewPen = CreatePen (PS_SOLID, 1, RGB(r, g, b));
	SelectObject(hdcFront, NewPen); 
	Ellipse (hdcFront, x1 - radius, y1 - radius, x1 + radius, y1 + radius);
	DeleteObject(NewPen);
	DeleteObject(NewBrush);
}

void QuickDraw::drawText (int x1, int y1, std::string message, int r, int g, int b)
{
	HBRUSH NewBrush = CreateSolidBrush(RGB(r, g, b));
	SelectObject(hdcFront, NewBrush); 
	HPEN NewPen = CreatePen (PS_SOLID, 1, RGB(r, g, b));
	SelectObject(hdcFront, NewPen); 

	RECT rect;
	rect.left = x1;
	rect.right = x1 + 100;
	rect.top = y1;
	rect.bottom = y1 + 100;

	SetRect(&rect, x1, y1, x1+100, y1+100);
	SetTextColor(hdcFront, RGB(r, g, b));

	std::wstring ws;
	ws.assign (message.begin (), message.end ());
	DrawText (hdcFront, ws.c_str (), -1, &rect, DT_LEFT | DT_TOP | DT_NOCLIP);

	DeleteObject(NewPen);
	DeleteObject(NewBrush);
}

void QuickDraw::swapBuffer ()

{
	RECT windowRect;
	GetClientRect(hWnd, &windowRect); 
	BitBlt (hdc, 0, 0, windowRect.right, windowRect.bottom, hdcFront, 0, 0, SRCCOPY);
}

void QuickDraw::screenSize (int & x, int & y)
{
	RECT windowRect;
	GetClientRect(hWnd, &windowRect); 
	x = windowRect.right - windowRect.left;
	y = windowRect.bottom - windowRect.top;
}

int QuickDraw::lastKey ()

{
	int v = keypress;
	return v;
}

void QuickDraw::lastMouse (int & x, int & y, int & button, bool & set)

{
  x = mousex;
  y = mousey;
  button = mousebutton;
  set = mouseset;
  mouseset = false;
}

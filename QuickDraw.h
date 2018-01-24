#pragma once

#include <Windows.h>

#include <string>

/// View: an abstraction of a view class, capable of updating the display.
class View
{
public:
	virtual void clearScreen (int r = 255, int g = 255, int b = 255) = 0;
	virtual void drawLine (int x1, int y1, int x2, int y2, int r = 0, int g = 0, int b = 0) = 0;
	virtual void drawSolidBox (int x1, int y1, int x2, int y2, int r = 0, int g = 0, int b = 0) = 0;
	virtual void drawCircle (int x1, int y1, int radius, int r = 0, int g = 0, int b = 0) = 0;
	virtual void drawSolidCircle (int x1, int y1, int radius, int r = 0, int g = 0, int b = 0) = 0;
	virtual void drawText (int x1, int y1, std::string message, int r = 0, int g = 0, int b = 0) = 0;
	virtual void swapBuffer () = 0;
	virtual void screenSize (int & x, int & y) = 0;
};

/// Controller: abstraction of the controller, recieves input from the user.
class Controller
{
public:
	virtual int lastKey () = 0;
	virtual void lastMouse (int & x, int & y, int & button, bool & set) = 0;
	virtual bool isActive(char ch) = 0;

};



typedef enum _mouse_buttons
{
	LBUTTON = 0,
	RBUTTON,
	MBUTTON
} MOUSE_BUTTON;

const size_t	NUM_MOUSE_BUTTONS = 3;
const size_t	NUM_KEYS = 256;

// type aliases for input maps
using key_map = bool[NUM_KEYS];
using mouse_map = bool[NUM_MOUSE_BUTTONS];


/// QuickDraw: some easy input/output with abstraction over the details of the systems graphics and
/// I/O facilities.
class QuickDraw : public View, Controller
{
friend void WindowProc (void * obj);

private:
	HINSTANCE hInst;								// current instance
    HWND hWnd;

	HBITMAP   frontBuffer;                          // Buffers for drawing.
    HDC hdc;
    HDC hdcFront;
	PAINTSTRUCT ps;

public:
	QuickDraw(void);
	virtual ~QuickDraw(void);

	int keypress;
	bool mouseset;
	int mousex;
	int mousey;
	int mousebutton;

	mouse_map		m_mouse;
	key_map			m_keyboard;

	void SetKeyDown(char ch);
	void SetKeyUp(char ch);
	bool isActive(char ch);


	void clearScreen (int r = 255, int g = 255, int b = 255);
	void drawLine (int x1, int y1, int x2, int y2, int r = 0, int g = 0, int b = 0);
	void drawSolidBox (int x1, int y1, int x2, int y2, int r = 0, int g = 0, int b = 0);
	void drawCircle (int x1, int y1, int radius, int r = 0, int g = 0, int b = 0);
	void drawSolidCircle (int x1, int y1, int radius, int r = 0, int g = 0, int b = 0);
	void drawText (int x1, int y1, std::string message, int r = 0, int g = 0, int b = 0);
	void swapBuffer ();
	void screenSize (int & x, int & y);

	int lastKey ();
	void lastMouse (int & x, int & y, int & button, bool & set);
};

void WindowProc (void * obj);


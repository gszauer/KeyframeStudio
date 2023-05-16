#include <windows.h>
#include <windowsx.h>
#include <ShellScalingAPI.h>
#include <iostream>
#include "../window.h"
#include "../memory.h"
#include "../graphics.h"
#include "../math.h"
#include "../../debt/stb_sprintf.h"
#include "../assert.h"
#include <dwmapi.h> // Dwmapi.lib

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

#define WINDOW_CLASS "GGF_Window"
#define WINDOW_TITLE "Gabors Game Framework"

#define HIHI_DPI  1
#define DPI_SCALE 1

#define MI_WP_SIGNATURE         0xFF515700
#define MI_WP_SIGNATURE_MASK    0xFFFFFF00
#define IsTouchEvent(dw) ((dw) & MI_WP_SIGNATURE_MASK) == MI_WP_SIGNATURE

// WinProc processes all inputs, the current input state is always written to the "buffer" Ie: bufferKeyboard, or bufferTouchActive
// Every update, the this (thisKeyboard, thisTouchActive) is copied to the last frame (lastKeyboard, lastTouchActive), and then
// the contents of the live buffer are copied to the this buffer. Tripple buffers input to synch with frame rate.
struct WindowData {
	HWND hwnd;
	HDC hdc;
	HINSTANCE hInstance; // Window instance
	UINT dpi;
	f32 normalizedDPI;
	HGLRC hglrc;

	void* userData;
	bool quit;
	bool running;

	u32 thisKeyboardLow;
	u32 thisKeyboardHigh;
	u32 lastKeyboardLow;
	u32 lastKeyboardHigh;

	u32 thisMouseX;
	u32 thisMouseY;
	i32 thisMouseScroll;

	u32 lastMouseX;
	u32 lastMouseY;
	i32 lastMouseScroll;

	u32 thisTouchX[WINDOW_MAX_TOUCHES];
	u32 thisTouchY[WINDOW_MAX_TOUCHES];
	bool thisTouchActive[WINDOW_MAX_TOUCHES]; // NOTE: Prob better as a u32

	u32 lastTouchX[WINDOW_MAX_TOUCHES];
	u32 lastTouchY[WINDOW_MAX_TOUCHES];
	bool lastTouchActive[WINDOW_MAX_TOUCHES]; // NOTE: Prob better as a u32

	u8 keyMap[256]; // should have 256 elements, key is a char (ie, 'a'), value is my own vk code.

	u32* inputQueue;
	u32 inputQueueLen;
	u32 inputQueueCap;
	u32 inputQueueHead;

	HICON iconLarge;
	HICON iconMedium;
	HICON iconSmall;
};

WindowData gWindowData;
HWND gHwnd;


#ifdef _DEBUG
#define WindowInternal_Assert(cond) if (!(cond)) {*(u8*)0 = 0;}
#else
#define WindowInternal_Assert(cond) 
#endif

extern "C" void* Initialize();
extern "C" void Update(float deltatTime, void* userData);
extern "C" void Render(unsigned int x, unsigned int y, unsigned int w, unsigned int h, float dpi, void* userData);
extern "C" void Shutdown(void* userData);


extern "C" void WindowOpenURL(const char* url) {
	ShellExecuteA(0, 0, url, 0, 0, SW_SHOW);
}

extern "C" u32 MouseGetX() {
	return gWindowData.thisMouseX;
}

extern "C" u32 MouseGetY() {
	return gWindowData.thisMouseY;
}

extern "C" i32 MouseGetScroll() {
	return gWindowData.thisMouseScroll;
}

extern "C" bool MouseDown(u32 button) {
	if (button == MouseButtonLeft) {
		return gWindowData.thisKeyboardLow & (1 << (KeyboardCodeLeftMouse - 1));
	}
	else if (button == MouseButtonMiddle) {
		return gWindowData.thisKeyboardLow & (1 << (KeyboardCodeMiddleMouse - 1));
	}
	else if (button == MouseButtonRight) {
		return gWindowData.thisKeyboardLow & (1 << (KeyboardCodeRightMouse - 1));
	}
	else {
		WindowInternal_Assert(false);
	}

	return 0;
}

extern "C" bool MousePrevDown(u32 button) {
	if (button == MouseButtonLeft) {
		return gWindowData.lastKeyboardLow & (1 << (KeyboardCodeLeftMouse - 1));
	}
	else if (button == MouseButtonMiddle) {
		return gWindowData.lastKeyboardLow & (1 << (KeyboardCodeMiddleMouse - 1));
	}
	else if (button == MouseButtonRight) {
		return gWindowData.lastKeyboardLow & (1 << (KeyboardCodeRightMouse - 1));
	}
	else {
		WindowInternal_Assert(false);
	}
	return 0;
}

extern "C" u32 MousePrevX() {
	return gWindowData.lastMouseX;
}

extern "C" u32 MousePrevY() {
	return gWindowData.lastMouseY;
}

extern "C" i32 MousePrevScroll() {
	return gWindowData.lastMouseScroll;
}

extern "C" u32 TouchGetMaxContacts() {
	return WINDOW_MAX_TOUCHES;
}

extern "C" u32 TouchGetNumContacts() {
	u32 contacts = 0;
	for (u32 i = 0; i < WINDOW_MAX_TOUCHES; ++i) {
		if (gWindowData.thisTouchActive[i]) {
			contacts += 1;
		}
	}
	return contacts;
}

extern "C" u32 TouchGetX(u32 touchIndex) {
	WindowInternal_Assert(touchIndex < WINDOW_MAX_TOUCHES);
	return gWindowData.thisTouchX[touchIndex];
}

extern "C" u32 TouchGetY(u32 touchIndex) {
	WindowInternal_Assert(touchIndex < WINDOW_MAX_TOUCHES);
	return gWindowData.thisTouchY[touchIndex];
}

extern "C" bool TouchIsActive(u32 touchIndex) {
	WindowInternal_Assert(touchIndex < WINDOW_MAX_TOUCHES);
	return gWindowData.thisTouchActive[touchIndex];
}

extern "C" u32 TouchGetPrevX(u32 touchIndex) {
	WindowInternal_Assert(touchIndex < WINDOW_MAX_TOUCHES);
	return gWindowData.lastTouchX[touchIndex];
}

extern "C" u32 TouchGetPrevY(u32 touchIndex) {
	WindowInternal_Assert(touchIndex < WINDOW_MAX_TOUCHES);
	return gWindowData.lastTouchY[touchIndex];
}

extern "C" bool TouchWasActive(u32 touchIndex) {
	WindowInternal_Assert(touchIndex < WINDOW_MAX_TOUCHES);
	return gWindowData.lastTouchActive[touchIndex];
}

extern "C" void WindowUpdateTitle(const char* newTitle) {
	SetWindowTextA(gWindowData.hwnd, newTitle);
}

static void WindowInternal_UpdateInpusState() {
	// Keyboard
	gWindowData.lastKeyboardLow = gWindowData.thisKeyboardLow;
	gWindowData.lastKeyboardHigh = gWindowData.thisKeyboardHigh;

	// Mouse 
	gWindowData.lastMouseX = gWindowData.thisMouseX;
	gWindowData.lastMouseY = gWindowData.thisMouseY;

	POINT p;
	GetCursorPos(&p);
	ScreenToClient(gWindowData.hwnd, &p);

	gWindowData.thisMouseX = p.x;
	gWindowData.thisMouseY = p.y;

	if (!GetAsyncKeyState(VK_LBUTTON)) {
		gWindowData.thisKeyboardLow &= ~(1 << (KeyboardCodeLeftMouse - 1));
	}

	if (!GetAsyncKeyState(VK_RBUTTON)) {
		gWindowData.thisKeyboardLow &= ~(1 << (KeyboardCodeRightMouse - 1));
	}

	if (!GetAsyncKeyState(VK_MBUTTON)) {
		gWindowData.thisKeyboardLow &= ~(1 << (KeyboardCodeMiddleMouse - 1));
	}

	// Scroll
	gWindowData.lastMouseScroll = gWindowData.thisMouseScroll;
	gWindowData.thisMouseScroll = 0; // Clear scroll
	
	// Touches
	for (u32 i = 0; i < WINDOW_MAX_TOUCHES; ++i) {
		gWindowData.lastTouchX[i] = gWindowData.thisTouchX[i];
		gWindowData.lastTouchY[i] = gWindowData.thisTouchY[i];
		gWindowData.lastTouchActive[i] = gWindowData.thisTouchActive[i];
	}
}

static void WindowInternal_InitializeKeyboardMap() {
	gWindowData.keyMap[VK_LBUTTON] = KeyboardCodeLeftMouse;
	gWindowData.keyMap[VK_RBUTTON] = KeyboardCodeRightMouse;
	gWindowData.keyMap[VK_BACK] = KeyboardCodeBackspace;
	gWindowData.keyMap[VK_MBUTTON] = KeyboardCodeMiddleMouse;
	gWindowData.keyMap[VK_TAB] = KeyboardCodeTab;
	gWindowData.keyMap[VK_RETURN] = KeyboardCodeReturn;
	gWindowData.keyMap[VK_SHIFT] = KeyboardCodeShift;
	gWindowData.keyMap[VK_CONTROL] = KeyboardCodeControl;
	gWindowData.keyMap[VK_MENU] = KeyboardCodeAlt;
	gWindowData.keyMap[VK_CAPITAL] = KeyboardCodeCapslock;
	gWindowData.keyMap[VK_ESCAPE] = KeyboardCodeEscape;
	gWindowData.keyMap[VK_SPACE] = KeyboardCodeSpace;
	gWindowData.keyMap[VK_LEFT] = KeyboardCodeLeft;
	gWindowData.keyMap[VK_UP] = KeyboardCodeUp;
	gWindowData.keyMap[VK_RIGHT] = KeyboardCodeRight;
	gWindowData.keyMap[VK_DOWN] = KeyboardCodeDown;
	gWindowData.keyMap[VK_DELETE] = KeyboardCodeDelete;
	gWindowData.keyMap[0x30] = KeyboardCode0;
	gWindowData.keyMap[0x31] = KeyboardCode1;
	gWindowData.keyMap[0x32] = KeyboardCode2;
	gWindowData.keyMap[0x33] = KeyboardCode3;
	gWindowData.keyMap[0x34] = KeyboardCode4;
	gWindowData.keyMap[0x35] = KeyboardCode5;
	gWindowData.keyMap[0x36] = KeyboardCode6;
	gWindowData.keyMap[0x37] = KeyboardCode7;
	gWindowData.keyMap[0x38] = KeyboardCode8;
	gWindowData.keyMap[0x39] = KeyboardCode9;
	gWindowData.keyMap[0x41] = KeyboardCodeA;
	gWindowData.keyMap[0x42] = KeyboardCodeB;
	gWindowData.keyMap[0x43] = KeyboardCodeC;
	gWindowData.keyMap[0x44] = KeyboardCodeD;
	gWindowData.keyMap[0x45] = KeyboardCodeE;
	gWindowData.keyMap[0x46] = KeyboardCodeF;
	gWindowData.keyMap[0x47] = KeyboardCodeG;
	gWindowData.keyMap[0x48] = KeyboardCodeH;
	gWindowData.keyMap[0x49] = KeyboardCodeI;
	gWindowData.keyMap[0x4A] = KeyboardCodeJ;
	gWindowData.keyMap[0x4B] = KeyboardCodeK;
	gWindowData.keyMap[0x4C] = KeyboardCodeL;
	gWindowData.keyMap[0x4D] = KeyboardCodeM;
	gWindowData.keyMap[0x4E] = KeyboardCodeN;
	gWindowData.keyMap[0x4F] = KeyboardCodeO;
	gWindowData.keyMap[0x50] = KeyboardCodeP;
	gWindowData.keyMap[0x51] = KeyboardCodeQ;
	gWindowData.keyMap[0x52] = KeyboardCodeR;
	gWindowData.keyMap[0x53] = KeyboardCodeS;
	gWindowData.keyMap[0x54] = KeyboardCodeT;
	gWindowData.keyMap[0x55] = KeyboardCodeU;
	gWindowData.keyMap[0x56] = KeyboardCodeV;
	gWindowData.keyMap[0x57] = KeyboardCodeW;
	gWindowData.keyMap[0x58] = KeyboardCodeX;
	gWindowData.keyMap[0x59] = KeyboardCodeY;
	gWindowData.keyMap[0x5A] = KeyboardCodeZ;
	gWindowData.keyMap[VK_OEM_PLUS] = KeyboardCodePlus;
	gWindowData.keyMap[VK_OEM_COMMA] = KeyboardCodeComma;
	gWindowData.keyMap[VK_OEM_MINUS] = KeyboardCodeMinus;
	gWindowData.keyMap[VK_OEM_PERIOD] = KeyboardCodePeriod;
	gWindowData.keyMap[VK_OEM_1] = KeyboardCodeSemicolon;
	gWindowData.keyMap[VK_OEM_2] = KeyboardCodeQuestionmark;
	gWindowData.keyMap[VK_OEM_3] = KeyboardCodeTilde;
	gWindowData.keyMap[VK_OEM_4] = KeyboardCodeLBracket;
	gWindowData.keyMap[VK_OEM_5] = KeyboardCodeBackslash;
	gWindowData.keyMap[VK_OEM_6] = KeyboardCodeRbracket;
	gWindowData.keyMap[VK_OEM_7] = KeyboardCodeQoute;
}

// WinMain follows. There is a main, and run function. 
// use run as an entry point if there is no c runtime.

extern "C" bool KeyboardDown(u32 scanCode) {
	PlatformAssert(scanCode > 0, __LOCATION__);
	PlatformAssert(scanCode <= 64, __LOCATION__);
	u32 bit = scanCode - 1;

	if (bit < 32) { // 0 to 32 is in low
		return gWindowData.thisKeyboardLow & (1 << bit);
	}
	else { // 32 to 64 
		return gWindowData.thisKeyboardHigh & (1 << (bit - 32));
	}
	
	return false;
}

extern "C" bool KeyboardPrevDown(u32 scanCode) {
	PlatformAssert(scanCode > 0, __LOCATION__);
	PlatformAssert(scanCode <= 64, __LOCATION__);
	u32 bit = scanCode - 1;

	if (bit < 32) { // 0 to 32 is in low
		return gWindowData.lastKeyboardLow & (1 << bit);
	}
	else { // 32 to 64 
		return gWindowData.lastKeyboardHigh & (1 << (bit - 32));
	}

	return false;
}

extern "C" void PushKey(u32 scanCode) {
	if (gWindowData.inputQueueLen + 1 >= gWindowData.inputQueueCap) {
		if (gWindowData.inputQueueCap == 0) {
			gWindowData.inputQueueCap = 4;
		}
		gWindowData.inputQueue = (u32*)MemRealloc(gWindowData.inputQueue, gWindowData.inputQueueCap * 2 * sizeof(u32));
		gWindowData.inputQueueCap = gWindowData.inputQueueCap * 2;
	}

	gWindowData.inputQueue[gWindowData.inputQueueLen++] = scanCode;
}

extern "C" u32 ConsumeKeyQueue() {
	if (gWindowData.inputQueueHead >= gWindowData.inputQueueLen) {
		return KeyboardCodeLeftMouse;
	}

	return gWindowData.inputQueue[gWindowData.inputQueueHead++];
}

extern "C" void ClearKeyQueue() {
	gWindowData.inputQueueHead = 0;
	gWindowData.inputQueueLen = 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	static HCURSOR arrow = LoadCursor(NULL, IDC_ARROW);

	switch (iMsg) {
	case WM_CREATE:
	{
		// This sets dark mode, without caring about user theme...
		BOOL value = TRUE;
		::DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));

		ShowWindow(gWindowData.hwnd, SW_NORMAL);
		InvalidateRect(gWindowData.hwnd, NULL, TRUE);
		UpdateWindow(gWindowData.hwnd);
	}
		return 0;
	case WM_SETCURSOR:
		SetCursor(arrow);
		return TRUE;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		return 0; // If an application processes this message, it should return zero.
	case WM_KEYDOWN:
		PlatformAssert(wParam < 256, __LOCATION__);
		if (wParam < 256) {
			u32 scanCode = gWindowData.keyMap[wParam];
			PlatformAssert(scanCode > 0, __LOCATION__);
			PlatformAssert(scanCode <= 64, __LOCATION__);
			u32 bit = scanCode - 1;

			if (bit < 32) { // 0 to 32 is in low
				gWindowData.thisKeyboardLow |= (1 << bit);
			}
			else { // 32 to 64 
				gWindowData.thisKeyboardHigh |= (1 << (bit - 32));
			}

			PushKey(scanCode);

			return 0;
		}
	break;
	case WM_KEYUP:
		PlatformAssert(wParam < 256, __LOCATION__);
		if (wParam < 256) {
			u32 scanCode = gWindowData.keyMap[wParam];
			PlatformAssert(scanCode > 0, __LOCATION__);
			PlatformAssert(scanCode <= 64, __LOCATION__);
			u32 bit = scanCode - 1;

			if (bit < 32) { // 0 to 32 is in low
				gWindowData.thisKeyboardLow &= ~(1 << bit);
			}
			else { // 32 to 64 
				gWindowData.thisKeyboardHigh &= ~(1 << (bit - 32));
			}

			return 0;
		}
	break;
	case WM_MOUSEWHEEL:
		gWindowData.thisMouseScroll = 0;
		if ((short)HIWORD(wParam) > 0) {
			gWindowData.thisMouseScroll = 1;
		}
		else if ((short)HIWORD(wParam) < 0) {
			gWindowData.thisMouseScroll = -1;
		}
		return 0;
#if 0
	case WM_MOUSEMOVE: {
		LPARAM info = GetMessageExtraInfo();
		if (IsTouchEvent(info)) {
			break;
		}
		
		gWindowData.thisMouseX = (u32)(GET_X_LPARAM(lParam));
		gWindowData.thisMouseY = (u32)(GET_Y_LPARAM(lParam));
		return 0;
	}
#endif
	case WM_TOUCH: {
		TOUCHINPUT  points[WINDOW_MAX_TOUCHES] = { 0 };
		UINT numInputs = LOWORD(wParam);
		if (numInputs > WINDOW_MAX_TOUCHES) {
			numInputs = WINDOW_MAX_TOUCHES;
		}

		if (numInputs > 0) {
			GetTouchInputInfo((HTOUCHINPUT)lParam, WINDOW_MAX_TOUCHES, points, sizeof(TOUCHINPUT));

			POINT ptClient = { 0,0 };
			ClientToScreen(hwnd, &ptClient);

			for (u32 i = 0; i < WINDOW_MAX_TOUCHES; ++i) {
				gWindowData.thisTouchX[i] = points[i].x / 100 - ptClient.x;
				gWindowData.thisTouchY[i] = points[i].y / 100 - ptClient.y;

				if (points[i].dwFlags & TOUCHEVENTF_DOWN) {
					gWindowData.thisTouchActive[i] = true;
				}
				else if (points[i].dwFlags & TOUCHEVENTF_UP) {
					gWindowData.thisTouchActive[i] = false;
				}
				else if (points[i].dwFlags & TOUCHEVENTF_MOVE) {
					gWindowData.thisTouchActive[i] = true;
				}
			}

			CloseTouchInputHandle((HTOUCHINPUT)lParam);
		}
		
		return 0;
	}
#if 1
	case WM_LBUTTONDOWN: {
		LPARAM info = GetMessageExtraInfo();
		if (IsTouchEvent(info)) {
			break;
		}

		gWindowData.thisMouseX = (u32)(GET_X_LPARAM(lParam));
		gWindowData.thisMouseY = (u32)(GET_Y_LPARAM(lParam));
		gWindowData.thisKeyboardLow |= (1 << (KeyboardCodeLeftMouse - 1));
		return 0;
	}
	case WM_RBUTTONDOWN: {
		LPARAM info = GetMessageExtraInfo();
		if (IsTouchEvent(info)) {
			break;
		}

		gWindowData.thisMouseX = (u32)(GET_X_LPARAM(lParam));
		gWindowData.thisMouseY = (u32)(GET_Y_LPARAM(lParam));
		gWindowData.thisKeyboardLow |= (1 << (KeyboardCodeRightMouse - 1));
		return 0;
	}
	case WM_MBUTTONDOWN: {
		LPARAM info = GetMessageExtraInfo();
		if (IsTouchEvent(info)) {
			break;
		}

		gWindowData.thisMouseX = (u32)(GET_X_LPARAM(lParam));
		gWindowData.thisMouseY = (u32)(GET_Y_LPARAM(lParam));
		gWindowData.thisKeyboardLow |= (1 << (KeyboardCodeMiddleMouse - 1));
		return 0;
	}
	case WM_LBUTTONUP: {
		LPARAM info = GetMessageExtraInfo();
		if (IsTouchEvent(info)) {
			break;
		}

		gWindowData.thisMouseX = (u32)(GET_X_LPARAM(lParam));
		gWindowData.thisMouseY = (u32)(GET_Y_LPARAM(lParam));
		gWindowData.thisKeyboardLow &= ~(1 << (KeyboardCodeLeftMouse - 1));
		return 0;
	}
	case WM_RBUTTONUP: {
		LPARAM info = GetMessageExtraInfo();
		if (IsTouchEvent(info)) {
			break;
		}

		gWindowData.thisMouseX = (u32)(GET_X_LPARAM(lParam));
		gWindowData.thisMouseY = (u32)(GET_Y_LPARAM(lParam));
		gWindowData.thisKeyboardLow &= ~(1 << (KeyboardCodeRightMouse - 1));
		return 0;
	}
	case WM_MBUTTONUP: {
		LPARAM info = GetMessageExtraInfo();
		if (IsTouchEvent(info)) {
			break;
		}

		gWindowData.thisMouseX = (u32)(GET_X_LPARAM(lParam));
		gWindowData.thisMouseY = (u32)(GET_Y_LPARAM(lParam));
		gWindowData.thisKeyboardLow &= ~(1 << (KeyboardCodeMiddleMouse - 1));
		return 0;
	}
#endif
	case WM_DESTROY:
	{
		WindowData* windowData = (WindowData*)GetWindowLongPtrA(hwnd, GWLP_USERDATA);
		PlatformAssert(windowData == &gWindowData, __LOCATION__);
		windowData->running = false;
		if (windowData->iconLarge != 0) {
			DestroyIcon(windowData->iconLarge);
			windowData->iconLarge = 0;
		}
		if (windowData->iconMedium != 0) {
			DestroyIcon(windowData->iconMedium);
			windowData->iconMedium = 0;
		}
		if (windowData->iconSmall != 0) {
			DestroyIcon(windowData->iconSmall);
			windowData->iconSmall = 0;
		}

		Shutdown(windowData->userData);
		windowData->userData = 0;
		GfxShutdown(0);
	}
		PostQuitMessage(0);
		return 0; // If an application processes this message, it should return zero.
	case WM_ERASEBKGND:
		return TRUE; // Ignore
	case WM_PAINT:
		/*PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);*/
		return 0; // An application returns zero if it processes this message.
	}

	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow) {
#if HIHI_DPI
	HRESULT hr = SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
#endif

	MemClear(&gWindowData, sizeof(WindowData));

	LARGE_INTEGER timerFrequency;
	if (!QueryPerformanceFrequency(&timerFrequency)) {
		PrintDebugString("WinMain: QueryPerformanceFrequency failed\n");
	}

	{ // Create window
		char windowClass[32] = { 0 };
		int i = 0;
		for (i = 0; i < 32; ++i) {
			windowClass[i] = WINDOW_CLASS[i];
			if (windowClass[i] == 0) {
				break;
			}
		}
		for (; i < 32; ++i) {
			windowClass[i] = (char)(MathRandom() * 255.0f);
		}

		const char* title_a = WINDOW_TITLE;
		TCHAR title_w[64] = { 0 }; // CreateWindowA still takes a wchar string
		for (int i = 0; i < 63; ++i) {
			title_w[i] = title_a[i];
			if (title_a[i] == 0) {
				break;
			}
		}
		title_w[63] = '\0';

		WNDCLASSA wndclass;
		wndclass.style = CS_HREDRAW | CS_VREDRAW;
		wndclass.lpfnWndProc = WndProc;
		wndclass.cbClsExtra = 0;
		wndclass.cbWndExtra = 0;
		wndclass.hInstance = hInstance;
		wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wndclass.hCursor = NULL;// LoadCursor(NULL, IDC_ARROW);
		wndclass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
		wndclass.lpszMenuName = 0;
		wndclass.lpszClassName = windowClass;
		RegisterClassA(&wndclass);

#if DPI_SCALE
		f32 dpi = (float)GetDpiForSystem() / 96.0f;
#else
		f32 dpi = 1.0f;
#endif
		f32 windowWidth = GetSystemMetrics(SM_CXSCREEN) - 400;
		f32 windowHeight = GetSystemMetrics(SM_CYSCREEN) - 400;

		RECT rClient;
		SetRect(&rClient, 0, 0, windowWidth, windowHeight);
		AdjustWindowRect(&rClient, WS_OVERLAPPEDWINDOW | WS_VISIBLE, FALSE);

		HWND hwnd = CreateWindowA(windowClass, (char*)title_w, WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			(GetSystemMetrics(SM_CXSCREEN) / 2) - ((rClient.right - rClient.left) / 2),
			(GetSystemMetrics(SM_CYSCREEN) / 2) - ((rClient.bottom - rClient.top) / 2),
			(rClient.right - rClient.left), (rClient.bottom - rClient.top), NULL, NULL, hInstance, 0);
		HDC hdc = GetDC(hwnd);
		RegisterTouchWindow(hwnd, 0);
		

		gHwnd = hwnd;
		MemClear(&gWindowData, sizeof(WindowData));
		gWindowData.hwnd = hwnd;
		gWindowData.hInstance = hInstance;
		gWindowData.dpi = GetDpiForSystem();
		gWindowData.normalizedDPI = dpi;
		gWindowData.quit = false;
		gWindowData.running = true;
		gWindowData.hdc = hdc;

		// Stash window data 
		LONG_PTR lptr = (LONG_PTR)(&gWindowData);
		SetWindowLongPtrA(gWindowData.hwnd, GWLP_USERDATA, lptr);
	}

	// Init OpenGL
#if DPI_SCALE
	f32 dpi = (float)GetDpiForSystem() / 96.0f;
#else
	f32 dpi = 1.0f;
#endif

	void* mem = (unsigned char*)MemPlatformAllocate(WINDOW_HEAP_SIZE);
	void* heap = MemInitializeHeap(mem, WINDOW_HEAP_SIZE);
	int vsynch = GfxInitialize(&gWindowData.hdc, &gWindowData.hglrc);
	WindowInternal_InitializeKeyboardMap();

	
	MSG msg = { 0 };
	gWindowData.userData = Initialize();

	LARGE_INTEGER thisTime;
	QueryPerformanceCounter(&thisTime);
	LARGE_INTEGER lastTime = thisTime;

	while (!gWindowData.quit) {
		if (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				gWindowData.quit = true;
				gWindowData.running = false;
				break;
			}
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}
		if (gWindowData.running) {

			QueryPerformanceCounter(&thisTime);
			auto timeDelta = thisTime.QuadPart - lastTime.QuadPart;
			double deltaTime = (double)timeDelta * 1000.0 / (double)timerFrequency.QuadPart;
			deltaTime = deltaTime / 1000.0;
			Update((float)deltaTime, gWindowData.userData);
			lastTime = thisTime;

			RECT clientRect = {};
			GetClientRect(gWindowData.hwnd, &clientRect);
			Render(clientRect.left, clientRect.top, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top, dpi, gWindowData.userData);

			WindowInternal_UpdateInpusState();
			

			SwapBuffers(gWindowData.hdc);
			if (vsynch != 0) {
				GfxFinish();
			}
		}
	}

	if (gWindowData.inputQueue != 0) {
		MemRelease(gWindowData.inputQueue);
		gWindowData.inputQueue = 0;
	}
	
	MemShutdownHeap(heap);
	MemPlatformRelease(mem);

	return (int)msg.wParam;
}

// https://gist.github.com/gyakoo/04da0231c632898bae5d4c966c0c4d3e
extern "C" char* ReadClipboard() {
	if (!IsClipboardFormatAvailable(CF_TEXT)) {
		return 0;
	}

	if (!OpenClipboard(NULL)) {
		return 0;
	}

	HGLOBAL hg = GetClipboardData(CF_TEXT);
	char* result = 0;
	if (hg) {
		LPCSTR strData = (LPCSTR)GlobalLock(hg);
		if (strData) {
			LPCSTR null = strData;
			while (*null) {
				++null;
			}
			i32 len = null - strData;
			if (len > 0) {
				result = (char*)MemAlloc(len + 1);
				for (i32 i = 0; i < len; ++i) {
					result[i] = strData[i];
				}
				result[len] = '\0';
				GlobalUnlock(hg);
			}
			
		}
	}

	CloseClipboard();
	return result;
}

// https://stackoverflow.com/questions/1264137/how-to-copy-string-to-clipboard-in-c
extern "C" void WriteClipboard(const char* buffer) { 
	if (buffer == 0) {
		return;
	}
	const char* null = buffer;
	while (*null) {
		++null;
	}
	i32 len = (null - buffer);

	HGLOBAL hdst;
	LPWSTR dst;

	// Allocate string
	hdst = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, (len + 1) * sizeof(WCHAR));
	dst = (LPWSTR)GlobalLock(hdst);

	for (u32 i = 0; i < len; ++i) {
		dst[i] = buffer[i];
	}
	dst[len] = 0;
	GlobalUnlock(hdst);

	// Set clipboard data
	if (!OpenClipboard(NULL)) {
		PlatformAssert(false, __LOCATION__);
	}
	EmptyClipboard();
	if (!SetClipboardData(CF_UNICODETEXT, hdst)) {
		PlatformAssert(false, __LOCATION__);
	}
	CloseClipboard();
}

#if 0

// https://stackoverflow.com/questions/41533158/create-32-bit-color-icon-programmatically
static HICON CreateIconFromBytes(HDC DC, int width, int height, u32* bytes) {
	HICON hIcon = NULL;

	ICONINFO iconInfo = {
		TRUE, // fIcon, set to true if this is an icon, set to false if this is a cursor
		NULL, // xHotspot, set to null for icons
		NULL, // yHotspot, set to null for icons
		NULL, // Monochrome bitmap mask, set to null initially
		NULL  // Color bitmap mask, set to null initially
	};

	u32* rawBitmap = (u32*)MemAlloc(width * height * sizeof(u32));

	ULONG uWidth = (ULONG)width;
	ULONG uHeight = (ULONG)height;
	u32* bitmapPtr = rawBitmap;
	for (ULONG y = 0; y < uHeight; y++) {
		for (ULONG x = 0; x < uWidth; x++) {
			// Bytes are expected to be in RGB order (8 bits each)
			// Swap G and B bytes, so that it is in BGR order for windows
			u32 byte = bytes[x + y * width];
			u8 A = (byte & 0xff000000) >> 24;
			u8 R = (byte & 0xff0000) >> 16;
			u8 G = (byte & 0xff00) >> 8;
			u8 B = (byte & 0xff);
			*bitmapPtr = (A << 24) | (R << 16) | (G << 8) | B;
			bitmapPtr++;
		}
	}

	iconInfo.hbmColor = CreateBitmap(width, height, 1, 32, rawBitmap);
	if (iconInfo.hbmColor) {
		iconInfo.hbmMask = CreateCompatibleBitmap(DC, width, height);
		if (iconInfo.hbmMask) {
			hIcon = CreateIconIndirect(&iconInfo);
			if (hIcon == NULL) {
				PrintDebugString("Failed to create icon.");
			}
			DeleteObject(iconInfo.hbmMask);
		}
		else {
			PrintDebugString("Failed to create color mask.");
		}
		DeleteObject(iconInfo.hbmColor);
	}
	else {
		PrintDebugString("Failed to create bitmap mask.");
	}

	delete[] rawBitmap;

	return hIcon;
}

extern "C" int SetIcon(int level, u32 w, u32 h, void* rgba) {
	HICON icon = CreateIconFromBytes(gWindowData.hdc, w, h, (u32*)rgba);

	if (level == 0) {
		if (gWindowData.iconLarge != 0) {
			DestroyIcon(gWindowData.iconLarge);
			gWindowData.iconLarge = 0;
		}
		gWindowData.iconLarge = icon;

		SendMessage(gWindowData.hwnd, WM_SETICON, ICON_BIG, (LPARAM)icon);
	}
	else if (level == 1) {
		if (gWindowData.iconMedium != 0) {
			DestroyIcon(gWindowData.iconMedium);
			gWindowData.iconMedium = 0;
		}
		gWindowData.iconMedium = icon;
		SendMessage(gWindowData.hwnd, WM_SETICON, ICON_SMALL, (LPARAM)icon);
	}
	else {
		if (gWindowData.iconSmall != 0) {
			DestroyIcon(gWindowData.iconSmall);
			gWindowData.iconSmall = 0;
		}
		gWindowData.iconSmall = icon;
		SendMessage(gWindowData.hwnd, WM_SETICON, ICON_SMALL2, (LPARAM)icon);
	}

	return 3; // Low, medium, high
}
#endif

int run() {
	return WinMain(GetModuleHandle(NULL), NULL, GetCommandLineA(), SW_SHOWDEFAULT);
}

int main(int argc, char** argv) {
	return run();
}
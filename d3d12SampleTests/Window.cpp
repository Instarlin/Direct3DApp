#include "Window.h"
#include "resource.h"
#include "Keyboard.h"
#include <basetsd.h>
#include <minwindef.h>
#include <windef.h>
#include <libloaderapi.h>
#include <winnt.h>
#include <WinUser.h>
#include <cstring>
#include <stringapiset.h>
#include <WinNls.h>

Window::WindowClass Window::WindowClass::wndClass;

Window::WindowClass::WindowClass() noexcept
	:
	hInst(GetModuleHandle(nullptr)) {
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = HandleMsgSetup;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetInstance();
	wc.hIcon = static_cast<HICON>(LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 32, 32, 0));
	wc.hCursor = nullptr;
	wc.hbrBackground = nullptr;
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = Window::cToWCHAR(wndClassName);
	wc.hIconSm = static_cast<HICON>(LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 16, 16, 0));;
	RegisterClassEx(&wc);
}

Window::WindowClass::~WindowClass() {
	UnregisterClass(Window::cToWCHAR(wndClassName), GetInstance());
}

const char* Window::WindowClass::GetName() noexcept {
	return wndClassName;
}

HINSTANCE Window::WindowClass::GetInstance() noexcept {
	return wndClass.hInst;
}

Window::Window(int width, int height, const char* name):width(width), height(height) {
	RECT wr;
	wr.left = 100;
	wr.right = width + wr.left;
	wr.top = 100;
	wr.bottom = height + wr.top;
	AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE);
	hWnd = CreateWindow(
		cToWCHAR(WindowClass::GetName()), Window::cToWCHAR(name),
		WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top,
		nullptr, nullptr, WindowClass::GetInstance(), this
	);
	
	ShowWindow(hWnd, SW_SHOWDEFAULT);
}

Window::~Window() {
	DestroyWindow(hWnd);
}

const WCHAR* Window::cToWCHAR(const char* source) noexcept {
	if (source == nullptr) {
		return nullptr;
	};

	unsigned int const sourceLength = static_cast<unsigned int>(strlen(source) + 1);
	unsigned int const wcharLength = MultiByteToWideChar(
		CP_ACP,
		0,
		source,
		sourceLength,
		NULL,
		0
	);
	WCHAR* wString = new WCHAR[wcharLength];
	MultiByteToWideChar(
		CP_ACP,
		0,
		source,
		sourceLength,
		wString,
		wcharLength
	);

	return wString;
}

LRESULT CALLBACK Window::HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) noexcept {
	if (msg == WM_NCCREATE) {
		const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lp);
		Window* const pWnd = static_cast<Window*>(pCreate->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
		SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::HandleMsgThunk));
		return pWnd->HandleMsg(hWnd, msg, wp, lp);
	}
	return DefWindowProc(hWnd, msg, wp, lp);
}

LRESULT CALLBACK Window::HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) noexcept {
	Window* const pWnd = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	return pWnd->HandleMsg(hWnd, msg, wp, lp);
}

LRESULT Window::HandleMsg(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) noexcept {
	switch (msg) {
		case WM_CLOSE:
			PostQuitMessage(0);
			return 0;
		case WM_KILLFOCUS:
			kbd.ClearState();
			break;
		case WM_KEYDOWN:
			if (!(lp & 0x40000000) || kbd.AutorepeatIsEnabled()) {
				kbd.OnKeyPressed(static_cast<unsigned char>(wp));
			}
			break;
		case WM_KEYUP:
			kbd.OnKeyReleased(static_cast<unsigned char>(wp));
			break;
		case WM_CHAR:
			kbd.OnChar(static_cast<unsigned char>(wp));
	}

	return DefWindowProc(hWnd, msg, wp, lp);
}
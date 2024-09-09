#include "Window.h"

int WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow
) {
	Window wnd(800, 500, "New Window");

	wnd.kbd.EnableAutorepeat();

	MSG msg;
	BOOL gResult;
	while (gResult = GetMessage(&msg, nullptr, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (wnd.kbd.KeyIsPressed(VK_SPACE)) {
			MessageBox(nullptr, wnd.cToWCHAR("New text"), wnd.cToWCHAR("Space key pressed"), MB_OK);
		}
	}

	if (gResult == -1) {
		return -1;
	}

	return static_cast<unsigned int>(msg.wParam);
}
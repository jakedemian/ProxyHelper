#include "stdafx.h"
#include <Windows.h>
#include <WinInet.h>

#define WM_MYMESSAGE (WM_USER + 1)
#define MENU_ENABLE 0
#define MENU_DISABLE 1
#define MENU_SEPARATOR 2
#define MENU_EXIT 3

const DWORD ENABLED_VALUE = 1;
const DWORD DISABLED_VALUE = 0;
const LPCWSTR REGISTRY_KEY_LOCATION = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings");
const LPCWSTR REGISTRY_KEY_NAME = TEXT("ProxyEnable");
const LPCWSTR MY_WINDOW_CLASS = TEXT("myWindowClass");

PHKEY OpenRegistryKey(PHKEY key) {
	LONG err = RegOpenKey(HKEY_CURRENT_USER, REGISTRY_KEY_LOCATION, key);
	if (err != ERROR_SUCCESS) {
		MessageBox(NULL, TEXT("Failed to open the registry key ProxyEnable."), TEXT("Registry open failed!"),
			MB_ICONEXCLAMATION | MB_OK);
	}
	return key;
}

void SetProxy(DWORD value) {
	HKEY key;
	OpenRegistryKey(&key);
	LONG err = RegSetValueEx(key, REGISTRY_KEY_NAME, 0, REG_DWORD, (const BYTE*)&value, sizeof(value));
	if (err != ERROR_SUCCESS) {
		MessageBox(NULL, TEXT("Failed to set the registry value for key ProxyEnable."), TEXT("Registry update failed!"),
			MB_ICONEXCLAMATION | MB_OK);
	}
	RegCloseKey(key);

	InternetSetOption(NULL, INTERNET_OPTION_PROXY_SETTINGS_CHANGED, NULL, 0);
	InternetSetOption(NULL, INTERNET_OPTION_REFRESH, NULL, 0);
}

void EnableProxy() {
	SetProxy(ENABLED_VALUE);
}

void DisableProxy() {
	SetProxy(DISABLED_VALUE);
}

void ShowContextMenu(HWND hWnd) {
	HMENU hMenu = CreatePopupMenu();
	AppendMenu(hMenu, MF_STRING, MENU_ENABLE, TEXT("Enable Proxy"));
	AppendMenu(hMenu, MF_STRING, MENU_DISABLE, TEXT("Disable Proxy"));
	AppendMenu(hMenu, MF_SEPARATOR, MENU_SEPARATOR, NULL);
	AppendMenu(hMenu, MF_STRING, MENU_EXIT, TEXT("Exit"));
	
	SetForegroundWindow(hWnd);
	POINT p;
	GetCursorPos(&p);
	TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_BOTTOMALIGN, p.x, p.y, 0, hWnd, NULL);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
	switch (msg){
		case WM_CLOSE:
			DestroyWindow(hwnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_MYMESSAGE:
			switch (lParam){
				case WM_RBUTTONDOWN:
				case WM_CONTEXTMENU:
					ShowContextMenu(hwnd);
					break;
			}
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case MENU_ENABLE:
					EnableProxy();
					break;
				case MENU_DISABLE:
					DisableProxy();
					break;
				case MENU_EXIT:
					exit(0);
					break;
			}
			break;
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

void CreateSystemTrayIcon(HWND hWnd) {
	NOTIFYICONDATA niData;
	niData.cbSize = sizeof(NOTIFYICONDATA);
	niData.hWnd = hWnd;
	niData.uID = 100;
	niData.uVersion = NOTIFYICON_VERSION;
	niData.uCallbackMessage = WM_MYMESSAGE;
	niData.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcscpy_s(niData.szTip, L"ProxyHelper");
	niData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;

	Shell_NotifyIcon(NIM_ADD, &niData);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	WNDCLASSEX wc;
	HWND hwnd;
	MSG Msg;

	//Step 1: Registering the Window Class
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = MY_WINDOW_CLASS;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc)) {
		MessageBox(NULL, TEXT("Window Registration Failed!"), TEXT("Error!"),
			MB_ICONEXCLAMATION | MB_OK);
	}

	// Step 2: Creating the Window
	hwnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		MY_WINDOW_CLASS,
		TEXT("The title of my window"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 240, 120,
		NULL, NULL, hInstance, NULL);

	if (hwnd == NULL) {
		MessageBox(NULL, TEXT("Window Creation Failed!"), TEXT("Error!"),
			MB_ICONEXCLAMATION | MB_OK);
	}

	UpdateWindow(hwnd);

    // other stuff here ///
	CreateSystemTrayIcon(hwnd);
	///////////////////////

	while (GetMessage(&Msg, NULL, 0, 0) > 0) {
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	return 0;
}

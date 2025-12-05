#include <windows.h>
#include <shellapi.h>
#include <thread>
#include <atomic>
#include "resource.h"

#define WM_TRAYICON (WM_USER + 1)
#define WM_SHOW_CONSOLE (WM_USER + 2)

HWND g_consoleWnd = NULL;
HWND g_trayHwnd = NULL;
HINSTANCE g_hInstance = NULL;
std::atomic<bool> g_consoleVisible = true;


void Show()
{
    ShowWindow(GetConsoleWindow(), SW_SHOW);
    SetForegroundWindow(GetConsoleWindow());
    g_consoleVisible = true;
}

void Hide()
{
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    g_consoleVisible = false;
}

LRESULT CALLBACK TrayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_TRAYICON:
        if (lParam == WM_LBUTTONUP)
        {
            if (g_consoleVisible)
                Hide();
            else
                Show();
        }
        break;
    case WM_SHOW_CONSOLE:
        Show();
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void InitTrayIcon()
{
    g_consoleWnd = GetConsoleWindow();
    g_hInstance = GetModuleHandle(nullptr);

    WNDCLASS wc = {};
    wc.lpfnWndProc = TrayWndProc;
    wc.hInstance = g_hInstance;
    wc.lpszClassName = "VRChat-YT-DLP-Fix";

    RegisterClass(&wc);

    g_trayHwnd = CreateWindow(
        wc.lpszClassName,
        "VRChat-YT-DLP-Fix",
        0,
        0, 0, 0, 0,
        NULL, NULL,
        g_hInstance, NULL
    );

    NOTIFYICONDATA nid = {};
    nid.cbSize = sizeof(nid);
    nid.hWnd = g_trayHwnd;
    nid.uID = 1;
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_ICON1));
    if (!nid.hIcon)
        nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    strcpy_s(nid.szTip, "VRChat-YT-DLP-Fix");

    Shell_NotifyIcon(NIM_ADD, &nid);
}

void TrayMessageLoop()
{
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void InitAndHide() // run at main
{
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
    freopen("CONIN$",  "r", stdin);

    InitTrayIcon();

    // Hide at start
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    g_consoleVisible = false;
}

void ShowFromOtherProcess()
{
    PostMessage(g_trayHwnd, WM_SHOW_CONSOLE, 0, 0);
}
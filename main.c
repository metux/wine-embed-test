#include <stdio.h>
#include <inttypes.h>

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <windowsx.h>

const wchar_t *PROP_NAME = L"__wine_x11_whole_window";

//
// Simple Win32 example: main window + several child windows
//

// Window procedures for each subwindow type
LRESULT CALLBACK ChildProc1(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_PAINT:
        {
            int XID = (int)GetPropW(hwnd, PROP_NAME);

            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            wchar_t buf[32];  // enough for "0x" + 16 hex digits + NUL
            swprintf(buf, sizeof(buf)/sizeof(buf[0]), L"HWND: 0x% XID: 0x%X" PRIxPTR, (uintptr_t)hwnd, XID);
            TextOutW(hdc, 10, 10, buf, wcslen(buf));
//            TextOutW(hdc, 10, 10, L"This is Child 1", 15);
            EndPaint(hwnd, &ps);
            return 0;
        }
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK ChildProc2(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            TextOutW(hdc, 10, 10, L"This is Child 2", 15);
            EndPaint(hwnd, &ps);
            return 0;
        }
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK ChildProc3(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            TextOutW(hdc, 10, 10, L"This is Child 3", 15);
            EndPaint(hwnd, &ps);
            return 0;
        }
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

// Main window procedure
LRESULT CALLBACK MainProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int flags = WS_CHILD | WS_VISIBLE | WS_BORDER | WS_OVERLAPPED | 0x01;
    switch (msg)
    {
        case WM_CREATE:
        {
            HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;
//            int flags = WS_CHILD | WS_VISIBLE | WS_BORDER | WS_OVERLAPPED;

            // Create 3 child windows, each with its own class and title
            HWND sub1 = CreateWindowExW(0, L"ChildClass1", L"Child Window #1",
                            flags,
                            13, 13, 211, 80, hwnd, NULL, hInst, NULL);

//            HWND sub2 = CreateWindowExW(0, L"ChildClass2", L"Child Window #2",
//                            flags,
//                            13, 113, 211, 80, hwnd, NULL, hInst, NULL);

//            HWND sub3 = CreateWindowExW(0, L"ChildClass3", L"Child Window #3",
//                            flags,
//                            13, 213, 211, 80, hwnd, NULL, hInst, NULL);

            return 0;
        }

        case WM_RBUTTONDOWN: {
            HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;

            HWND sub3 = CreateWindowExW(0, L"ChildClass1", L"Child Window #4",
                            flags,
                            13, 13, 400, 400, hwnd, NULL, hInst, NULL);

//            HWND sub3 = CreateWindowExW(0, L"ChildClass3", L"Child Window #3",
//                            flags,
//                            5, 5, 400, 400, hwnd, NULL, hInst, NULL);

//            const wchar_t *PROP_NAME = L"__wine_x11_whole_window";

//            int myData = (int)GetPropW(sub3, PROP_NAME);
//            wchar_t buffer[64];
//            wsprintfW(buffer, L"Property value = 0x%X", myData);
//            MessageBoxW(hwnd, buffer, L"GetPropW", MB_OK);

            return 0;
        }

        case WM_MOUSEMOVE: {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);

            wchar_t buffer[64];
            wsprintfW(buffer, L"Mouse at %d, %d", x, y);
            SetWindowTextW(hwnd, buffer); // update window title with position
            return 0;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE hPrev, PWSTR lpCmdLine, int nShowCmd)
{
    // --- Register main window class ---
    WNDCLASSW wc = {
        .lpfnWndProc = MainProc,
        .hInstance = hInst,
        .hCursor = LoadCursorW(NULL, IDC_ARROW),
        .hbrBackground = (HBRUSH)(COLOR_WINDOW + 1),
        .lpszClassName = L"MainWinClass",
    };
    RegisterClassW(&wc);

    // --- Register 3 different child window classes ---
    WNDCLASSW child1 = {
        .hInstance = hInst,
        .hCursor = LoadCursorW(NULL, IDC_ARROW),
        .hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1),
        .lpfnWndProc = ChildProc1,
        .lpszClassName = L"ChildClass1",
    };
    RegisterClassW(&child1);

    WNDCLASSW child2 = {
        .hInstance = hInst,
        .hCursor = LoadCursorW(NULL, IDC_ARROW),
        .hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1),
        .lpfnWndProc = ChildProc2,
        .lpszClassName = L"ChildClass2",
    };
    RegisterClassW(&child2);

    WNDCLASSW child3 = {
        .hInstance = hInst,
        .hCursor = LoadCursorW(NULL, IDC_ARROW),
        .hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1),
        .lpfnWndProc = ChildProc3,
        .lpszClassName = L"ChildClass3",
    };
    RegisterClassW(&child3);

    // --- Create main window ---
    HWND hwndMain = CreateWindowExW(
        0, L"MainWinClass", L"Main Window",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 1024, 768,
        NULL, NULL, hInst, NULL);

    if (!hwndMain)
        return 0;

    // --- Standard message loop ---
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}

#include <stdio.h>
#include <inttypes.h>

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <windowsx.h>

/* hack for telling WinE to create an actual X11 window
   Windows doesn't know it and ignores/cleans the lower bits
*/
#define WS_X_NATIVE 0x01

#define BOX_W 120
#define BOX_H 60

const wchar_t *PROP_NAME_XID = L"__wine_x11_whole_window";

// Window procedures for each subwindow type
LRESULT CALLBACK ChildProc1(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_PAINT:
        {
            int XID = (int)GetPropW(hwnd, PROP_NAME_XID);

            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            wchar_t buf[256];
            swprintf(buf, sizeof(buf)/sizeof(buf[0]), L"WIN: 0x%X", (uintptr_t)hwnd);
            TextOutW(hdc, 5, 5, buf, wcslen(buf));
            swprintf(buf, sizeof(buf)/sizeof(buf[0]), L"XID: 0x%X", XID);
            TextOutW(hdc, 5, 20, buf, wcslen(buf));
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
    switch (msg)
    {
        case WM_CREATE:
        {
            int flags = WS_CHILD | WS_VISIBLE | WS_BORDER | WS_OVERLAPPED;
            HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;

            /* store the hInstance for later use */
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)hInst);

            /* Create 3 child windows, each with its own class and title */
            CreateWindowExW(0, L"ChildClass1", L"Child Window #1",
                            flags,
                            5, 10, BOX_W, BOX_H, hwnd, NULL, hInst, NULL);

            CreateWindowExW(0, L"ChildClass2", L"Child Window #2",
                            flags,
                            5, 113, BOX_W, BOX_H, hwnd, NULL, hInst, NULL);

            CreateWindowExW(0, L"ChildClass3", L"Child Window #3",
                            flags,
                            5, 213, BOX_W, BOX_H, hwnd, NULL, hInst, NULL);

            return 0;
        }

        case WM_RBUTTONDOWN: {
            int flags = WS_CHILD | WS_VISIBLE | WS_BORDER | WS_OVERLAPPED | WS_X_NATIVE;
            HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            HWND sub3 = CreateWindowExW(0, L"ChildClass1", L"Child Window #4",
                            flags,
                            BOX_W + 10, 10, 600, 600, hwnd, NULL, hInst, NULL);

            wchar_t buffer[1024];
            wsprintfW(buffer, L"RBUTTONDOWN SUB3=%p", sub3);
            SetWindowTextW(hwnd, buffer);
            return 0;
        }

        case WM_MOUSEMOVE: {
            wchar_t buffer[64];
            wsprintfW(buffer, L"Mouse at %d, %d -- XID=0x%X", 
                GET_X_LPARAM(lParam),
                GET_Y_LPARAM(lParam),
                (int)GetPropW(hwnd, PROP_NAME_XID));
            SetWindowTextW(hwnd, buffer);
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
    (void)nShowCmd;
    (void)hPrev;
    (void)lpCmdLine;

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

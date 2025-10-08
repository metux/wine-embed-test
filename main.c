#include <stdio.h>
#include <inttypes.h>

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <windowsx.h>

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

            wchar_t buf[32];  // enough for "0x" + 16 hex digits + NUL
            swprintf(buf, sizeof(buf)/sizeof(buf[0]), L"HWND: 0x% XID: 0x%X" PRIxPTR, (uintptr_t)hwnd, XID);
            TextOutW(hdc, 10, 10, buf, wcslen(buf));
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

            /* store the hInstance for later use */
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)hInst);

            // Create 3 child windows, each with its own class and title
            CreateWindowExW(0, L"ChildClass1", L"Child Window #1",
                            flags,
                            13, 13, 211, 80, hwnd, NULL, hInst, NULL);

            CreateWindowExW(0, L"ChildClass2", L"Child Window #2",
                            flags,
                            13, 113, 211, 80, hwnd, NULL, hInst, NULL);

            CreateWindowExW(0, L"ChildClass3", L"Child Window #3",
                            flags,
                            13, 213, 211, 80, hwnd, NULL, hInst, NULL);

            return 0;
        }

        case WM_RBUTTONDOWN: {
            HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            HWND sub3 = CreateWindowExW(0, L"ChildClass1", L"Child Window #4",
                            flags,
                            13, 13, 400, 400, hwnd, NULL, hInst, NULL);

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

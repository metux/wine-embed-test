#define UNICODE
#include <windows.h>

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
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            TextOutW(hdc, 10, 10, L"This is Child 1", 15);
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
            HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;

            // Create 3 child windows, each with its own class and title
            CreateWindowExW(0, L"ChildClass1", L"Child Window #1",
                            WS_CHILD | WS_VISIBLE | WS_BORDER,
                            10, 10, 200, 80, hwnd, NULL, hInst, NULL);

            CreateWindowExW(0, L"ChildClass2", L"Child Window #2",
                            WS_CHILD | WS_VISIBLE | WS_BORDER,
                            10, 100, 200, 80, hwnd, NULL, hInst, NULL);

            CreateWindowExW(0, L"ChildClass3", L"Child Window #3",
                            WS_CHILD | WS_VISIBLE | WS_BORDER,
                            10, 190, 200, 80, hwnd, NULL, hInst, NULL);
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
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = MainProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"MainWinClass";
    RegisterClassW(&wc);

    // --- Register 3 different child window classes ---
    WNDCLASSW child = {0};
    child.hInstance = hInst;
    child.hCursor = LoadCursorW(NULL, IDC_ARROW);
    child.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);

    child.lpfnWndProc = ChildProc1;
    child.lpszClassName = L"ChildClass1";
    RegisterClassW(&child);

    child.lpfnWndProc = ChildProc2;
    child.lpszClassName = L"ChildClass2";
    RegisterClassW(&child);

    child.lpfnWndProc = ChildProc3;
    child.lpszClassName = L"ChildClass3";
    RegisterClassW(&child);

    // --- Create main window ---
    HWND hwndMain = CreateWindowExW(
        0, L"MainWinClass", L"Main Window",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 350,
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

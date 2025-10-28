#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <windowsx.h>
#include <wininet.h>

#pragma comment(lib, "wininet.lib")

/* hack for telling WinE to create an actual X11 window
   Windows doesn't know it and ignores/cleans the lower bits
*/
#define WS_X_NATIVE 0x01

#define BOX_W 140
#define BOX_H 60

#define FRAME_W 1600
#define FRAME_H 960

#define CONTAINER_W 1400
#define CONTAINER_H 900

#define BROWSERD_URL L"http://localhost:8080"
#define BROWSERD_SLOT 1

#define CLS_TAB L"mtxTab"
#define CLS_CONTAINER L"mtxViewer"

#define NUM_TABS 3

const wchar_t *PROP_NAME_XID = L"__wine_x11_whole_window";

typedef struct {
    HWND tab_window;
    HWND container_window;
    bool visible;
    bool container_created;
} TabRec;

TabRec tabs[NUM_TABS] = { 0 };

int current_tab = -1;

void show_tab(int idx) {
    for (int x=0; x<NUM_TABS; x++) {
        if (x == idx)
            ShowWindow(tabs[x].container_window, SW_SHOW);
        else
            ShowWindow(tabs[x].container_window, SW_HIDE);
    }
    current_tab = idx;
}

int doHttp(const LPCWSTR url)
{
    HINTERNET hInternet = InternetOpenW(L"MyWin32App", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) {
        wprintf(L"InternetOpen failed: %lu\n", GetLastError());
        return 1;
    }

    HINTERNET hConnect = InternetOpenUrlW(
        hInternet,
        url,
        NULL,
        0,
        INTERNET_FLAG_RELOAD,
        0
    );

    if (!hConnect) {
        wprintf(L"InternetOpenUrl failed: %lu\n", GetLastError());
        InternetCloseHandle(hInternet);
        return 1;
    }

    BYTE buffer[1024];
    DWORD bytesRead;

    wprintf(L"--- Response ---\n");

    while (InternetReadFile(hConnect, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0) {
        buffer[bytesRead] = 0;
        printf("%s", buffer);
    }

    wprintf(L"\n--- End ---\n");

    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);
    return 0;
}

// Window procedures for each subwindow type
LRESULT CALLBACK TabWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_PAINT:
        {
            int XID = (int)GetPropW(hwnd, PROP_NAME_XID);

            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            wchar_t buf[1024];
            int length = GetWindowTextW(hwnd, buf, sizeof(buf) / sizeof(buf[0]));
            TextOutW(hdc, 5, 5, buf, length);
            swprintf(buf, sizeof(buf)/sizeof(buf[0]), L"x WIN: 0x%X", (uintptr_t)hwnd);
            TextOutW(hdc, 5, 20, buf, wcslen(buf));
            swprintf(buf, sizeof(buf)/sizeof(buf[0]), L"x XID: 0x%X", XID);
            TextOutW(hdc, 5, 35, buf, wcslen(buf));
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_LBUTTONDOWN: {
            long id = GetWindowLongPtr(hwnd, GWLP_USERDATA);
            fprintf(stderr, "TAB CLICKED %ld\n", id);
            show_tab(id);
            return 0;
        }

    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK ContainerWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_PAINT:
        {
            int XID = (int)GetPropW(hwnd, PROP_NAME_XID);

            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            wchar_t buf[1024];
            int length = GetWindowTextW(hwnd, buf, sizeof(buf) / sizeof(buf[0]));
            TextOutW(hdc, 5, 5, buf, length);
            swprintf(buf, sizeof(buf)/sizeof(buf[0]), L"WIN: 0x%X", (uintptr_t)hwnd);
            TextOutW(hdc, 5, 20, buf, wcslen(buf));
            swprintf(buf, sizeof(buf)/sizeof(buf[0]), L"XID: 0x%X", XID);
            TextOutW(hdc, 5, 35, buf, wcslen(buf));
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_RBUTTONDOWN: {
            int XID = (int)GetPropW(hwnd, PROP_NAME_XID);
            wchar_t buffer[1024];
            const wchar_t *url = L"https%3A%2F%2Fwww.thur.de%2F%0A";
            long id = GetWindowLongPtr(hwnd, GWLP_USERDATA);

            wsprintfW(buffer, BROWSERD_URL L"/create/%ld/%X/%d/%d/%ls", id, XID, CONTAINER_W, CONTAINER_H, url);
            wprintf(L"%ls\n", buffer);

            if (doHttp(buffer) == 0)
                SetWindowTextW(hwnd, L"send browserd command");
            else
                SetWindowTextW(hwnd, L"failed connecting to browserd");

            return 0;
        }
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void create_tab(int idx, HINSTANCE hInst, HWND parent) {
    wchar_t buffer[128];

    wsprintfW(buffer, L"TAB %d", idx);

    tabs[idx].tab_window = CreateWindowExW(0,
        CLS_TAB,
        buffer,
        WS_CHILD | WS_VISIBLE | WS_BORDER | WS_OVERLAPPED,
        5,
        10 + (idx * 100),
        BOX_W, BOX_H,
        parent,
        NULL,
        hInst,
        NULL);

    wsprintfW(buffer, L"BROWSER %d", idx);

    tabs[idx].container_window = CreateWindowExW(0,
        CLS_CONTAINER,
        buffer,
        WS_CHILD | WS_BORDER | WS_OVERLAPPED | WS_X_NATIVE,
        BOX_W + 10,
        10,
        CONTAINER_W,
        CONTAINER_H,
        parent,
        NULL,
        hInst,
        NULL);

    SetWindowLongPtr(tabs[idx].container_window, GWLP_USERDATA, idx);
    SetWindowLongPtr(tabs[idx].tab_window, GWLP_USERDATA, idx);
}

// Main window procedure
LRESULT CALLBACK MainProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_CREATE:
        {
            HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;

            /* store the hInstance for later use */
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)hInst);

            create_tab(0, hInst, hwnd);
            create_tab(1, hInst, hwnd);
            create_tab(2, hInst, hwnd);

            show_tab(0);

            return 0;
        }

        case WM_RBUTTONDOWN: {
            wchar_t buffer[1024];
            wsprintfW(buffer, L"RBUTTONDOWN");
            SetWindowTextW(hwnd, buffer);
            ShowWindow(tabs[current_tab].container_window, SW_HIDE);
            return 0;
        }

        case WM_LBUTTONDOWN: {
            wchar_t buffer[1024];
            wsprintfW(buffer, L"LBUTTONDOWN");
            SetWindowTextW(hwnd, buffer);
            ShowWindow(tabs[current_tab].container_window, SW_SHOW);
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
    WNDCLASSW tabClass = {
        .hInstance = hInst,
        .hCursor = LoadCursorW(NULL, IDC_ARROW),
        .hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1),
        .lpfnWndProc = TabWindowProc,
        .lpszClassName = CLS_TAB,
    };
    RegisterClassW(&tabClass);

    WNDCLASSW containerClass = {
        .hInstance = hInst,
        .hCursor = LoadCursorW(NULL, IDC_ARROW),
        .hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1),
        .lpfnWndProc = ContainerWindowProc,
        .lpszClassName = CLS_CONTAINER,
    };
    RegisterClassW(&containerClass);

    HWND hwndMain = CreateWindowExW(
        0, L"MainWinClass", L"Main Window",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, FRAME_W, FRAME_H,
        NULL, NULL, hInst, NULL);

    if (!hwndMain)
        return 0;

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}

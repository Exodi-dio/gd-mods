
#include <Geode/Geode.hpp>

#include "windowsUtils.hpp"

#ifdef GEODE_IS_WINDOWS

HWND hwnd_window = NULL;

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    DWORD windowPid = 0;
    GetWindowThreadProcessId(hwnd, &windowPid);

    DWORD currentPid = GetCurrentProcessId();
    if (windowPid == currentPid && hwnd != GetConsoleWindow()) {
        if (!IsWindowVisible(hwnd)) {
            return TRUE;
        }

        if (GetParent(hwnd) != NULL) {
            return TRUE;
        }

        LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
        if (exStyle & WS_EX_TOOLWINDOW) {
            return TRUE;
        }

        hwnd_window = hwnd;
        return FALSE;
    }
    return TRUE;
}

HWND GDH::WindowUtils::FindMainHWND() {
    hwnd_window = NULL;
    EnumWindows(EnumWindowsProc, 0);
    return hwnd_window;
}


struct WindowStateBackup {
    HWND hwnd;
    LONG Style;
    LONG ExStyle;
    RECT ClientRect;
    POINT WindowPos;
};

void GDH::WindowUtils::setWindowBorderless(bool apply, int width, int height) {
    static WindowStateBackup backup;
    if (apply) {
        HWND hwnd = FindMainHWND();
        backup.hwnd = hwnd;
        backup.Style = GetWindowLong(hwnd, GWL_STYLE);
        backup.ExStyle = GetWindowLong(hwnd, GWL_EXSTYLE);

        RECT client;
        GetClientRect(hwnd, &client);
    
        POINT topLeft = { 0, 0 };
        ClientToScreen(hwnd, &topLeft);
        backup.WindowPos = topLeft;
    
        backup.ClientRect = client;

        SetWindowLong(hwnd, GWL_STYLE, WS_POPUP);
        SetWindowLong(hwnd, GWL_EXSTYLE, 0);
    
        SetWindowPos(hwnd, HWND_TOP, 0, 0, width, height, SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
    }
    else {
        SetWindowLong(backup.hwnd, GWL_STYLE, backup.Style);
        SetWindowLong(backup.hwnd, GWL_EXSTYLE, backup.ExStyle);
    
        RECT adjusted = backup.ClientRect;
        AdjustWindowRectEx(&adjusted, backup.Style, FALSE, backup.ExStyle);
    
        int width = adjusted.right - adjusted.left;
        int height = adjusted.bottom - adjusted.top;
    
        SetWindowPos(backup.hwnd, HWND_TOP, backup.WindowPos.x, backup.WindowPos.y, width, height, SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
    }
}

#endif

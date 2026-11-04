#pragma once
#ifdef GEODE_IS_WINDOWS
#include <Windows.h>

namespace GDH { namespace WindowUtils {
    HWND FindMainHWND();
    void setWindowBorderless(bool apply = false, int width = 1920, int height = 1080);
} }
#endif
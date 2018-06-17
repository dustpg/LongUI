// debug
#include <windows.h>

#ifndef DEBUG
void ui_debug_output_info(const char* str) {
    OutputDebugStringA(str);
}
void ui_debug_output_infow(const wchar_t* str) {
    OutputDebugStringW(str);
}
#endif

void longui_error_beep() {
    MessageBeep(MB_ICONERROR);
}

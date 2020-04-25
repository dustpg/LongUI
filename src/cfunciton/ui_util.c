#include <windows.h>
#include <shellapi.h>

#ifndef DEBUG
void ui_debug_output_info(const char* str) {
    OutputDebugStringA(str);
}
void ui_debug_output_infow(const wchar_t* str) {
    OutputDebugStringW(str);
}
#endif

void longui_error_beep() {
    MessageBeep(MB_ICONWARNING);
    //MessageBeep(MB_ICONERROR);
}

void longui_open_href(const char* href) {
    ShellExecuteA(NULL, NULL, href, NULL, NULL, SW_NORMAL);
}
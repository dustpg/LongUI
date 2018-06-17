#pragma once

// HANDLE FOR WINDOWS
#define LUI_DECLARE_HANDLE(name) struct name##__; using name = name##__ *;
LUI_DECLARE_HANDLE(HWND);
LUI_DECLARE_HANDLE(HRAWINPUT);
#undef LUI_DECLARE_HANDLE


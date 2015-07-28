#define LONGUI_WITH_DEFAULT_HEADER
#define _CRT_SECURE_NO_WARNINGS
#include "../../LongUI/LongUI.h"

constexpr char* hello_world = 
u8R"(<?xml version="1.0" encoding="utf-8"?><Window>
    <Text text="Hello, world!" />
</Window>
)";

int WINAPI WinMain(HINSTANCE, HINSTANCE, char*, int) {
    if (SUCCEEDED(::OleInitialize(nullptr))) {
        if (SUCCEEDED(UIManager.Initialize())) {
            UIManager.CreateUIWindow(hello_world);
            UIManager.Run();
        }
        UIManager.UnInitialize();
        ::OleUninitialize();
    }
    return 0;
}

#ifdef _MSC_VER

#ifdef _DEBUG
#   pragma comment(lib, "../../Debug/longui")
#else
#   pragma comment(lib, "../../Release/longui")
#endif


#ifndef PUGIXML_HEADER_ONLY
#ifdef _DEBUG
#   pragma comment(lib, "../../Debug/pugixml")
#else
#   pragma comment(lib, "../../Release/pugixml")
#endif
#endif

#ifdef _DEBUG
#   pragma comment(lib, "../../Debug/dlmalloc")
#else
#   pragma comment(lib, "../../Release/dlmalloc")
#endif

#endif
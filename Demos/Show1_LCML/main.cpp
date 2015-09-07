#define LONGUI_WITH_DEFAULT_HEADER
#define _CRT_SECURE_NO_WARNINGS
#include "LongUI.h"

// XML Layout
constexpr char* const DEMO_XML = 
u8R"(<?xml version="1.0" encoding="utf-8"?>
<Window>
    <HorizontalLayout margin="4,4,4,4" borderwidth="1">
        <Text text="Hello, world!" margin="4,4,4,4" borderwidth="1"/>
        <Text text="Hello, world!" margin="4,4,4,4" borderwidth="1"/>
        <Text text="Hello, world!" margin="4,4,4,4" borderwidth="1"/>
    </HorizontalLayout>
    <HorizontalLayout margin="4,4,4,4" borderwidth="1">
        <Text text="Hello, world!" margin="4,4,4,4" borderwidth="1"/>
        <Text text="Hello, world!" margin="4,4,4,4" borderwidth="1"/>
        <Text text="Hello, world!" margin="4,4,4,4" borderwidth="1"/>
    </HorizontalLayout>
</Window>
)";
// Entry for App
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char* lpCmdLine, int nCmdShow) {
    // every windows desktop app should do this
    ::HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);
    // use OleInitialize to init ole and com
    if (SUCCEEDED(::OleInitialize(nullptr))) {
        // init longui manager
        if (SUCCEEDED(UIManager.Initialize())) {
            // my style
            UIManager << DL_Hint << L"Battle Control Online!" << LongUI::endl;
            // create main window, return nullptr for some error
            UIManager.CreateUIWindow(DEMO_XML);
            // run this app
            UIManager.Run();
            // my style
            UIManager << DL_Hint << L"Battle Control Terminated!" << LongUI::endl;
        }
        // cleanup longui
        UIManager.UnInitialize();
    }
    // cleanup ole and com
    ::OleUninitialize();
    // exit
    return EXIT_SUCCESS;
}


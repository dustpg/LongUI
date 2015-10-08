#define LONGUI_WITH_DEFAULT_HEADER
#define _CRT_SECURE_NO_WARNINGS
#include "LongUI.h"
#include "demo.h"

// Entry for App
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char* lpCmdLine, int nCmdShow) {
    const wchar_t* message = L"There are 3 ways to handle event:\r\n"
        L"  1. script\r\n"
        L"  2. callback\r\n"
        L"  3. doevent\r\n"
        L"see comment for more detail";
    ::MessageBoxW(nullptr, message, L"Hint", MB_OK);
    /*
        1. if script(IUIScript interface and ScriptUI data).
            call IUIScript::Evaluation
        2. if UICallBack.IsOK
            call it
        3. end, send event to window of sender
        see: LongUI::UIControl::call_uievent to get more detail
    */
    // every windows desktop app should do this
    ::HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);
    // use OleInitialize to init ole and com
    if (SUCCEEDED(::OleInitialize(nullptr))) {
        // init longui manager
        if (SUCCEEDED(UIManager.Initialize())) {
            // my style
            UIManager << DL_Hint << L"Battle Control Online!" << LongUI::endl;
            // create main window, return nullptr for some error
            UIManager.CreateUIWindow<LongUI::Demo::MainWindow>(DEMO_XML);
            // run this app
            UIManager.Run();
            // my style
            UIManager << DL_Hint << L"Battle Control Terminated!" << LongUI::endl;
        }
        // cleanup longui
        UIManager.Uninitialize();
    }
    // cleanup ole and com
    ::OleUninitialize();
    // exit
    return EXIT_SUCCESS;
}


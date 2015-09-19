#define LONGUI_WITH_DEFAULT_HEADER
#define _CRT_SECURE_NO_WARNINGS
#include "LongUI.h"
#include "demo.h"

// Entry for App
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char* lpCmdLine, int nCmdShow) {
    const wchar_t* message = L"There are 3 ways to handle e:\r\n"
        L"  1. script   - High  Priority\r\n"
        L"  2. callback - Normal Priority\r\n"
        L"  3. e    - Low   Priority\r\n"
        L"see comment for more detail";
    ::MessageBoxW(nullptr, message, L"Hint", MB_OK);
    /*
        1. if script(IUIScript interface and ScriptUI data).
            call IUIScript::Evaluation
        2. if callback(CUISubEventCaller)
            call it
        3. other way, send event to window of sender
        see: LongUI::CUISubEventCaller::operator() get detail
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
        UIManager.UnInitialize();
    }
    // cleanup ole and com
    ::OleUninitialize();
    // exit
    return EXIT_SUCCESS;
}


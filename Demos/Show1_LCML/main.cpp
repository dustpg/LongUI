#define LONGUI_WITH_DEFAULT_HEADER
#define _CRT_SECURE_NO_WARNINGS
#include "LongUI.h"

// longui::demo namespace
LONGUI_NAMESPACE_BEGIN namespace Show {
    // config
    class MyConfig final : public CUIDefaultConfigure {
        // super class
        using Super = CUIDefaultConfigure;
    public:
        // ctor
        MyConfig() : Super(UIManager) { }
        // locale name
        auto GetLocaleName(wchar_t name[/*LOCALE_NAME_MAX_LENGTH*/]) noexcept ->void override {
            std::wcscpy(name, L"en-us");
        };
        // return flags
        auto GetConfigureFlag() noexcept ->ConfigureFlag override { 
            return Flag_OutputDebugString | Flag_RenderByCPU | Flag_DbgOutputFontFamily;
        }
        // create interface
        auto CreateInterface(const IID& iid, void** obj) noexcept ->HRESULT override {
            // bind it
            auto create_fc = [](const wchar_t* fl) noexcept {
                return LongUI::DX::CreateFontCollection(L"*.*tf", fl); 
            };
            // IDWriteFontCollection 字体集合
            if (iid == LongUI::GetIID<IDWriteFontCollection>()) {
                // check current fonts
                auto collection = create_fc(L"fonts");
                if (!collection) {
                    // check current fonts
                    collection = create_fc(LR"(..\..\fonts)");
                }
                *obj = collection;
                return collection ? S_OK : E_FAIL;
            }
            // super will do it
            return Super::CreateInterface(iid, obj);
        }
    };
}
LONGUI_NAMESPACE_END

// XML Layout
constexpr char* const DEMO_XML = 
u8R"(<?xml version="1.0" encoding="utf-8"?>
<Window>
    <HorizontalLayout margin="4,4,4,4" borderwidth="1">
        <Text text="Hello, world!楷体楷體" margin="4,4,4,4" borderwidth="1" textfamily="KaiTi"/>
        <Text text="Hello, world!" margin="4,4,4,4" borderwidth="1" textsize="60" textfamily="Jellyka BeesAntique Handwriting"/>
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
        // config
        LongUI::Show::MyConfig config;
        // init longui manager
        if (SUCCEEDED(UIManager.Initialize(&config))) {
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
        UIManager.Uninitialize();
    }
    // cleanup ole and com
    ::OleUninitialize();
    // exit
    return EXIT_SUCCESS;
}


#define _CRT_SECURE_NO_WARNINGS
#include <LongUI.h>
#include "../UIBlurText/UIBlurText.h"

static const char* const BLURTEXT_LAYOUT = 
u8R"xml(<?xml version="1.0" encoding="utf-8"?>
<Window size="512, 256" debug="false" titlename="LongUI Blur-Text Window">
    <BlurText name="txtBlur" blur="3" textrichtype="xml"
        text="&lt;font size=&quot;40&quot;&gt;Gaussian Blur&lt;/font&gt;Text"/>
    <HorizontalLayout size="0,46">
        <Slider name="sldBlur" end="10" value="3" thumbsize="32,32" 
            margin="4,4,4,4" />
        <Button name="btnAuto" text="Auto Test" size="-1, 0"
            borderwidth="1" margin="4,4,4,4" />
    </HorizontalLayout>
</Window>
)xml";

// init the window
void InitWindow(LongUI::XUIBaseWindow*, int nCmdShow) noexcept;

// Entry for App
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char* lpCmdLine, int nCmdShow) {
    // every windows desktop app should do this
    ::HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);
    // use OleInitialize to init ole and com
    if (SUCCEEDED(::OleInitialize(nullptr))) {
        // init longui manager
        if (SUCCEEDED(UIManager.Initialize())) {
            // register class
            UIManager.RegisterControlClass(LongUI::UIBlurText::CreateControl, "BlurText");
            // my style
            UIManager << DL_Hint << L"Battle Control Online!" << LongUI::endl;
            // create main window, return nullptr for some error
            ::InitWindow(UIManager.CreateUIWindow(BLURTEXT_LAYOUT), nCmdShow);
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


/// <summary>
/// Initializes the window.
/// </summary>
/// <param name="window">The window.</param>
/// <param name="nCmdShow">The n command show.</param>
/// <returns></returns>
void InitWindow(LongUI::XUIBaseWindow* window, int nCmdShow) noexcept {
    if (!window) {
        auto msg = L" May: 1.OOM 2.Xml 3.Dxgi";
        ::MessageBoxW(nullptr, msg, L"Create Window Failed", MB_ICONERROR);
        return;
    }
    if (const auto slider = window->FindControl("sldBlur")) {
        auto txt = window->FindControl("txtBlur");
        assert(txt && "control not found");
        auto blurtext = LongUI::longui_cast<LongUI::UIBlurText*>(txt);
        slider->Add_OnChanged([=](LongUI::UIControl* s) noexcept {
            auto sd = s->GetFloat();
            blurtext->SetBlurValue(sd);
            //window->InvalidateWindow();
            return true;
        });
    }
    if (const auto button = window->FindControl("btnAuto")) {
        auto slider = window->FindControl("sldBlur");
        assert(slider && "control not found");
        button->Add_OnClicked([=](LongUI::UIControl* s) noexcept {
            UIManager.AddTimeCapsule([=](float t) noexcept {
                auto type = LongUI::AnimationType::Type_CubicEaseOut;
                t = LongUI::EasingFunction(type, t);
                t *= 100.f;
                t -= float(int(t / 10.f)) * 10.f;
                slider->SetFloat(t);
                return false;
            }, s, 10.f);
            return true;
        });
    }
    window->ShowWindow(nCmdShow);
}

#pragma comment(lib, "longui")
#pragma comment(lib, "pugixml")
#pragma comment(lib, "dlmalloc")
#pragma comment(lib, "UIBlurText")

#define _CRT_SECURE_NO_WARNINGS
#include <LongUI.h>
#include <LongUI/luiUiDConf.h>
#include "../UIColorHsv/UIColorHsv.h"
#include <Control/UIColor.h>

static const char* const BLURTEXT_LAYOUT = 
u8R"xml(<?xml version="1.0" encoding="utf-8"?>
<Window size="800, 600" titlename="LongUI HSV Color Picker">
    <HorizontalLayout >
        <ColorHSV name="hsvMain" s="sldS" v="sldV"/>
        <Color color="#F00" name="colShow" weight="0.3"/>
    </HorizontalLayout >
    <HorizontalLayout size="0,64">
        <Slider name="sldS" value="1" thumbsize="32,32" 
            margin="4,4,4,4" />
        <Slider name="sldV" value="1" thumbsize="32,32" 
            margin="4,4,4,4" />
        <Button name="btnAuto" text="测试" size="-1, 0" 
            textreadingdirection="top2bottom" textflowdirection="right2left"
            borderwidth="1" margin="4,4,4,4" />
    </HorizontalLayout>
</Window>
)xml";

// init the window
void InitWindow(LongUI::XUIBaseWindow*, int nCmdShow) noexcept;

// hsv
struct HsvConfig : LongUI::CUIDefaultConfigure {
    LONGUI_BASIC_INTERFACE_IMPL;
    /*auto GetTemplateString() noexcept ->const char* override {
        return BLURTEXT_TEMPLATE;
    }*/
    // add all custom controls
    void RegisterSome() noexcept override {
        auto cc = LongUI::UIColorHsv::CreateControl;
        UIManager.RegisterControlClass(cc, "ColorHSV");
    };
    // choose
    auto ChooseAdapter(const DXGI_ADAPTER_DESC1 adapters[], 
        const size_t length) noexcept -> size_t override {
        // A记显卡
        for (size_t i = 0; i < length; ++i) {
            if (*adapters[i].Description == 'A') return i;
        }
        // M记显卡
        for (size_t i = 0; i < length; ++i) {
            if (*adapters[i].Description == 'M') return i;
        }
        // I记显卡
        for (size_t i = 0; i < length; ++i) {
            if (*adapters[i].Description == 'I') return i;
        }
        // N记显卡
        for (size_t i = 0; i < length; ++i) {
            if (*adapters[i].Description == 'N') return i;
        }
        return length;
    }
    HsvConfig() : CUIDefaultConfigure(UIManager) {
        //this->resource = BLURTEXT_RESOURCE;
    }
};

// Entry for App
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char* lpCmdLine, int nCmdShow) {
    // every windows desktop app should do this
    ::HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);
    // use OleInitialize to init ole and com
    if (SUCCEEDED(::OleInitialize(nullptr))) {
        // config
        HsvConfig config;
        // init longui manager
        if (SUCCEEDED(UIManager.Initialize(&config))) {
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
    using namespace LongUI;
    auto hsv = longui_cast<UIColorHsv*>(window->FindControl("hsvMain"));
    assert(hsv && "control not found");
    // hsv
    if(const auto c = window->FindControl("colShow")){
        auto color = longui_cast<UIColor*>(c);
        hsv->Add_OnChanged([=](LongUI::UIControl*) noexcept {
            auto get_alpha = []() noexcept {return 1.f; };
            auto base = hsv->PickColor();
            base.a = get_alpha();
            color->SetColor(base);
            return true;
        });
    }
    // S
    if (const auto slider = window->FindControl("sldS")) {
        slider->Add_OnChanged([=](LongUI::UIControl* sc) noexcept {
            hsv->SetS(sc->GetFloat());
            return true;
        });
    }
    // S
    if (const auto slider = window->FindControl("sldV")) {
        slider->Add_OnChanged([=](LongUI::UIControl* sc) noexcept {
            hsv->SetV(sc->GetFloat());
            return true;
        });
    }
    // BTN
    if (const auto button = window->FindControl("btnAuto")) {
        button->Add_OnClicked([=](LongUI::UIControl* s) noexcept {
            UIManager.AddTimeCapsule([=](float t) noexcept {
                auto type = LongUI::AnimationType::Type_CubicEaseOut;
                t = LongUI::EasingFunction(type, t);
                t *= 40.f;
                t -= float(int(t / 10.f)) * 10.f;
                hsv->SetHue(t * 36.f);
                return false;
            }, s, 4.f);
            return true;
        });
    }
    window->ShowWindow(nCmdShow);
}

#pragma comment(lib, "longui")
#pragma comment(lib, "pugixml")
#pragma comment(lib, "dlmalloc")
#pragma comment(lib, "UIColorHsv")
#pragma comment(lib, "D3DCompiler")

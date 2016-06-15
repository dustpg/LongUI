#define _CRT_SECURE_NO_WARNINGS
#include <algorithm>
#include <LongUI.h>
#include <LongUI/luiUiDConf.h>
#include <Control/UIColor.h>
#include "../UIColorHsv/UIColorHsv.h"
#include "../UIColorHsv/UIColorButton.h"
#include "colorpicker.h"

const char* const DEMO_LAYOUT = u8R"xml(<?xml version="1.0" encoding="utf-8"?>
<Window size="800, 600" titlename="Choose Color Demo" margin="8,8,8,8" >
    <HorizontalLayout>
        <Button name="btnA" textsize="30" margin="4,4,4,4" borderwidth="1" text="Click Me"/>
        <Button name="btnB" textsize="30" margin="4,4,4,4" borderwidth="1" text="Click Me"/>
        <Button name="btnC" textsize="30" margin="4,4,4,4" borderwidth="1" text="Click Me"/>
        <Button name="btnD" textsize="30" margin="4,4,4,4" borderwidth="1" text="Click Me"/>
    </HorizontalLayout>
    <HorizontalLayout>
        <Button name="btnE" textsize="30" margin="4,4,4,4" borderwidth="1" text="Click Me"/>
        <Button name="btnF" textsize="30" margin="4,4,4,4" borderwidth="1" text="Click Me"/>
        <Button name="btnG" textsize="30" margin="4,4,4,4" borderwidth="1" text="Click Me"/>
        <Button name="btnH" textsize="30" margin="4,4,4,4" borderwidth="1" text="Click Me"/>
    </HorizontalLayout>
    <HorizontalLayout>
        <Button name="btnI" textsize="30" margin="4,4,4,4" borderwidth="1" text="Click Me"/>
        <Button name="btnJ" textsize="30" margin="4,4,4,4" borderwidth="1" text="Click Me"/>
        <Button name="btnK" textsize="30" margin="4,4,4,4" borderwidth="1" text="Click Me"/>
        <Button name="btnL" textsize="30" margin="4,4,4,4" borderwidth="1" text="Click Me"/>
    </HorizontalLayout>
    <HorizontalLayout>
        <Button name="btnM" textsize="30" margin="4,4,4,4" borderwidth="1" text="Click Me"/>
        <Button name="btnN" textsize="30" margin="4,4,4,4" borderwidth="1" text="Click Me"/>
        <Button name="btnO" textsize="30" margin="4,4,4,4" borderwidth="1" text="Click Me"/>
        <Button name="btnP" textsize="30" margin="4,4,4,4" borderwidth="1" text="Click Me"/>
    </HorizontalLayout>
</Window>
)xml";

// init the window
void InitWindow(
    LongUI::XUIBaseWindow*, 
    LongUI::CUIColorPicker&,
    int nCmdShow
) noexcept;

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
        cc = LongUI::UIColorButton::CreateControl;
        UIManager.RegisterControlClass(cc, "ColorButton");
    };
    // choose
    auto ChooseAdapter(const DXGI_ADAPTER_DESC1 adapters[], 
        const size_t length) noexcept -> size_t override {
        // M记显卡
        for (size_t i = 0; i < length; ++i) {
            if (*adapters[i].Description == 'M') return i;
        }
        // A记显卡
        for (size_t i = 0; i < length; ++i) {
            if (*adapters[i].Description == 'A') return i;
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
        HsvConfig config; LongUI::CUIColorPicker picker;
        // init longui manager
        if (SUCCEEDED(UIManager.Initialize(&config))) {
            // my style
            UIManager << DL_Hint << L"Battle Control Online!" << LongUI::endl;
            // create main window, return nullptr for some error
            //picker.Create(nullptr); picker.ShowWindow(nCmdShow);
            ::InitWindow(UIManager.CreateUIWindow(DEMO_LAYOUT), picker, nCmdShow);
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
/// <param name="picker">The picker.</param>
/// <param name="nCmdShow">The n command show.</param>
/// <returns></returns>
void InitWindow(
    LongUI::XUIBaseWindow* window, 
    LongUI::CUIColorPicker& picker,
    int nCmdShow) noexcept {
    using namespace LongUI;
    if (!window) {
        auto msg = L" May: 1.OOM 2.Xml 3.Dxgi";
        ::MessageBoxW(nullptr, msg, L"Create Window Failed", MB_ICONERROR);
        return;
    }
    // button commnad
    auto command = [=, &picker](UIControl* btn) noexcept {
        auto text = longui_cast<UIText*>(btn);
        // set the callback on color picked
        picker.SetCallback([=](const D2D1_COLOR_F* color) noexcept {
            // on ok
            if (color) {
                text->SetBasicColor(State_Normal, *color);
                text->SetBasicColor(State_Hover,  *color);
                text->SetBasicColor(State_Pushed, *color);
            }
            // on cancel
#ifdef _DEBUG
            else {
                UIManager << DL_Log << L"On Cancel" << endl;
            }
#endif
            return true;
        });
        // create the picker window
        if (picker.Create(window)) {
            D2D1_COLOR_F oldc{ 0.f,0.f,0.f,1.f };
            text->GetBasicColor(State_Normal, oldc);
            picker.SetText(picker.Index_OK, L"确定: R U OK?");
            picker.SetText(picker.Index_Old, L"之前");
            picker.SetInitColor(oldc);
            picker.ShowWindow(nCmdShow);
        }
        return true;
    };
    // set event
    constexpr int count = 16; char name[] = "btn ";
    for (int i = 0; i < count; ++i) {
        name[3] = char(i) + 'A';
        auto btn = window->FindControl(name);
        assert(btn && "control not found");
        btn->Add_OnClicked(command);
    }
    window->ShowWindow(nCmdShow);
}

#pragma comment(lib, "lz4")
#pragma comment(lib, "longui")
#pragma comment(lib, "pugixml")
#pragma comment(lib, "dlmalloc")
#pragma comment(lib, "UIColorHsv")
#pragma comment(lib, "D3DCompiler")

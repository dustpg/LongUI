#define _CRT_SECURE_NO_WARNINGS
#include <algorithm>
#include <LongUI.h>
#include <LongUI/luiUiDConf.h>
#include <Control/UIColor.h>
#include "../UIColorHsv/UIColorHsv.h"
#include "../UIColorHsv/UIColorButton.h"
#include "colorpicker.h"

static const char* const BLURTEXT_LAYOUT = 
u8R"xml(<?xml version="1.0" encoding="utf-8"?>
<Window size="800, 600" titlename="LongUI HSV Color Picker">
    <HorizontalLayout>
        <ColorHSV weight="2" name="hsvMain" h="sldH" s="sldS" v="sldV"
            r="sldR" g="sldG" b="sldB"/>
        <VerticalLayout size="32, 0">
            <Text text="H"/>
            <Text text="S"/>
            <Text text="V"/>
            <Null/>
            <Text text="R"/>
            <Text text="G"/>
            <Text text="B"/>
            <Null/>
            <Text text="A"/>
        </VerticalLayout>
        <VerticalLayout>
            <Slider name="sldH" end="360" thumbsize="8,16" margin="4,4,4,4" />
            <Slider name="sldS" value="1" thumbsize="8,16" margin="4,4,4,4" />
            <Slider name="sldV" value="1" thumbsize="8,16" margin="4,4,4,4" />
            <Null/>
            <Slider name="sldR" value="1" thumbsize="8,16" margin="4,4,4,4" />
            <Slider name="sldG" value="0" thumbsize="8,16" margin="4,4,4,4" />
            <Slider name="sldB" value="0" thumbsize="8,16" margin="4,4,4,4" />
            <Null/>
            <Slider name="sldA" value="1" thumbsize="8,16" margin="4,4,4,4" />
        </VerticalLayout>
        <VerticalLayout size="64,0">
            <Edit name="edtH" text="0.00" margin="4,4,4,4" borderwidth="1"/>
            <Edit name="edtS" text="1.00" margin="4,4,4,4" borderwidth="1"/>
            <Edit name="edtV" text="1.00" margin="4,4,4,4" borderwidth="1"/>
            <Null/>
            <Edit name="edtR" text="1.00" margin="4,4,4,4" borderwidth="1"/>
            <Edit name="edtG" text="0.00" margin="4,4,4,4" borderwidth="1"/>
            <Edit name="edtB" text="0.00" margin="4,4,4,4" borderwidth="1"/>
            <Null/>
            <Edit name="edtA" text="1.00" margin="4,4,4,4" borderwidth="1"/>
        </VerticalLayout>
        <VerticalLayout size="32, 0">
            <Text name="txtH" text="0"/>
            <Text name="txtS" text="100"/>
            <Text name="txtV" text="100"/>
            <Null/>
            <Text name="txtR" text="255"/>
            <Text name="txtG" text=  "0"/>
            <Text name="txtB" text=  "0"/>
            <Null/>
            <Text name="txtA" text="255"/>
        </VerticalLayout>
    </HorizontalLayout>
    <HorizontalLayout weight="0.3">
        <Color color="RED" direct="true" name="colShowA"/>
        <Color color="blue" name="colShowB"/>
        <ColorButton color="#6CF" colorpadding="8,8,8,8"
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
            picker.Create(nullptr); picker.ShowWindow(nCmdShow);
            //::InitWindow(UIManager.CreateUIWindow(BLURTEXT_LAYOUT), nCmdShow);
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
    window->ShowWindow(nCmdShow);
}

#pragma comment(lib, "lz4")
#pragma comment(lib, "longui")
#pragma comment(lib, "pugixml")
#pragma comment(lib, "dlmalloc")
#pragma comment(lib, "UIColorHsv")
#pragma comment(lib, "D3DCompiler")

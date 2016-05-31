#define _CRT_SECURE_NO_WARNINGS
#include <LongUI.h>
#include <LongUI/luiUiDConf.h>

extern const char* BLURTEXT_XXLAYOUT;
extern const char* BLURTEXT_TEMPLATE;
extern const char* BLURTEXT_RESOURCE;

struct XmlConfig : LongUI::CUIDefaultConfigure {
    LONGUI_BASIC_INTERFACE_IMPL;
    auto GetTemplateString() noexcept ->const char* override {
        return BLURTEXT_TEMPLATE;
    }
    XmlConfig() : CUIDefaultConfigure(UIManager) {
        this->resource = BLURTEXT_RESOURCE;
    }
};

// init the window
void InitWindow(LongUI::XUIBaseWindow*, int nCmdShow) noexcept;

// Entry for App
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char* lpCmdLine, int nCmdShow) {
    // every windows desktop app should do this
    ::HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);
    // use OleInitialize to init ole and com
    if (SUCCEEDED(::OleInitialize(nullptr))) {
        // config
        XmlConfig conifg;
        // init longui manager
        if (SUCCEEDED(UIManager.Initialize(&conifg))) {
            // my style
            UIManager << DL_Hint << L"Battle Control Online!" << LongUI::endl;
            // create main window, return nullptr for some error
            ::InitWindow(UIManager.CreateUIWindow(BLURTEXT_XXLAYOUT), nCmdShow);
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

#include <Control/UIEdit.h>

/// <summary>
/// Initializes the window.
/// </summary>
/// <param name="window">The window.</param>
/// <param name="nCmdShow">The n command show.</param>
/// <returns></returns>
void InitWindow(LongUI::XUIBaseWindow* window, int nCmdShow) noexcept {
    using namespace LongUI;
    if (!window) {
        auto msg = L" May: 1.OOM 2.Xml 3.Dxgi";
        ::MessageBoxW(nullptr, msg, L"Create Window Failed", MB_ICONERROR);
        return;
    }
    auto input = longui_cast<UIEdit*>(window->FindControl("i"));
    auto output = longui_cast<UIEdit*>(window->FindControl("o"));
    assert(input && output);
    // bold
    if (const auto b = window->FindControl("btnB")) {
        auto&icomponent = input->GetComponent();
        auto&i = *input; auto&o = *output;
        b->Add_OnClicked([&](UIControl*) noexcept {
            auto range = icomponent.GetSelectionRange();
            if (range.length) {
                icomponent.layout->SetFontWeight(
                    DWRITE_FONT_WEIGHT_BOLD, range
                );
                i.InvalidateThis();
            }
            return true;
        });
    }
    // underline
    if (const auto u = window->FindControl("btnU")) {
        auto&icomponent = input->GetComponent();
        auto&i = *input; auto&o = *output;
        u->Add_OnClicked([&](UIControl*) noexcept {
            auto range = icomponent.GetSelectionRange();
            if (range.length) {
                icomponent.layout->SetUnderline(true, range);
                i.InvalidateThis();
            }
            return true;
        });
    }
    window->ShowWindow(nCmdShow);
}

#pragma comment(lib, "longui")
#pragma comment(lib, "pugixml")
#pragma comment(lib, "dlmalloc")
#pragma comment(lib, "UIBlurText")


const char* BLURTEXT_XXLAYOUT = 
u8R"xml(<?xml version="1.0" encoding="utf-8"?>
<Window size="800, 600" debug="false" titlename="LongUI Xml Rich Editor">
    <HorizontalLayout>
        <VerticalLayout>
            <HorizontalLayout size="0,46">
                <Button name="btnA" templateid="1"
    text="&lt;font face=&quot;Courier New&quot;&gt;A&lt;/font&gt; A" />
                <Button name="btnT" templateid="1"
    text="&lt;font size=&quot;12&quot;&gt;T&lt;/font&gt;T" />
                <Button name="btnC" templateid="1"
    text="&lt;font color=&quot;#F00&quot;&gt;C&lt;/font&gt;" />
                <Button name="btnR" templateid="1"
    text="&lt;ruby str=&quot;ɑː(r)&quot;&gt;R&lt;/ruby&gt;" />
                <Button name="btnB" templateid="1" text="&lt;b&gt;B&lt;/b&gt;"/>
                <Button name="btnI" templateid="1" text="&lt;i&gt;I&lt;/i&gt;" />
                <Button name="btnU" templateid="1" text="&lt;u&gt;U&lt;/u&gt;" />
                <Button name="btnS" templateid="1" text="&lt;s&gt;S&lt;/s&gt;" />
            </HorizontalLayout>
            <Edit name="i" text="Hello, world!" textformat="1" textrich="true"
                textmultiline="true" borderwidth="1" margin="4,4,4,4" />
        </VerticalLayout>
        <VerticalLayout>
            <HorizontalLayout size="0,46">
                <Button templateid="2" text="Copy" />
                <Button templateid="2" text="Escape & Copy" />
            </HorizontalLayout>
            <Edit name="o" text="Hello, world!" textformat="1" borderwidth="1" 
                textmultiline="true" margin="4,4,4,4" />
        </VerticalLayout>
    </HorizontalLayout>
</Window>
)xml";


const char* BLURTEXT_TEMPLATE = 
u8R"xml(<?xml version="1.0" encoding="utf-8"?>
<Template>
    <!-- Index 1 -->
    <Control borderwidth="1" margin="4,4,4,4" textrichtype="xml" />
    <!-- Index 2 -->
    <Control borderwidth="1" margin="4,4,4,4" />
</Template>
)xml";



const char* BLURTEXT_RESOURCE = u8R"xml(<?xml version="1.0" encoding="utf-8"?>
<Resource>
    <!-- Bitmap区域Zone -->
    <Bitmap>

    </Bitmap>
    <!-- Text Format区域Zone -->
    <TextFormat>
        <!-- You can use other name not limited in 'Item' -->
        <Item  valign="top" align="left" wordwrapping="character"/>
    </TextFormat>
    <!-- Meta区域Zone -->
    <uMeta>
        <Item desc="按钮1无效图元" bitmap="1" rect="0,  0, 96, 24" rule="button"/>
        <Item desc="按钮1通常图元" bitmap="1" rect="0, 72, 96, 96" rule="button"/>
        <Item desc="按钮1悬浮图元" bitmap="1" rect="0, 24, 96, 48" rule="button"/>
        <Item desc="按钮1按下图元" bitmap="1" rect="0, 48, 96, 72" rule="button"/>

        <Item bitmap="2" rect="  0,  0, 32, 32" rule="button"/>
        <Item bitmap="2" rect=" 32,  0, 64, 32" rule="button"/>
        <Item bitmap="2" rect=" 64,  0, 96, 32" rule="button"/>
        <Item bitmap="2" rect=" 96,  0,128, 32" rule="button"/>
        <Item bitmap="2" rect="  0, 32, 32, 64" rule="button"/>
        <Item bitmap="2" rect=" 32, 32, 64, 64" rule="button"/>
        <Item bitmap="2" rect=" 64, 32, 96, 64" rule="button"/>
        <Item bitmap="2" rect=" 96, 32,128, 64" rule="button"/>
        <Item bitmap="2" rect="  0, 64, 32, 96" rule="button"/>
        <Item bitmap="2" rect=" 32, 64, 64, 96" rule="button"/>
        <Item bitmap="2" rect=" 64, 64, 96, 96" rule="button"/>
        <Item bitmap="2" rect=" 96, 64,128, 96" rule="button"/>
    </uMeta>
</Resource>
)xml";

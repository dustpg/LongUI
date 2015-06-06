#include "stdafx.h"
#include "included.h"


// 测试XML &#xD; --> \r &#xA; --> \n
#if 1
const char* test_xml = u8R"xml(<?xml version="1.0" encoding="utf-8"?>
<Window size="1024, 768" name="MainWindow" vscrollbar="ScrollBarA" hscrollbar="ScrollBarA">
    <VerticalLayout name="VLayout1" pos="0, 0, 1366, 128">
        <!--Video name="asd" /-->
        <Test name="test" texttype="core" text="%cHello%], %cworld!%]%c泥壕!%]世界!%p#0F0, #F00, #00F"/>
        <Slider name="6" renderparent="true"/>
    </VerticalLayout>
    <HorizontalLayout name="HLayout" pos="0, 0, 0, 256">
        <EditBasic name="edit01" textmultiline="true" text="Hello, world!&#xD;&#xA;泥壕, 世界!"/>
        <VerticalLayout name="VLayout2">
            <Label name="2" texttype="core" text="%cHello%], world!泥壕!世界!%p#F00"/>
            <Button name="4" disabledmeta="1" normalmeta="2" script="App.click_button1($apparg)"
                hovermeta="3" pushedmeta="4" text="Hello, world!"/>
            <CheckBox name="5" text="Hello, world!"/>
            <!--Button name="uac" disabledmeta="1" normalmeta="2" 
                hovermeta="3" pushedmeta="4" text="Try  Elevate UAC Now "/-->
        </VerticalLayout>
    </HorizontalLayout>
</Window>
)xml";
#else
const char* test_xml = u8R"xml(<?xml version="1.0" encoding="utf-8"?>
<Window size="1024, 768" name="MainWindow" >
    <VerticalLayout name="VLayout1">
        <Button name="1" disabledmeta="1" normalmeta="2" hovermeta="3" pushedmeta="4" text="Hello, world!"/>
        <Button name="2" disabledmeta="1" normalmeta="2" hovermeta="3" pushedmeta="4" text="Hello, world!"/>
    </VerticalLayout>
    <HorizontalLayout name="HLayout">
        <Button name="3" disabledmeta="1" normalmeta="2" hovermeta="3" pushedmeta="4" text="Hello, world!"/>
        <Button name="4" disabledmeta="1" normalmeta="2" hovermeta="3" pushedmeta="4" text="Hello, world!"/>
    </HorizontalLayout>
</Window>
)xml";
#endif

constexpr char* res_xml = u8R"xml(<?xml version="1.0" encoding="utf-8"?>
<Resource>
    <!-- Bitmap区域Zone -->
    <Bitmap desc="按钮1" res="btn.png"/>
    <!-- Meta区域Zone -->
    <Meta desc="按钮1无效图元" bitmap="1" rect="0,  0, 96, 24"/>
    <Meta desc="按钮1通常图元" bitmap="1" rect="0, 72, 96, 96"/>
    <Meta desc="按钮1悬浮图元" bitmap="1" rect="0, 24, 96, 48"/>
    <Meta desc="按钮1按下图元" bitmap="1" rect="0, 48, 96, 72"/>
</Resource>
)xml";

DWORD color_a; 


// Test UIControl
class TestControl : public LongUI::UIControl {
    // super class define
    typedef LongUI::UIControl Super;
public:
    // create 创建
    static UIControl* WINAPI CreateControl(pugi::xml_node node) noexcept {
        if (!node) {
            UIManager << DL_Warning << L"node null" << LongUI::endl;
        }
        // 申请空间
        auto pControl = LongUI::UIControl::AllocRealControl<TestControl>(
            node,
            [=](void* p) noexcept { new(p) TestControl(node);}
        );
        if (!pControl) {
            UIManager << DL_Error << L"alloc null" << LongUI::endl;
        }
        return pControl;
    }
public:
    // Render This Control
    virtual HRESULT Render(LongUI::RenderType type) noexcept override {
        D2D1_RECT_F draw_rect;
        switch (type)
        {
        case LongUI::RenderType::Type_RenderBackground:
            __fallthrough;
        case LongUI::RenderType::Type_Render:
            // 父类背景
            Super::Render(LongUI::RenderType::Type_RenderBackground);
            // 背景中断
            if (type == LongUI::RenderType::Type_RenderBackground) {
                break;
            }
            __fallthrough;
        case LongUI::RenderType::Type_RenderForeground:
            draw_rect = GetDrawRect(this);
            D2D1_COLOR_F color = D2D1::ColorF(0xfcf7f4);
            m_pBrush_SetBeforeUse->SetColor(&color);
            m_pRenderTarget->FillRectangle(&draw_rect, m_pBrush_SetBeforeUse);
            color.a = float(reinterpret_cast<uint8_t*>(&color_a)[3]) / 255.f;
            color.r = float(reinterpret_cast<uint8_t*>(&color_a)[2]) / 255.f;
            color.g = float(reinterpret_cast<uint8_t*>(&color_a)[1]) / 255.f;
            color.b = float(reinterpret_cast<uint8_t*>(&color_a)[0]) / 255.f;
            m_pBrush_SetBeforeUse->SetColor(&color);
            m_pRenderTarget->FillRectangle(&draw_rect, m_pBrush_SetBeforeUse);
            m_pRenderTarget->DrawImage(m_pEffectOut);
            // 父类前景
            Super::Render(LongUI::RenderType::Type_RenderForeground);
            break;
        case LongUI::RenderType::Type_RenderOffScreen:
            if (m_bDrawSizeChanged) {
                this->draw_zone = this->show_zone;
            }
            draw_rect = GetDrawRect(this);
            // 检查布局
            if (m_bDrawSizeChanged) {
                ::SafeRelease(m_pCmdList);
                m_pRenderTarget->CreateCommandList(&m_pCmdList);
                // 设置大小
                m_text.SetNewSize(this->draw_zone.width, this->draw_zone.height);
                // 渲染文字
                m_pRenderTarget->SetTarget(m_pCmdList);
                m_pRenderTarget->BeginDraw();
                m_text.Render(draw_rect.left, draw_rect.top);
                m_pRenderTarget->EndDraw();
                m_pCmdList->Close();
                // 设置为输入
                m_pEffect->SetInput(0, m_pCmdList);
            }
        }
        return S_OK;
    }
    //do the event
    virtual bool DoEvent(LongUI::EventArgument& arg) noexcept  override {
        if (arg.sender) {
            if (arg.event == LongUI::Event::Event_FindControl &&
                LongUI::IsPointInRect(this->show_zone, arg.pt)) {
                arg.ctrl = this;
            }
            else if (arg.event == LongUI::Event::Event_FinishedTreeBuliding) {
                // 注册事件
                this->SetEventCallBackT(L"6", LongUI::Event::Event_SliderValueChanged, &TestControl::OnValueChanged);
            }
        }
        return false;
    }
    // recreate resource
    virtual HRESULT Recreate(LongUIRenderTarget* target) noexcept override {
        ::SafeRelease(m_pEffectOut);
        ::SafeRelease(m_pEffect);
        // 创建特效
        target->CreateEffect(CLSID_D2D1GaussianBlur, &m_pEffect);
        assert(m_pEffect);
        // 设置模糊属性
        m_pEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_OPTIMIZATION, D2D1_GAUSSIANBLUR_OPTIMIZATION_QUALITY );
        m_pEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.f  );
        // 获取输出
        m_pEffect->GetOutput(&m_pEffectOut);
        // 首次就注册
        if (m_FirstRecreate) {
            m_pWindow->RegisterOffScreenRender2D(this);
            m_FirstRecreate = false;
        }
        return Super::Recreate(target);
    }
    // On Value Changed
    bool OnValueChanged(UIControl* control) {
        register auto value = static_cast<LongUI::UISlider*>(control)->GetValue();
        m_pEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, value * 10.f);
        return true;
    }
    // close this control
    virtual void Close() noexcept { delete this; };
protected:
    // constructor
    TestControl(pugi::xml_node node) noexcept : Super(node), m_text(node){
    }
    // destructor
    ~TestControl() {
        ::SafeRelease(m_pCmdList);
        ::SafeRelease(m_pEffectOut);
        ::SafeRelease(m_pEffect);
    }
protected:
    // text
    LongUI::UIText              m_text;
    // bool
    bool                        m_FirstRecreate = true;
    //
    bool                test_unused[sizeof(void*) / sizeof(bool) - 2];
    // command list
    ID2D1CommandList*           m_pCmdList = nullptr;
    // effect
    ID2D1Effect*                m_pEffect = nullptr;
    // effect output
    ID2D1Image*                 m_pEffectOut = nullptr;
};



// Test Video Control
class UIVideoAlpha : public LongUI::UIControl {
    // super class define
    typedef LongUI::UIControl Super;
public:
    // create 创建
    static UIControl* WINAPI CreateControl(pugi::xml_node node) noexcept {
        if (!node) {
            UIManager << DL_Warning << L"node null" << LongUI::endl;
        }
        // 申请空间
        auto pControl = LongUI::UIControl::AllocRealControl<UIVideoAlpha>(
            node,
            [=](void* p) noexcept { new(p) UIVideoAlpha(node); }
            );
        if (!pControl) {
            UIManager << DL_Error << L"alloc null" << LongUI::endl;
        }
        return pControl;
    }
public:
    // Render This Control
    virtual HRESULT Render(LongUI::RenderType type) noexcept override {
        switch (type)
        {
        case LongUI::RenderType::Type_RenderBackground:
            D2D1_RECT_F draw_rect;
            __fallthrough;
        case LongUI::RenderType::Type_Render:
            // 父类背景
            Super::Render(LongUI::RenderType::Type_RenderBackground);
            // 背景中断
            if (type == LongUI::RenderType::Type_RenderBackground) {
                break;
            }
            __fallthrough;
        case LongUI::RenderType::Type_RenderForeground:
            // 更新刻画地区
            if (m_bDrawSizeChanged) {
                this->draw_zone = this->show_zone;
            }
            if (m_bDrawSizeChanged) {
                this->draw_zone = this->show_zone;
            }
            draw_rect = GetDrawRect(this);
            m_video.Render(&draw_rect);
            m_pWindow->StartRender(1.f, this);
            // 父类前景
            Super::Render(LongUI::RenderType::Type_RenderForeground);
            break;
        case LongUI::RenderType::Type_RenderOffScreen:
            break;
        }
        return S_OK;





    }
    //do the event
    virtual bool DoEvent(LongUI::EventArgument& arg) noexcept override {
        if (arg.sender) {
            if (arg.event == LongUI::Event::Event_FindControl &&
                LongUI::IsPointInRect(this->show_zone, arg.pt)) {
                arg.ctrl = this;
            }
            else if (arg.event == LongUI::Event::Event_FinishedTreeBuliding) {
            }
        }
        return false;
    }
    // recreate resource
    virtual HRESULT Recreate(LongUIRenderTarget* target) noexcept override {
        // 重建视频
        register auto hr = m_video.Recreate(target);
        // 重建父类
        if (SUCCEEDED(hr)) {
            hr = Super::Recreate(target);
        }
        return hr;
    }
    // close this control
    virtual void Close() noexcept override { delete this; };
protected:
    // constructor
    UIVideoAlpha(pugi::xml_node node) noexcept : Super(node) {
        m_video.Init();
        auto re = m_video.HasVideo();
        auto hr = m_video.SetSource(L"arcv45.mp4");
    }
    // destructor
    ~UIVideoAlpha() {
    }
protected:
    // video
    LongUI::CUIVideoComponent       m_video;
};


// 应用程序入口
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char* lpCmdLine, int nCmdShow) {
    DWORD colors[32];
    const DWORD ycolor = (237 << 16) | (222 << 8) | (105);
    for (int i = 0; i < lengthof(colors); ++i) {
        colors[i] = ::GetSysColor(i);
    }
    DWORD buffer_size = sizeof DWORD;
    auto error_code = ::RegGetValueA(
        HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\DWM",
        "ColorizationColor",
        RRF_RT_DWORD,
        nullptr,
        &color_a,
        &buffer_size
        );
    ::HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);
    // configure for this demo
    class DemoConfigure final : public LongUI::CUIDefaultConfigure {
        typedef LongUI::CUIDefaultConfigure Super;
    public:
        // 构造函数
        DemoConfigure() : Super() { this->script = &mruby; this->resource = res_xml; }
        // 获取地区名称
        auto GetLocaleName(wchar_t name[/*LOCALE_NAME_MAX_LENGTH*/]) noexcept->void override {
            ::wcscpy(name, L"en-us");
        };
        // 添加自定义控件
        auto AddCustomControl(LongUI::CUIManager& manager) noexcept->void override{
            manager.AddS2CPair(L"Test", TestControl::CreateControl);
            manager.AddS2CPair(L"Video", UIVideoAlpha::CreateControl);
        };
        // 使用CPU渲染
        auto IsRenderByCPU() noexcept ->bool override { return true; }
    private:
        // mruby script
        MRubyScript     mruby;
    } config;
    //
    // Buffer of MainWindow, align for 4(x86)
    alignas(sizeof(void*)) size_t buffer[sizeof(MainWindow) / sizeof(size_t) + 1];
    // 初始化 OLE (OLE会调用CoInitializeEx初始化COM)
    if (SUCCEEDED(::OleInitialize(nullptr))) {
        // 初始化 窗口管理器 
        UIManager.Initialize(&config);
        // 作战控制连线!
        UIManager << DL_Hint << L"Battle Control Online!" << LongUI::endl;
        // 创建主窗口
        UIManager.CreateUIWindow<MainWindow>(test_xml, buffer);
        // 运行本程序
        UIManager.Run();
        // 作战控制终止!
        UIManager << DL_Hint << L"Battle Control Terminated!" << LongUI::endl;
        // 反初始化 窗口管理器
        UIManager.UnInitialize();
        // 反初始化 COM 与 OLE
        ::OleUninitialize(); 
    }
    return EXIT_SUCCESS;
}


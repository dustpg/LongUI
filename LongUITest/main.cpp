#include "stdafx.h"
#include "included.h"


//  animationduration="2"
// 测试XML &#xD; --> \r &#xA; --> \n
#if 0
const char* test_xml = u8R"xml(<?xml version="1.0" encoding="utf-8"?>
<Window size="1024, 768" name="MainWindow" vscrollbar="ScrollBarA" hscrollbar="ScrollBarA">
    <VerticalLayout name="VLayout1" pos="0, 0, 1100, 128">
        <!--Video name="asd" /-->
        <Test name="test" texttype="core" text="%cHello%], %cworld!%]%c泥壕!%]世界!%p#0F0, #F00, #00F"/>
        <Slider name="6" renderparent="true"/>
    </VerticalLayout>
    <HorizontalLayout name="HLayout" pos="0, 0, 0, 256">
        <EditBasic name="edit01" textmultiline="true" text="Hello, world!&#xD;&#xA;泥壕, 世界!"/>
        <VerticalLayout name="VLayout2">
            <Label name="label_test" texttype="core" text="%cHello%], world!泥壕!世界!%p#F00"/>
            <Button name="btn_systemlook" margin="4,4,4,4" borderwidth="1" text="Hello, world!"/>
            <Button name="4" disabledmeta="1" normalmeta="2" script="App.click_button1($apparg)"
                margin="4,4,4,4" hovermeta="3" pushedmeta="4" borderwidth="1" text="Hello, world!"/>
            <CheckBox name="btn_skinlook" text="Hello, world!"/>
            <!--Button name="uac" disabledmeta="1" normalmeta="2" 
                hovermeta="3" pushedmeta="4" text="Try  Elevate UAC Now "/-->
        </VerticalLayout>
    </HorizontalLayout>
</Window>
)xml";
#else
const char* test_xml = u8R"xml(<?xml version="1.0" encoding="utf-8"?>
<Window size="1024, 768" name="MainWindow" margin="32,32,32,32"
    rightcontrol="ScrollBarA" bottomcontrol="ScrollBarA" clearcolor="1,1,1,0.85">
    <VerticalLayout name="V" csize="1366, 512">
        <Button name="1" margin="4,4,4,4" disabledmeta="1" csize = "1500,0"
            normalmeta="2" hovermeta="3" pushedmeta="4" text="Hello, world!"/>
        <Button name="2" margin="4,4,4,4" borderwidth="1" text="Hello, world!"/>
    </VerticalLayout>
    <HorizontalLayout name="H" csize="0, 512">
        <Button name="3" margin="4,4,4,4" disabledmeta="1"
            normalmeta="2" hovermeta="3" pushedmeta="4" text="Hello, world!"/>
        <Button name="4" margin="4,4,4,4" borderwidth="1" text="Hello, world!"/>
    </HorizontalLayout>
</Window>
)xml";
#endif

constexpr char* res_xml = u8R"xml(<?xml version="1.0" encoding="utf-8"?>
<Resource>
    <!-- Bitmap区域Zone -->
    <Bitmap>
        <Item desc="按钮1" res="btn.png"/>
    </Bitmap>
    <!-- Meta区域Zone -->
    <Meta>
        <Item desc="按钮1无效图元" bitmap="1" rect="0,  0, 96, 24" rule="1"/>
        <Item desc="按钮1通常图元" bitmap="1" rect="0, 72, 96, 96" rule="1"/>
        <Item desc="按钮1悬浮图元" bitmap="1" rect="0, 24, 96, 48" rule="1"/>
        <Item desc="按钮1按下图元" bitmap="1" rect="0, 48, 96, 72" rule="1"/>
    </Meta>
</Resource>
)xml";

// Test UIControl
class TestControl : public LongUI::UIControl {
    // super class define
    typedef LongUI::UIControl Super;
public:
    // create 创建
    static UIControl* WINAPI CreateControl(LongUI::CreateEventType type, pugi::xml_node node) noexcept {
        // 分类判断
        UIControl* pControl = nullptr;
        switch (type)
        {
        case LongUI::Type_CreateControl:
            if (!node) {
                UIManager << DL_Warning << L"node null" << LongUI::endl;
            }
            // 申请空间
            pControl = LongUI::UIControl::AllocRealControl<TestControl>(
                node,
                [=](void* p) noexcept { new(p) TestControl(node); }
            );
            if (!pControl) {
                UIManager << DL_Error << L"alloc null" << LongUI::endl;
            }
            break;
        case LongUI::Type_Initialize:
            break;
        case LongUI::Type_Recreate:
            break;
        case LongUI::Type_Uninitialize:
            break;
        }
        return pControl;
    }
public:
    // Render This Control
    virtual void Render(LongUI::RenderType type) const noexcept override {
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
            this->GetContentRect(draw_rect);
            D2D1_COLOR_F color = D2D1::ColorF(0xfcf7f4);
            m_pBrush_SetBeforeUse->SetColor(&color);
            m_pRenderTarget->FillRectangle(&draw_rect, m_pBrush_SetBeforeUse);
            UIManager.GetThemeColor(color);
            m_pBrush_SetBeforeUse->SetColor(&color);
            m_pRenderTarget->FillRectangle(&draw_rect, m_pBrush_SetBeforeUse);
            m_pRenderTarget->DrawImage(m_pEffectOut);
            // 父类前景
            Super::Render(LongUI::RenderType::Type_RenderForeground);
            break;
        case LongUI::RenderType::Type_RenderOffScreen:
            this->GetContentRect(draw_rect);
            // 渲染文字
            if (false) {
                m_pRenderTarget->SetTarget(m_pCmdList);
                m_pRenderTarget->BeginDraw();
                m_text.Render(draw_rect.left, draw_rect.top);
                m_pRenderTarget->EndDraw();
                m_pCmdList->Close();
                // 设置为输入
                m_pEffect->SetInput(0, m_pCmdList);
            }
        }
    }
    // update
    void Update() noexcept override {
        // 检查布局
        if (this->IsControlSizeChanged()) {
            ::SafeRelease(m_pCmdList);
            m_pRenderTarget->CreateCommandList(&m_pCmdList);
            // 设置大小
            m_text.SetNewSize(this->cwidth, this->cheight);

            // 已经处理
            this->ControlSizeChangeHandled();
        }
        Super::Update();
    }
    //do the event
    virtual bool DoEvent(const LongUI::EventArgument& arg) noexcept  override {
        D2D1_MATRIX_3X2_F world; this->GetWorldTransform(world);
        D2D1_POINT_2F pt4self = LongUI::TransformPointInverse(world, arg.pt);
        if (arg.sender) {
            /*if (arg.event == LongUI::Event::Event_FindControl) {
                // 检查鼠标范围
                if (FindControlHelper(pt4self, this)) {
                    arg.ctrl = this;
                }
                return true;
            }
            else*/ if (arg.event == LongUI::Event::Event_TreeBulidingFinished) {
                // 注册事件
                this->SetEventCallBack(
                    L"6", 
                    LongUI::Event::Event_SliderValueChanged, 
                    [](UIControl* t, UIControl* s) noexcept { return static_cast<TestControl*>(t)->OnValueChangedConst(s); }
                    );
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
    bool OnValueChangedConst(UIControl* control) const {
        register auto value = static_cast<LongUI::UISlider*>(control)->GetValue();
        m_pEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, value * 10.f);
        return true;
    }
    // close this control
    virtual void Cleanup() noexcept { delete this; };
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
    static UIControl* WINAPI CreateControl(LongUI::CreateEventType type, pugi::xml_node node) noexcept {
        // 分类判断
        UIControl* pControl = nullptr;
        switch (type)
        {
        case LongUI::Type_CreateControl:
            if (!node) {
                UIManager << DL_Warning << L"node null" << LongUI::endl;
            }
            // 申请空间
            pControl = LongUI::UIControl::AllocRealControl<UIVideoAlpha>(
                node,
                [=](void* p) noexcept { new(p) UIVideoAlpha(node); }
            );
            if (!pControl) {
                UIManager << DL_Error << L"alloc null" << LongUI::endl;
            }
            break;
        case LongUI::Type_Initialize:
            break;
        case LongUI::Type_Recreate:
            break;
        case LongUI::Type_Uninitialize:
            break;
        }
        return pControl;
    }
public:
    // Render This Control
    virtual void Render(LongUI::RenderType type) const noexcept override {
        /*switch (type)
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
            draw_rect = this->GetDrawRect();
            m_video.Render(&draw_rect);
            m_pWindow->StartRender(1.f, this);
            // 父类前景
            Super::Render(LongUI::RenderType::Type_RenderForeground);
            break;
        case LongUI::RenderType::Type_RenderOffScreen:
            break;
        }
        return S_OK;*/
    }
    //do the event
    virtual bool DoEvent(const LongUI::EventArgument& arg) noexcept override {
        if (arg.sender) {
            /*if (arg.event == LongUI::Event::Event_FindControl) {
                // 检查鼠标范围
                assert(arg.pt.x < this->width && arg.pt.y < this->width && "check it");
                arg.ctrl = this;
            }
            else*/ if (arg.event == LongUI::Event::Event_TreeBulidingFinished) {
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
    virtual void Cleanup() noexcept override { delete this; };
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
    // set info
    ::HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);
    // configure for this demo
    class DemoConfigure final : public LongUI::CUIDefaultConfigure {
        typedef LongUI::CUIDefaultConfigure Super;
    public:
        // 构造函数
        DemoConfigure() : Super(UIManager) { this->script = &mruby; this->resource = res_xml; }
        // 获取地区名称
        auto GetLocaleName(wchar_t name[/*LOCALE_NAME_MAX_LENGTH*/]) noexcept->void override {
            ::wcscpy(name, L"en-us");
        };
        // 添加自定义控件
        auto AddCustomControl() noexcept->void override {
            m_manager.RegisterControl(TestControl::CreateControl, L"Test");
            m_manager.RegisterControl(UIVideoAlpha::CreateControl, L"Video");
        };
        // 使用CPU渲染
        auto IsRenderByCPU() noexcept ->bool override { return true; }
    private:
        // mruby script
        MRubyScript     mruby = MRubyScript(UIManager);
    } config;
    // 创建
    auto create_main_window = [](pugi::xml_node node, LongUI::UIWindow* parent, void* buffer) {
        reinterpret_cast<MainWindow*>(buffer)->MainWindow::MainWindow(node, parent);
        return static_cast<LongUI::UIWindow*>(reinterpret_cast<MainWindow*>(buffer));
    };
    // Buffer of MainWindow, align for 4(x86)
    alignas(sizeof(void*)) size_t buffer[sizeof(MainWindow) / sizeof(size_t) + 1];
    // 初始化 OLE (OLE会调用CoInitializeEx初始化COM)
    if (SUCCEEDED(::OleInitialize(nullptr))) {
        // 初始化 窗口管理器 
        UIManager.Initialize(&config);
        // 作战控制连线!
        UIManager << DL_Hint << L"Battle Control Online!" << LongUI::endl;
        // 创建主窗口
        UIManager.CreateUIWindow(test_xml, nullptr, create_main_window, buffer);
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


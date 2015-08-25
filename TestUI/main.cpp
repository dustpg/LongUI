#if 1
#define LONGUI_WITH_DEFAULT_HEADER
#define _CRT_SECURE_NO_WARNINGS
#include "../LongUI/LongUI.h"

//  animationduration="2"
// 测试XML &#xD; --> \r &#xA; --> \n
#if 0
const char* test_xml = u8R"xml(<?xml version="1.0" encoding="utf-8"?>
<Window size="1024, 768" name="MainWindow" bottomcontrol="ScrollBarA" rightcontrol="ScrollBarA">
    <VerticalLayout name="VLayout1" size="1100, 128">
        <!--Video name="asd" /-->
        <Test name="test" textrichtype="core" text="%cHello%], %cworld!%]%c泥壕!%]世界!%p#0F0, #F00, #00F"/>
        <Slider name="6" renderparent="true"/>
    </VerticalLayout>
    <HorizontalLayout name="HLayout" size="0, 256">
        <Edit name="edit01" textmultiline="true" text="Hello, world!&#xD;&#xA;泥壕, 世界!"/>
        <VerticalLayout name="VLayout2">
            <Text name="label_test" textrichtype="core" text="%cHello%], world!泥壕!世界!%p#F00"/>
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
// bottomcontrol="ScrollBarA" rightcontrol="ScrollBarA" margin="16,16,16,16"
const char* test_xml = u8R"xml(<?xml version="1.0" encoding="utf-8"?>
<Window size="800, 600" name="MainWindow" debug="true"
    clearcolor="1,1,1,0.95" >
    <VerticalLayout name="V" bottomcontrol="ScrollBarA" rightcontrol="ScrollBarA">
        <Button name="1" templateid="1" text="Hello, world!"/>
        <Slider name="sb" thumbsize="32,32" margin="4,4,4,4"/>
        <Button name="2" size="0, 0" templateid="2" text="Hello, world!"/>
    </VerticalLayout>
    <Slider name="6" thumbsize="32,32" margin="4,4,4,4" size="0,64"/>
    <HorizontalLayout name="H" size="0, 128">
        <Button name="3" margin="4,4,4,4" disabledmeta="1"
            normalmeta="2" hovermeta="3" pushedmeta="4" text="Hello, world!"/>
        <Button name="4" margin="4,4,4,4" borderwidth="1" text="Hello, world!"/>
    </HorizontalLayout>
    <Button name="btn_x" size="0, 32" borderwidth="1" textrichtype="core"
        text="%cHello%], %cworld!%]%c泥壕!%]世界!%p#0F0, #F00, #00F"/>
</Window>
)xml";
#endif
constexpr char* res_xml = u8R"xml(<?xml version="1.0" encoding="utf-8"?>
<Resource>
    <!-- Bitmap区域Zone -->
    <Bitmap>
        <!-- You can use other name not limited in 'Item' -->
        <Item desc="按钮1" res="btn.png"/>
    </Bitmap>
    <!-- Meta区域Zone -->
    <Meta>
        <Item desc="按钮1无效图元" bitmap="1" rect="0,  0, 96, 24" rule="button"/>
        <Item desc="按钮1通常图元" bitmap="1" rect="0, 72, 96, 96" rule="button"/>
        <Item desc="按钮1悬浮图元" bitmap="1" rect="0, 24, 96, 48" rule="button"/>
        <Item desc="按钮1按下图元" bitmap="1" rect="0, 48, 96, 72" rule="button"/>
    </Meta>
</Resource>
)xml";

// MainWindow class
class MainWindow final : public LongUI::UIWindow {
    // super class
    using Super = LongUI::UIWindow;
private:
    // dtor
    ~MainWindow() = default;
public:
    // ctor
    MainWindow(pugi::xml_node node, LongUI::UIWindow* parent) : Super(node, parent) {}
    // do some event
    virtual bool DoEvent(const LongUI::EventArgument&) noexcept override;
    // clean up
    virtual void Cleanup() noexcept override { this->~MainWindow(); }
private:
    // init
    void init();
    // on number button clicked
    void number_button_clicked(LongUI::UIControl* btn);
    // on plus
    bool on_plus();
    // on minus
    bool on_minus();
    // on equal
    bool on_equal();
private:
};

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
            this->GetViewRect(draw_rect);
            D2D1_COLOR_F color = D2D1::ColorF(0xfcf7f4);
            m_pBrush_SetBeforeUse->SetColor(&color);
            UIManager_RenderTarget->FillRectangle(&draw_rect, m_pBrush_SetBeforeUse);
            UIManager.GetThemeColor(color);
            m_pBrush_SetBeforeUse->SetColor(&color);
            UIManager_RenderTarget->FillRectangle(&draw_rect, m_pBrush_SetBeforeUse);
            UIManager_RenderTarget->DrawImage(m_pEffectOut);
            // 父类前景
            Super::Render(LongUI::RenderType::Type_RenderForeground);
            break;
        case LongUI::RenderType::Type_RenderOffScreen:
            this->GetViewRect(draw_rect);
            // 渲染文字
            if (false) {
                UIManager_RenderTarget->SetTarget(m_pCmdList);
                UIManager_RenderTarget->BeginDraw();
                m_text.Render(draw_rect.left, draw_rect.top);
                UIManager_RenderTarget->EndDraw();
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
            UIManager_RenderTarget->CreateCommandList(&m_pCmdList);
            // 设置大小
            m_text.Resize(this->view_size.width, this->view_size.height);
            // 已经处理
            this->ControlSizeChangeHandled();
        }
        Super::Update();
    }
    //do the event
    virtual bool DoEvent(const LongUI::EventArgument& arg) noexcept  override {
        D2D1_POINT_2F pt4self = LongUI::TransformPointInverse(this->world, arg.pt);
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
                this->SetSubEventCallBack(
                    L"6",
                    LongUI::SubEvent::Event_SliderValueChanged,
                    [](UIControl* t, UIControl* s) noexcept { return static_cast<TestControl*>(t)->OnValueChangedConst(s); }
                );
            }
        }

        return false;
    }
    // recreate resource
    virtual HRESULT Recreate() noexcept override {
        ::SafeRelease(m_pEffectOut);
        ::SafeRelease(m_pEffect);
        // 创建特效
        UIManager_RenderTarget->CreateEffect(CLSID_D2D1GaussianBlur, &m_pEffect);
        assert(m_pEffect);
        // 设置模糊属性
        m_pEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_OPTIMIZATION, D2D1_GAUSSIANBLUR_OPTIMIZATION_QUALITY);
        m_pEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 0.f);
        // 获取输出
        m_pEffect->GetOutput(&m_pEffectOut);
        // 首次就注册
        if (m_FirstRecreate) {
            m_pWindow->RegisterOffScreenRender2D(this);
            m_FirstRecreate = false;
        }
        return Super::Recreate();
    }
    // On Value Changed
    bool OnValueChangedConst(UIControl* control) const {
        register auto value = static_cast<LongUI::UISlider*>(control)->GetValueSE();
        m_pEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, value );
        return true;
    }
    // close this control
    virtual void Cleanup() noexcept { delete this; };
protected:
    // constructor
    TestControl(pugi::xml_node node) noexcept : Super(node), m_text(node) {
    }
    // destructor
    ~TestControl() {
        ::SafeRelease(m_pCmdList);
        ::SafeRelease(m_pEffectOut);
        ::SafeRelease(m_pEffect);
    }
protected:
    // text
    LongUI::Component::ShortText     m_text;
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
    virtual HRESULT Recreate() noexcept override {
        // 重建视频
        register auto hr = m_video.Recreate();
        // 重建父类
        if (SUCCEEDED(hr)) {
            hr = Super::Recreate();
        }
        return hr;
    }
    // close this control
    virtual void Cleanup() noexcept override { delete this; };
protected:
    // constructor
    UIVideoAlpha(pugi::xml_node node) noexcept : Super(node) {
        auto hr = m_video.Initialize();
        assert(SUCCEEDED(hr));
        auto re = m_video.HasVideo();
        hr = m_video.SetSource(L"arcv45.mp4");
        assert(SUCCEEDED(hr));
        hr = S_OK;
    }
    // destructor
    ~UIVideoAlpha() {
    }
protected:
    // video
    LongUI::Component::Video    m_video;
};


// 应用程序入口
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char* lpCmdLine, int nCmdShow) {
    // 设置堆信息
    ::HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);
    // 本Demo的配置信息
#pragma region Configure for this demo
    class DemoConfigure final : public LongUI::CUIDefaultConfigure {
        typedef LongUI::CUIDefaultConfigure Super;
    public:
        // 构造函数
        DemoConfigure() : Super(UIManager) { /*this->script = &mruby;*/ this->resource = res_xml; }
        // 析构函数
        ~DemoConfigure() { if (m_hDll) { ::FreeLibrary(m_hDll); m_hDll = nullptr; } }
        // 获取地区名称
        auto GetLocaleName(wchar_t name[/*LOCALE_NAME_MAX_LENGTH*/]) noexcept->void override {
            ::wcscpy(name, L"en-us");
        };
        // 获取控件模板
        auto GetTemplateString() noexcept->const char* override {
            return u8R"xml(<?xml version="1.0" encoding="utf-8"?>
<!-- You can use other name not limited in 'Template' -->
<Template>
    <!-- You can use other name not limited in 'Control' -->
    <!-- Index 1 -->
    <Control desc="System look like button" margin="4,4,4,4" borderwidth="1"/>
    <!-- Index 2 -->
    <Control desc="btn.png look like button" margin="4,4,4,4" disabledmeta="1"
            normalmeta="2" hovermeta="3" pushedmeta="4"/>
</Template>
)xml";
        }
        // 添加自定义控件
        auto AddCustomControl() noexcept->void override {
            m_manager.RegisterControl(TestControl::CreateControl, L"Test");
            m_manager.RegisterControl(UIVideoAlpha::CreateControl, L"Video");
            /*if (m_hDll) {
            auto func = reinterpret_cast<LongUI::CreateControlFunction>(
            ::GetProcAddress(m_hDll, "LongUICreateControl")
            );
            m_manager.RegisterControl(func, L"DllTest");
            }*/
        };
        // 使用CPU渲染
        auto IsRenderByCPU() noexcept ->bool override { return true; }
    private:
        // mruby script
        //MRubyScript     mruby = MRubyScript(UIManager);
        // dll
        HMODULE         m_hDll = ::LoadLibraryW(L"test.dll");
    } config;
#pragma endregion
    // MainWindow 的缓存/栈空间地址, 在x86上4字节对齐
    alignas(sizeof(void*)) size_t buffer[sizeof(MainWindow) / sizeof(size_t) + 1];
    // 初始化 OLE (OLE会调用CoInitializeEx初始化COM)
    if (SUCCEEDED(::OleInitialize(nullptr))) {
        // 初始化 UI管理器 
        if (SUCCEEDED(UIManager.Initialize(&config))) {
            // 作战控制连线!
            UIManager << DL_Hint << L"Battle Control Online!" << LongUI::endl;
            // 创建主窗口
            UIManager.CreateUIWindow<MainWindow>(test_xml, nullptr, buffer);
            // 运行本程序
            UIManager.Run();
            // 作战控制终止!
            UIManager << DL_Hint << L"Battle Control Terminated!" << LongUI::endl;
            // 反初始化 UI管理器
            UIManager.UnInitialize();
        }
        // 反初始化 COM 与 OLE
        ::OleUninitialize();
    }
    // 成功退出
    return EXIT_SUCCESS;
}

// ------------------------------- IMPL -----------------------------------
// do event for ui
bool MainWindow::DoEvent(const LongUI::EventArgument& arg) noexcept {
    // longui event
    if (arg.sender) {
        switch (arg.event)
        {
        /*case LongUI::Event::Event_ButtonClicked:
            // number button clicked event
            //this->number_button_clicked(arg.sender);
            return true;*/
        case LongUI::Event::Event_TreeBulidingFinished:
            // Event_TreeBulidingFinished could as "init" event
            //this->init();
            // super will send this event to children
            __fallthrough;
        default:
            return Super::DoEvent(arg);
        }
    }
    // system event
    else {
        return Super::DoEvent(arg);
    }
}
#else
#define LONGUI_WITH_DEFAULT_HEADER
#define _CRT_SECURE_NO_WARNINGS
#include "../LongUI/LongUI.h"
#include "../Demos/Step3_handleeventex/demo.h"

// longui::demo namespace
namespace LongUI { namespace Demo {
    // config
    class MyConfig final : public CUIDefaultConfigure {
        // super class
        using Super = CUIDefaultConfigure;
    public:
        // ctor
        MyConfig() : Super(UIManager) { }
        // return true, if use cpu rendering
        virtual auto IsRenderByCPU() noexcept->bool override {
            return cpu_rendering;
        }
        bool    cpu_rendering = true;
    };
}}

// Entry for App
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char* lpCmdLine, int nCmdShow) {
    // every windows desktop app should do this
    ::HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);
    // use OleInitialize to init ole and com
    if (SUCCEEDED(::OleInitialize(nullptr))) {
        // my config
        LongUI::Demo::MyConfig config;
        // -GPU
        if (!std::strcmp("-GPU", lpCmdLine)) config.cpu_rendering = false;
        // init longui manager
        if (SUCCEEDED(UIManager.Initialize(&config))) {
            // my style
            UIManager << DL_Hint << L"Battle Control Online!" << LongUI::endl;
            // create main window, return nullptr for some error
            UIManager.CreateUIWindow<LongUI::Demo::MainWindow>(DEMO_XML);
            // run this app
            UIManager.Run();
            // my style
            UIManager << DL_Hint << L"Battle Control Terminated!" << LongUI::endl;
            // cleanup longui
            UIManager.UnInitialize();
        }
    }
    // cleanup ole and com
    ::OleUninitialize();
    // exit
    return EXIT_SUCCESS;
}

#endif

// Common Control for MS
#ifdef _MSC_VER
#ifdef _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif
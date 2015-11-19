#if 1
#define LONGUI_WITH_DEFAULT_HEADER
#define _CRT_SECURE_NO_WARNINGS
#include "LongUI.h"

//  animationduration="2"
// 测试XML &#xD; --> \r &#xA; --> \n
const char* test_xml_01 = u8R"xml(<?xml version="1.0" encoding="utf-8"?>
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

// bottomcontrol="ScrollBarA" rightcontrol="ScrollBarA" margin="16,16,16,16"
const char* test_xml_02 = u8R"xml(<?xml version="1.0" encoding="utf-8"?>
<Window size="800, 600" name="MainWindow" debugshow="true" clearcolor="1,1,1,0.95" >
    <VerticalLayout name="V" topcontrol="ScrollBarA" >
        <Button templateid="1" text="Hello, world!"/>
        <Slider name="sb" thumbsize="32,32" margin="4,4,4,4"/>
        <Button name="2" size="1024, 0" templateid="2" text="Hello, world!"/>
    </VerticalLayout>
    <Slider name="6" thumbsize="32,32" margin="4,4,4,4" size="0,64"/>
    <Edit debug="true" name="edit_demo" size="0,64" text="ABC甲乙丙123"/>
    <Button name="btn_x" size="0, 32" borderwidth="1" textrichtype="core"
        text="%cHello%], %cworld!%]%c泥壕!%]世界!%p#0F0, #F00, #00F"/>
    <VerticalLayout name="V2" topcontrol="ScrollBarA" >
        <Button templateid="1" text="Hello, world!"/>
        <Slider thumbsize="32,32" margin="4,4,4,4"/>
        <Button size="2048, 0" templateid="2" text="Hello, world!"/>
    </VerticalLayout>
</Window>
)xml";

/*

        <VerticalLayout name="V2" topcontrol="ScrollBarA">
            <Button size="2048, 0" templateid="1" text="Hello, world!"/>
        </VerticalLayout>

    <HorizontalLayout name="H" size="0, 128">
        <Button name="3" margin="4,4,4,4" disabledmeta="1"
            normalmeta="2" hovermeta="3" pushedmeta="4" text="Hello, world!"/>
        <Button name="4" margin="4,4,4,4" borderwidth="1" text="Hello, world!"/>
    </HorizontalLayout>*/

// <Slider name="sld_01" thumbsize="32,32" margin="4,4,4,4" size="0,64"/>
// 

const char* test_xml_03 = u8R"xml(<?xml version="1.0" encoding="utf-8"?>
<Window size="800, 600" name="MainWindow" debugshow="true"
    autoshow="false" clearcolor="1,1,1,0.95" >
    <Slider name="sld_01" thumbsize="32,32" margin="4,4,4,4" size="0,64"/>
    <List debug="ftrue" sort="true" name="lst_01" linetemplate="Text, Text">
        <ListHeader marginal="top" sepwidth="-8">
            <Button borderwidth="1" text="name" name="lst_header0"/>
            <Button borderwidth="1" text="desc" name="lst_header1"/>
        </ListHeader>
        <ScrollBarA marginal="bottom"/>
        <ScrollBarA marginal="right"/>
        <ListLine name="lin1">
            <Text text="1" templateid="4" name="listline1-1"/>
            <Text text="伍湖" templateid="4"/>
        </ListLine>
        <ListLine name="lin2">
            <Text text="2" templateid="4" name="listline2-1"/>
            <Text text="亖死" templateid="4"/>
        </ListLine>
        <ListLine name="lin3">
            <Text text="3" templateid="4" name="listline3-1"/>
            <Text text="三妻" templateid="4"/>
        </ListLine>
        <ListLine name="lin4">
            <Text text="5" templateid="4" name="listline4-1"/>
            <Text text="二连" templateid="4"/>
        </ListLine>
        <ListLine name="lin5">
            <Text text="4" templateid="4" name="listline5-1"/>
            <Text text="一瞬" templateid="4"/>
        </ListLine>
        <ListLine>
            <Text text="4" templateid="4"/>
            <Text text="一瞬" templateid="4"/>
        </ListLine>
        <ListLine>
            <Text text="4" templateid="4"/>
            <Text text="一瞬" templateid="4"/>
        </ListLine>
        <ListLine>
            <Text text="4" templateid="4"/>
            <Text text="一瞬" templateid="4"/>
        </ListLine>
        <ListLine>
            <Text text="4" templateid="4"/>
            <Text text="一瞬" templateid="4"/>
        </ListLine>
    </List>
    <Single><Button text="XYZ" borderwidth="1" margin="4,4,4,4"/></Single>
    <Edit debug="false" name="edit_demo" size="0,64" text="ABC甲乙丙123"/>
    <Button name="btn_x0" size="0, 48" borderwidth="1"
        margin="4,4,4,4" text="这是楷体字  這是楷體字" textfamily="KaiTi"/>
    <!--Button name="btn_x1" size="0, 64" borderwidth="1" textrichtype="core" margin="4,4,4,4" 
        text="%cHello%], %uworld!%]%f黑体字%]%f楷体字%]%p#0F0, SimHei, KaiTi"/-->
    <Button name="btn_x1" size="0, 64" borderwidth="1" textrichtype="core" margin="4,4,4,4" 
        text="%t水的化学式: H%t2%]O%t繁体%]%]%tdòng动%]%pss06,sups ,trad, ruby"/>
</Window>
)xml";

//text="%t这是繁体字哦%]!%t這是簡體字哦%]!%ptrad, smpl"/>



const char* test_xml_04 = u8R"xml(<?xml version="1.0" encoding="utf-8"?>
<Window textantimode="cleartype" size="800, 600" name="MainWindow" debugshow="true"
    autoshow="false" clearcolor="1,1,1,0.95" >
    <Slider name="sld_01" thumbsize="32,32" margin="4,4,4,4" size="0,64"/>
    <Page>
        <Button borderwidth="1" margin="4,4,4,4" text="如果"/>
        <Button borderwidth="1" margin="4,4,4,4" text="但是"/>
    </Page>
    <Page>
        <Button borderwidth="1" margin="4,4,4,4" text="虽然"/>
        <Button borderwidth="1" margin="4,4,4,4" text="不过"/>
    </Page>
    <Single templatesize="256, 0"><Edit text="这个"/></Single>
    <Single templatesize="256, 0">
        <Slider thumbsize="32,32" margin="4,4,4,4"/>
    </Single>
</Window>
)xml";

const char* test_xml = test_xml_03;


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
    // placement new
    auto operator new(size_t size, void* ptr) noexcept -> void* { return ::operator new(size, ptr); };
    // placement delete
    void operator delete(void* mem, void *ptr) noexcept { return ::operator delete(mem, ptr); };
    // ctor
    MainWindow(pugi::xml_node node, LongUI::UIWindow* parent) : Super(node, parent) {}
    // do some event
    virtual bool DoEvent(const LongUI::EventArgument& arg) noexcept override;
private:
    // clean up
    virtual void cleanup() noexcept override { this->~MainWindow(); }
    // init
    void init() {
        auto list = LongUI::longui_cast<LongUI::UIList*>(this->FindControl("lst_01"));
        if (list) {
            list->AddBeforSortCallBack([](LongUI::UIControl* list) {
                for (auto line : static_cast<LongUI::UIList*>(list)->GetContainer()) {
                    auto tobesorted =  line->GetToBeSorted();
                    auto ptr = const_cast<wchar_t*>(tobesorted->GetText());
                    tobesorted->user_ptr = ptr;
                }
                return true;
            });
        }
        auto btn = this->FindControl("btn_x1");
        if (btn) {
            auto ctrl1 = btn;
            auto ctrl2 = btn->prev->prev;
            btn->AddEventCall([ctrl1, ctrl2, this](UIControl*) noexcept {
                // 交换
                this->SwapChild(LongUI::MakeIteratorBI(ctrl1), LongUI::MakeIteratorBI(ctrl2));
                return true;
            }, LongUI::SubEvent::Event_ItemClicked);
        }
        btn = this->FindControl("btn_x0");
        if (btn) {
            auto ctrl1 = btn;
            auto ctrl2 = btn->prev;
            btn->AddEventCall([ctrl1, ctrl2, this](UIControl*) noexcept {
                // 交换
                this->SwapChild(LongUI::MakeIteratorBI(ctrl1), LongUI::MakeIteratorBI(ctrl2));
                return true;
            }, LongUI::SubEvent::Event_ItemClicked);
        }
    }
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
    // close this control
    virtual void cleanup() noexcept { delete this; };
public:
    // create 创建
    static UIControl* WINAPI CreateControl(LongUI::CreateEventType type, pugi::xml_node node) noexcept {
        // 分类判断
        UIControl* pControl = nullptr;
        switch (type)
        {
        case LongUI::Type_Initialize:
            break;
        case LongUI::Type_Recreate:
            break;
        case LongUI::Type_Uninitialize:
            break;
        case_LongUI__Type_CreateControl:
            // 警告
            if (!node) {
                UIManager << DL_Hint << L"node null" << LongUI::endl;
            }
            // 申请空间
            pControl = LongUI::CreateWidthCET<TestControl>(type, node);
            // OOM
            if (!pControl) {
                UIManager << DL_Error << L"alloc null" << LongUI::endl;
            }
        }
        return pControl;
    }
public:
    // Render This Control
    virtual void Render() const noexcept override {
        /*D2D1_RECT_F draw_rect;
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
        }*/
    }
    // update
    void Update() noexcept override {
        // 检查布局
        if (this->IsControlLayoutChanged()) {
            LongUI::SafeRelease(m_pCmdList);
            UIManager_RenderTarget->CreateCommandList(&m_pCmdList);
            // 设置大小
            m_text.Resize(this->view_size.width, this->view_size.height);
            // 已经处理
            this->ControlLayoutChangeHandled();
        }
        Super::Update();
    }
    //do the event
    virtual bool DoEvent(const LongUI::EventArgument& arg) noexcept  override {
        if (arg.sender) {
            // 注册事件
            if (arg.event == LongUI::Event::Event_TreeBulidingFinished) {
            }
        }

        return false;
    }
    // recreate resource
    virtual HRESULT Recreate() noexcept override {
        LongUI::SafeRelease(m_pEffectOut);
        LongUI::SafeRelease(m_pEffect);
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
        auto value = static_cast<LongUI::UISlider*>(control)->GetValueSE();
        m_pEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, value );
        return true;
    }
    // constructor
    TestControl(LongUI::UIContainer* cp, pugi::xml_node node) 
        noexcept : Super(cp, node), m_text(node) {
    }
protected:
    // destructor
    ~TestControl() {
        LongUI::SafeRelease(m_pCmdList);
        LongUI::SafeRelease(m_pEffectOut);
        LongUI::SafeRelease(m_pEffect);
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
    // close this control
    virtual void cleanup() noexcept override { delete this; };
public:
    // create 创建
    static UIControl* WINAPI CreateControl(LongUI::CreateEventType type, pugi::xml_node node) noexcept {
        // 分类判断
        UIControl* pControl = nullptr;
        switch (type)
        {
        case LongUI::Type_Initialize:
            break;
        case LongUI::Type_Recreate:
            break;
        case LongUI::Type_Uninitialize:
            break;
        case_LongUI__Type_CreateControl:
            // 警告
            if (!node) {
                UIManager << DL_Hint << L"node null" << LongUI::endl;
            }
            // 申请空间
            pControl = LongUI::CreateWidthCET<UIVideoAlpha>(type, node);
            // OOM
            if (!pControl) {
                UIManager << DL_Error << L"alloc null" << LongUI::endl;
            }
        }
        return pControl;
    }
public:
    // Render This Control
    virtual void Render() const noexcept override {
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
    // do the event
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
        auto hr = m_video.Recreate();
        // 重建父类
        if (SUCCEEDED(hr)) {
            hr = Super::Recreate();
        }
        return hr;
    }
    // constructor
    UIVideoAlpha(LongUI::UIContainer* cp, pugi::xml_node node) 
        noexcept : Super(cp, node) {
        auto hr = m_video.Initialize();
        assert(SUCCEEDED(hr));
        /*auto re =*/ m_video.HasVideo();
        hr = m_video.SetSource(L"arcv45.mp4");
        assert(SUCCEEDED(hr));
        hr = S_OK;
    }
protected:
    // destructor
    ~UIVideoAlpha() {
    }
protected:
    // video
    LongUI::Component::MMFVideo    m_video;
};

// 本Demo的配置信息
class DemoConfigure final : public LongUI::CUIDefaultConfigure {
    typedef LongUI::CUIDefaultConfigure Super;
public:
    // 构造函数
    DemoConfigure() : Super(UIManager, "longui.log") { /*this->script = &mruby;*/ this->resource = res_xml; }
    // 析构函数
    ~DemoConfigure() { if (m_hDll) { ::FreeLibrary(m_hDll); m_hDll = nullptr; } }
    // 获取地区名称
    auto GetLocaleName(wchar_t name[/*LOCALE_NAME_MAX_LENGTH*/]) noexcept ->void override {
        std::wcscpy(name, L"en-us");
    };
    // 获取控件模板
    auto GetTemplateString() noexcept ->const char* override {
        return u8R"xml(<?xml version="1.0" encoding="utf-8"?>
<!-- You can use other name not limited in 'Template' -->
<Template>
    <!-- You can use other name not limited in 'Control' -->
    <!-- Index 1 -->
    <Control desc="System look like button" margin="4,4,4,4" borderwidth="1"/>
    <!-- Index 2 -->
    <Control desc="btn.png look like button" margin="4,4,4,4" disabledmeta="1"
            normalmeta="2" hovermeta="3" pushedmeta="4"/>
    <!-- Index 3 -->
    <Control margin="4,4,4,4" borderwidth="1"/>
    <!-- Index 4 -->
    <Control margin="1,1,1,1" borderwidth="1"/>
</Template>
)xml";
    }
    // 添加自定义控件
    auto RegisterSome() noexcept ->void override {
        m_manager.RegisterControlClass(TestControl::CreateControl, "Test");
        m_manager.RegisterControlClass(UIVideoAlpha::CreateControl, "Video");
        /*if (m_hDll) {
        auto func = reinterpret_cast<LongUI::CreateControlFunction>(
        ::GetProcAddress(m_hDll, "LongUICreateControl")
        );
        m_manager.RegisterControl(func, L"DllTest");
        }*/
    };
    // return flags
    virtual auto GetConfigureFlag() noexcept ->ConfigureFlag override { 
        return Flag_OutputDebugString | Flag_RenderByCPU /*| Flag_DbgOutputFontFamily*/;
    }
    virtual auto ChooseAdapter(const DXGI_ADAPTER_DESC1 adapters[], const size_t length) noexcept -> size_t override {
        // Intel 测试
        for (size_t i = 0; i < length; ++i) {
            if (!std::wcsncmp(L"Intel", adapters[i].Description, 5))
                return i;
        }
        // 核显卡优先 
        for (size_t i = 0; i < length; ++i) {
            if (!std::wcsncmp(L"NVIDIA", adapters[i].Description, 6))
                return i;
        }
        return length;
    }
private:
    // mruby script
    //MRubyScript     mruby = MRubyScript(UIManager);
    // dll
    HMODULE         m_hDll = nullptr; // ::LoadLibraryW(L"test.dll");
};

// 应用程序入口
int WINAPI wWinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, wchar_t* lpCmdLine, int nCmdShow) {
//int wmain(int argc, wchar_t* argv[]) {
    // 设置堆信息
    ::HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);
    //int nCmdShow = SW_SHOW;
    // 本Demo的配置信息
    class DemoConfigure config;
    // MainWindow 的缓存/栈空间地址, 在x86上4字节对齐
    alignas(sizeof(void*)) size_t buffer[sizeof(MainWindow) / sizeof(size_t) + 1];
    // 初始化 OLE (OLE会调用CoInitializeEx初始化COM)
    if (SUCCEEDED(::OleInitialize(nullptr))) {
        // 初始化 UI管理器 
        if (SUCCEEDED(UIManager.Initialize(&config))) {
            // 作战控制连线!
            UIManager << DL_Hint << L"Battle Control Online!" << LongUI::endl;
            // 创建主窗口
            UIManager.CreateUIWindow<MainWindow>(test_xml, nullptr, buffer)->ShowWindow(nCmdShow);
            // 运行本程序
            UIManager.Run();
            // 作战控制终止!
            UIManager << DL_Hint << L"Battle Control Terminated!" << LongUI::endl;
            // 反初始化 UI管理器
            UIManager.Uninitialize();
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
        /*case LongUI::Event::Event_ItemClicked:
            // number button clicked event
            //this->number_button_clicked(arg.sender);
            return true;*/
        case LongUI::Event::Event_TreeBulidingFinished:
            // Event_TreeBulidingFinished could as "init" event
            this->init();
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
#include "LongUI.h"
#include "../Demos/Step3_handleeventex/demo.h"

// window xml layout
static const char* const DEMO_XML2 =
u8R"xml(<?xml version="1.0" encoding="utf-8"?>
<Window size="512, 512" templatesize="512, 0" debugshow="true" name="LongUI Demo Window">
    <Text name="display" text="0"/>
    <HorizontalLayout>
        <Button name="btn_num7" margin="4,4,4,4" borderwidth="1" text="7"/>
        <Button name="btn_num8" margin="4,4,4,4" borderwidth="1" text="8"/>
        <Button name="btn_num9" margin="4,4,4,4" borderwidth="1" text="9"/>
    </HorizontalLayout>
    <HorizontalLayout>
        <Button name="btn_num4" margin="4,4,4,4" borderwidth="1" text="4"/>
        <Button name="btn_num5" margin="4,4,4,4" borderwidth="1" text="5"/>
        <Button name="btn_num6" margin="4,4,4,4" borderwidth="1" text="6"/>
    </HorizontalLayout>
    <HorizontalLayout>
        <Button name="btn_num1" margin="4,4,4,4" borderwidth="1" text="1"/>
        <Button name="btn_num2" margin="4,4,4,4" borderwidth="1" text="2"/>
        <Button name="btn_num3" margin="4,4,4,4" borderwidth="1" text="3"/>
    </HorizontalLayout>
    <HorizontalLayout>
        <Button name="btn_plus" margin="4,4,4,4" borderwidth="1" text="+"/>
        <Button name="btn_num0" margin="4,4,4,4" borderwidth="1" text="0"/>
        <Button name="btn_minu" margin="4,4,4,4" borderwidth="1" text="-"/>
    </HorizontalLayout>
    <HorizontalLayout>
        <Button name="btn_equl" weight="1" margin="4,4,4,4" borderwidth="1" text="="/>
        <Button name="btn_clear" weight="0.5" margin="4,4,4,4" borderwidth="1" text="C"/>
    </HorizontalLayout>
</Window>
)xml";


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
        virtual auto GetConfigureFlag() noexcept ->ConfigureFlag override { 
            auto base = IUIConfigure::Flag_OutputDebugString;
            //auto base = IUIConfigure::Flag_None;
            return base | (cpu_rendering ? IUIConfigure::Flag_RenderByCPU : IUIConfigure::Flag_None); ;
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
            UIManager.CreateUIWindow<LongUI::Demo::MainWindow>(DEMO_XML2);
            // run this app
            UIManager.Run();
            // my style
            UIManager << DL_Hint << L"Battle Control Terminated!" << LongUI::endl;
            // cleanup longui
            UIManager.Uninitialize();
        }
    }
    // cleanup ole and com
    ::OleUninitialize();
    // exit
    return EXIT_SUCCESS;
}

#endif
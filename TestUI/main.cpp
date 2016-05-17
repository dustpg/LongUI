#if 1
#define _CRT_SECURE_NO_WARNINGS
#include "LongUI.h"
#include "Control/UISlider.h"
#include "Control/UICheckBox.h"
#include "Control/UIPage.h"
#include "Control/UIList.h"
#include <LongUI/luiUiTmCap.h>
#include <Platonly/luiPoFile.h>
#include "LongUI/luiUiDConf.h"

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
            <Button name="4" metagroup="1,2,3,4" script="App.click_button1($apparg)"
                margin="4,4,4,4" borderwidth="1" text="Hello, world!"/>
            <CheckBox name="btn_skinlook" text="Hello, world!"/>
            <!--Button name="uac" metagroup="1,2,3,4" text="Try  Elevate UAC Now "/-->
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
        <Button name="3" margin="4,4,4,4" metagroup="1,2,3,4" text="Hello, world!"/>
        <Button name="4" margin="4,4,4,4" borderwidth="1" text="Hello, world!"/>
    </HorizontalLayout>*/

    // <Slider name="sld_01" thumbsize="32,32" margin="4,4,4,4" size="0,64"/>
    // 

const char* test_xml_03 = u8R"xml(<?xml version="1.0" encoding="utf-8"?>
<Window size="1024, 768" name="MainWindow" debugshow="true"
    autoshow="false" clearcolor="1,1,1,0.95" >
    <HorizontalLayout name="H" size="0, 128">
        <Text  text="Hello, world!"/>
        <!--BlurText  text="Hello, world!"/>
        <Single><BlurText  text="Hello, world!" blur="2"/></Single-->
        <Text  text="Hello, world!"/>
    </HorizontalLayout>
    <Slider value="0.5" enabled="true" name="sld_01" thumbsize="32,32" margin="4,4,4,4" size="0,64"/>
    <CheckBox checkstate="checked" name="cbx_0" margin="4,4,4,4" text="复选框例子" size="0,64"/>
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
    <Single><Button name="btn_ind" text="XYZ" templateid="2" enabled="true"/></Single>
    <Edit debug="false" name="edit_demo" size="0,64" text="ABC甲乙丙123"/>
    <Button name="btn_x0" size="0, 48" borderwidth="1" enabled="false"
        margin="4,4,4,4" text="这是楷体字  這是楷體字" textfamily="KaiTi"/>
    <!--Button name="btn_x1" size="0, 64" borderwidth="1" textrichtype="core" margin="4,4,4,4" 
        text="%cHello%], %uworld!%]%f黑体字%]%f楷体字%]%p#0F0, SimHei, KaiTi"/-->
    <Button name="btn_x1" size="0, 64" borderwidth="1" textrichtype="core" margin="4,4,4,4" 
        text="%t水的化学式: H%t2%]O%t繁体而不是简体%]%]%tdòng动%]%pss06,sups ,trad, ruby"/>
</Window>
)xml";

//text="%t这是繁体字哦%]!%t這是簡體字哦%]!%ptrad, smpl"/>
/*
<List debug="ftrue" sort="true" name="lst_01" linetemplate="Text, Text">
    <ScrollBarA marginal="right"/>
    <ListLine>Combo测试<Text text="真的" templateid="4"/></ListLine>
    <ListLine>Combo测试<Text text="假的" templateid="4"/></ListLine>
</List>
*/

const char* test_xml_04 = u8R"xml(<?xml version="1.0" encoding="utf-8"?>
<Window textantimode="grayscale" size="800, 600" name="MainWindow" debugshow="true" clearcolor="1, 1, 1, 0.95" titlename="A_LongUI">
    <Slider name="sld_01" thumbsize="32,32" margin="4,4,4,4" size="0,64"/>
    <Page name="pg_1" animationduration="0.5">
        <Button name="btn_p1" borderwidth="1" margin="4,4,4,4" text="页面1, 点击到页面2" textrenderer="outline" textcontext="2"/>
        <Button name="btn_p2" borderwidth="1" margin="4,4,4,4" text="页面2, 点击到页面1"/>
    </Page>
    <Button name="btn_x1" size="0, 64" borderwidth="1" textrichtype="xml" margin="4,4,4,4" 
        __textfamily="Segoe UI"
        text="&amp;lt;普通文本 &lt;b&gt;粗体文本&lt;/b&gt; &lt;font color = &quot;#F00&quot; 
            face=&quot;KaiTi&quot;&gt;楷体&lt;/font&gt;测试{sad str=&quot;s&quot; /} "/>
    <HorizontalLayout name="H" templatesize="600, 0">
        <Button templateid="2" text="占位测试" name="btn_test"/>
        <ComboBox textformat="1" textoffsetx="4" name="cbb_01" align="left" margin="4,4,4,4" borderwidth="1">
            <List debug="ftrue" sort="true" name="lst_01" linetemplate="Text">
                <ScrollBarA marginal="right"/>
                <ListLine><Text text="真的"/></ListLine>
                <ListLine><Text text="假的"/></ListLine>
            </List>
        </ComboBox>
    </HorizontalLayout>
    <HorizontalLayout>
        <Edit name="edt1" borderwidth="1" textformat="1" text="😀"/>
        <Edit name="edt2" borderwidth="1" text="这个"/>
    </HorizontalLayout>
    <HorizontalLayout templatesize="256, 0">
        <RadioButton text="单选A" checked="true"/>
        <RadioButton text="单选B"/>
    </HorizontalLayout>
    <Single templatesize="512, 0">
        <Slider name="sld_opacity" thumbsize="32,32" margin="4,4,4,4"/>
    </Single>
    <Single templatesize="256, 0">
        <CheckBox animationduration="0.5" text="卧槽" margin="4,4,4,4"/>
    </Single>
</Window>
)xml";


const char* test_xml_05 = u8R"xml(<?xml version="1.0" encoding="utf-8"?>
<Window textantimode="grayscale" size="800, 600" name="MainWindow" debugshow="true" clearcolor="1, 1, 1, 0.95" titlename="A_LongUI">
    <HorizontalLayout>
        <VerticalLayout weight="5">
            <Null/>
            <HorizontalLayout size="0, 128">
                <Button borderwidth="1" textrichtype="xml" margin="4,4,4,4" textformat="2"
                text="字汉字&#xD;&#xA;&lt;s&gt;&lt;font face=&quot;KaiTi&quot;size=&quot;32&quot;color=
        &quot;#6CF&quot;&gt;字&lt;ruby str=&quot;hàn&quot;&gt;汉&lt;/ruby&gt;字&lt;/font&gt;&lt;/s&gt;&#xD;&#xA;字汉字" />

                <Button borderwidth="1" textrichtype="xml" margin="4,4,4,4" textformat="2"
                text="&lt;ruby str=&quot;dú dǒng kōng qì &quot;&gt;察言观色&lt;/ruby&gt;
&#xD;&#xA;
&lt;ruby str=&quot;dú&quot;&gt;察&lt;/ruby&gt;&lt;ruby str=&quot;dǒng &quot;&gt;言&lt;/ruby&gt;&lt;ruby str=&quot; kōng&quot;&gt;观&lt;/ruby&gt;&lt;ruby str=&quot;qì&quot;&gt;色&lt;/ruby&gt;
"               />

            </HorizontalLayout>

            <Button size="0, 64" borderwidth="1" textrichtype="xml" margin="4,4,4,4" textformat="2"
                text="ABCDEFG&lt;img src=&quot;1&quot; size=&quot;16,16&quot;/&gt;测试"/>

            <Null/>

            <Button size="0, 64" borderwidth="1" textrichtype="xml" margin="4,4,4,4" 
                text="&amp;lt;普通文本 &lt;b&gt;粗体文本&lt;/b&gt; &lt;font color = &quot;#F00&quot; 
                    face=&quot;KaiTi&quot;&gt;楷体&lt;/font&gt;测试 "/>

            <Button size="0, 64" borderwidth="1" textrichtype="xml" margin="4,4,4,4" 
                text="&lt;font color=&quot;#6CF&quot; size = &quot;30&quot;&gt;三十像素中嵌套的&lt;font 
        size = &quot;20&quot;&gt;二十像素&lt;/font&gt;与&lt;font size = &quot;10&quot; 
        color=&quot;#000&quot;&gt;十像&lt;font color=&quot;#00F&quot;&gt;素&lt;/font&gt;&lt;/font&gt;字体&lt;/font&gt;"/>

            <Button size="0, 64" borderwidth="1" textrichtype="xml" margin="4,4,4,4" textformat="2"
                text="使用&amp;lt;b&amp;gt;文字&amp;lt;/b&amp;gt;显示&lt;b&gt;粗体&lt;/b&gt;文字"/>

            <Button size="0, 64" borderwidth="1" textrichtype="xml" margin="4,4,4,4" textformat="2"
                text="使用&amp;lt;i&amp;gt;文字&amp;lt;/i&amp;gt;显示&lt;i&gt;斜体&lt;/i&gt;文字"/>

            <Button size="0, 64" borderwidth="1" textrichtype="xml" margin="4,4,4,4" textformat="2"
                text="使用&amp;lt;s&amp;gt;文字&amp;lt;/s&amp;gt;显示&lt;s&gt;删除线&lt;/s&gt;"/>

            <Button size="0, 64" borderwidth="1" textrichtype="xml" margin="4,4,4,4" textformat="2"
                text="使用&amp;lt;u&amp;gt;文字&amp;lt;/u&amp;gt;显示&lt;u&gt;下划线&lt;/u&gt;"/>

            <Null/>    

        </VerticalLayout>
        <VerticalLayout>
            <Button borderwidth="1" textrichtype="xml" margin="4,4,4,4" textformat="3"
                text="明月几时有？把酒问青天。不知天上宫阙，今夕是何年。我欲乘风归去，又恐琼楼玉宇，高处不胜寒。起舞弄清影，何似在人间。"/>
        </VerticalLayout>

    </HorizontalLayout>
</Window>
)xml";

const char* test_xml = test_xml_05;


constexpr char* res_xml = u8R"xml(<?xml version="1.0" encoding="utf-8"?>
<Resource>
    <!-- Bitmap区域Zone -->
    <Bitmap>
        <!-- You can use other name not limited in 'Item' -->
        <Item desc="按钮1" res="btn.png"/>
        <Item desc="复选框1" res="cbx.png"/>
    </Bitmap>
    <!-- Text Format区域Zone -->
    <TextFormat>
        <!-- You can use other name not limited in 'Item' -->
        <Item desc="雅黑" family="Microsoft YaHei" valign="middle"/>
        <Item desc="雅黑" family="Microsoft YaHei" valign="middle" align="center"/>
        <Item desc="楷体" family="KaiTi" valign="middle" align="center"
            flowdirection="top2bottom" readingdirection="right2left"/>
    </TextFormat>
    <!-- Meta区域Zone -->
    <Meta>
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
    </Meta>
</Resource>
)xml";

// MainWindow class
class MainWindow final : public LongUI::UIViewport {
    // super class
    using Super = LongUI::UIViewport;
    // frend class
    friend class Super;
private:
    // removed
    MainWindow(const MainWindow&) = delete;
    // dtor
    ~MainWindow() = default;
public:
    // ctor
    MainWindow(LongUI::XUIBaseWindow* window) : Super(window) { }
    // do some event
    virtual bool DoEvent(const LongUI::EventArgument& arg) noexcept override;
    // closed? MessageBoxW是阻塞式函数, 会卡住窗口, 不推荐
    virtual bool CanbeClosedNow() noexcept override {
        //return ::MessageBoxW(m_pWindow->GetHwnd(), L"是否退出", L"提示", MB_OKCANCEL) == IDOK;
        return true; 
    }
protected:
    // something must do before deleted
    void before_deleted() noexcept { Super::before_deleted(); }
    // init
    void initialize(pugi::xml_node node) noexcept { return Super::initialize(node); }
private:
    // clean up
    virtual void cleanup() noexcept override { this->before_deleted(); delete this; }
    // init
    void init() {
        LongUI::UIControl* ctrl = nullptr;
        auto slider = LongUI::longui_cast<LongUI::UISlider*>(m_pWindow->FindControl("sld_opacity"));
        if (slider) {
            auto window = m_pWindow;
            slider->SetValue01(window->clear_color.a);
            slider->AddEventCall([slider, window](LongUI::UIControl*) {
                window->clear_color.a = slider->GetValue01();
                window->InvalidateWindow();
                return true;
            }, LongUI::SubEvent::Event_ValueChanged);
        }
        /*auto list = LongUI::longui_cast<LongUI::UIList*>(m_pWindow->FindControl("lst_01"));
        if (list) {
            list->AddBeforSortCallBack([](LongUI::UIControl* list) {
                for (auto line : static_cast<LongUI::UIList*>(list)->GetContainer()) {
                    auto tobesorted = line->GetToBeSorted();
                    auto ptr = const_cast<wchar_t*>(tobesorted->GetText());
                    tobesorted->user_ptr = ptr;
                }
                return true;
            });
        }
        ctrl = m_pWindow->FindControl("btn_x1");
        if (ctrl) {
            auto ctrl1 = ctrl;
            auto ctrl2 = ctrl->prev->prev;
            ctrl->AddEventCall([ctrl1, ctrl2, this](UIControl*) noexcept {
                // 交换
                this->SwapChild(LongUI::MakeIteratorBI(ctrl1), LongUI::MakeIteratorBI(ctrl2));
                return true;
            }, LongUI::SubEvent::Event_ItemClicked);
        }
        if ((ctrl = m_pWindow->FindControl("btn_x0"))) {
            auto ctrl1 = ctrl;
            auto ctrl2 = ctrl->prev;
            ctrl->AddEventCall([ctrl1, ctrl2, this](UIControl*) noexcept {
                // 交换
                this->SwapChild(LongUI::MakeIteratorBI(ctrl1), LongUI::MakeIteratorBI(ctrl2));
                return true;
            }, LongUI::SubEvent::Event_ItemClicked);
        }
        if ((ctrl = m_pWindow->FindControl("btn_ind"))) {
            auto cbx = LongUI::longui_cast<LongUI::UICheckBox*>(m_pWindow->FindControl("cbx_0"));
            ctrl->AddEventCall([cbx, this](UIControl*) noexcept {
                auto stt = LongUI::CheckBoxState::State_Indeterminate;
                if (cbx->GetCheckBoxState() == LongUI::CheckBoxState::State_Indeterminate) {
                    stt = LongUI::CheckBoxState::State_Unchecked;
                }
                cbx->ForceSetCheckBoxState(stt);
                return true;
            }, LongUI::SubEvent::Event_ItemClicked);
        }*/
        if ((ctrl = m_pWindow->FindControl("btn_p1"))) {
            auto page1 = LongUI::longui_cast<LongUI::UIPage*>(m_pWindow->FindControl("pg_1"));
            ctrl->AddEventCall([page1](UIControl*) noexcept {
                page1->DisplayNextPage(1ui32);
                return true;
            }, LongUI::SubEvent::Event_ItemClicked);
        }
        if ((ctrl = m_pWindow->FindControl("btn_p2"))) {
            auto page1 = LongUI::longui_cast<LongUI::UIPage*>(m_pWindow->FindControl("pg_1"));
            ctrl->AddEventCall([page1](UIControl*) noexcept {
                page1->DisplayNextPage(0ui32);
                return true;
            }, LongUI::SubEvent::Event_ItemClicked);
        }
        if ((ctrl = m_pWindow->FindControl("btn_test"))) {
            static bool s_zoom = true;
            auto page1 = LongUI::longui_cast<LongUI::UIPage*>(m_pWindow->FindControl("pg_1"));
            ctrl->AddEventCall([page1](UIControl* id) noexcept {
                s_zoom =! s_zoom;
                UIManager.AddTimeCapsule([page1](float x) noexcept -> bool{
                    x = LongUI::EasingFunction(LongUI::AnimationType::Type_CubicEaseOut, x);
                    float z = s_zoom ? 2.f : 1.f;
                    if (s_zoom) x = -x;
                    z = z + x;
                    page1->SetZoom(z, z);
                    return false;
                }, id, 1.f);

                return true;
            }, LongUI::SubEvent::Event_ItemClicked);

        }
        /*if ((ctrl = m_pWindow->FindControl("edt1"))) {
            auto hwnd = m_pWindow->GetHwnd();
            ctrl->AddEventCall([hwnd](UIControl* c) noexcept {
                ::MessageBoxW(hwnd, L"Clicked", c->GetText(), MB_OK);
                return true;
            }, LongUI::SubEvent::Event_EditReturned);
        }*/
    }
private:
};

#if 0
// Test Video Control
class UIVideoAlpha : public LongUI::UIControl {
    // super class define
    typedef LongUI::UIControl Super;
    // close this control
    virtual void cleanup() noexcept override { this->before_deleted(); delete this; };
public:
    // create 创建
    static UIControl* CreateControl(LongUI::CreateEventType type, pugi::xml_node node) noexcept {
        // 分类判断
        UIVideoAlpha* pControl = nullptr;
        switch (type)
        {
        case LongUI::Type_Initialize:
            break;
        case LongUI::Type_Recreate:
            break;
        case LongUI::Type_Uninitialize:
            break;
        case_LongUI__Type_CreateControl:
            LongUI__CreateWidthCET(UIVideoAlpha, pControl, type, node);
        }
        return pControl;
    }
public:
    // Render This Control
    virtual void Render() const noexcept override {
        D2D1_RECT_F draw_rect;
        this->GetBorderRect(draw_rect);
        //m_btn.Render(draw_rect);
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
        
            /*if (arg.event == LongUI::Event::Event_FindControl) {
            // 检查鼠标范围
            assert(arg.pt.x < this->width && arg.pt.y < this->width && "check it");
            arg.ctrl = this;
            }
            else*/ if (arg.event == LongUI::Event::Event_TreeBuildingFinished) {
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
    UIVideoAlpha(LongUI::UIContainer* cp)
        noexcept : Super(cp) {
        auto hr = m_video.Init();
        assert(SUCCEEDED(hr));
        /*auto re =*/ m_video.HasVideo();
        hr = m_video.SetSource(L"arcv45.mp4");
        assert(SUCCEEDED(hr));
        hr = S_OK;
    }
protected:
    // destructor
    ~UIVideoAlpha() { }
protected:
    // video
    LongUI::Component::MMFVideo         m_video;
};
#endif

// 本Demo的配置信息
class DemoConfigure final : public LongUI::CUIDefaultConfigure {
    typedef LongUI::CUIDefaultConfigure Super;
public:
    // add/release
    LONGUI_BASIC_INTERFACE_IMPL;
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
    <Control desc="btn.png look like button" margin="4,4,4,4" metagroup="1,2,3,4"/>
    <!-- Index 3 -->
    <Control margin="4,4,4,4" borderwidth="1"/>
    <!-- Index 4 -->
    <Control margin="1,1,1,1" borderwidth="1"/>
    <!-- Index 5 -->
    <Control margin="1,1,1,1" metagroup="5,6,7,8,9,10,11,12,13,14,15,16"/>
</Template>
)xml";
    }
    // 添加自定义控件
    auto RegisterSome() noexcept ->void override {
        /*if (m_hDll) {
        auto func = reinterpret_cast<LongUI::CreateControlFunction>(
        ::GetProcAddress(m_hDll, "LongUICreateControl")
        );
        m_manager.RegisterControl(func, L"DllTest");
        }*/
    };
    // return flags
    virtual auto GetConfigureFlag() noexcept ->ConfigureFlag override {
        return Flag_OutputDebugString /*| Flag_RenderByCPU /*| Flag_DbgOutputFontFamily*/;
    }
    // choose
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
    // 本Demo的配置信息
    class DemoConfigure config;
    // 初始化 OLE (OLE会调用CoInitializeEx初始化COM)
    if (SUCCEEDED(::OleInitialize(nullptr))) {
        // 初始化 UI管理器 
        if (SUCCEEDED(UIManager.Initialize(&config))) {
            // 作战控制连线!
            UIManager << DL_Hint << L"Battle Control Online!" << LongUI::endl;
            // 创建主窗口 [注: 发生错误时会返回null]
            UIManager.CreateUIWindow<MainWindow>(test_xml)->ShowWindow(nCmdShow);
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
    switch (arg.event)
    {
        /*case LongUI::Event::Event_ItemClicked:
            // number button clicked event
            //this->number_button_clicked( arg.sender );
            return true;*/
    case LongUI::Event::Event_TreeBuildingFinished:
        // Event_TreeBuildingFinished could as "init" event
        this->init();
        // super will send this event to children
        __fallthrough;
    default:
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
namespace LongUI {
    namespace Demo {
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
    }
}

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
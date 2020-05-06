// Gui
#include <core/ui_ctrlmeta.h>
#include <control/ui_scale.h>
#include <util/ui_aniamtion.h>
#include <control/ui_image.h>
#include <core/ui_color_list.h>
#include <control/ui_scrollbar.h>
// C++
#include <cassert>
// Private
#include "../private/ui_private_control.h"

// ui namespace
namespace LongUI {
    // UIScrollBar类 元信息
    LUI_CONTROL_META_INFO(UIScrollBar, "scrollbar");
    // ScrollBar 私有数据
    struct UIScrollBar::Private : CUIObject {
        // 控件类型
        enum ControlType : uint32_t {
            Type_UpTop = 0,
            Type_DownTop,
            Type_Slider,
            Type_UpBottom,
            Type_DownBottom,
            TYPE_COUNT,
        };
        // 按钮类
        using SBButton = UIImage;
        // 滑条类
        using SBSlider = UIScale;
        // 构造函数
        Private(UIScrollBar& parent) noexcept;
#ifndef NDEBUG
        // 占位指针位 调试
        void*               placeholder_debug1 = nullptr;
#endif
        // 滚动条按钮 顶上
        SBButton            up_top;
        // 滚动条按钮 顶下
        SBButton            down_top;
        // 滑动条控件 中央
        SBSlider            slider;
        // 滚动条按钮 底上
        SBButton            up_bottom;
        // 滚动条按钮 底下
        SBButton            down_bottom;
    };
    /// <summary>
    /// Privates the scroll bar.
    /// </summary>
    /// <param name="parent">The parent.</param>
    /// <returns></returns>
    UIScrollBar::Private::Private(UIScrollBar& parent) noexcept
        : up_top(&parent), down_top(&parent), slider(&parent)
        , up_bottom(&parent), down_bottom(&parent) {
#ifndef NDEBUG
        up_top.name_dbg     = "scrollbar::up_top";
        down_top.name_dbg   = "scrollbar::down_top";
        slider.name_dbg     = "scrollbar::slider";
        up_bottom.name_dbg  = "scrollbar::up_bottom";
        down_bottom.name_dbg= "scrollbar::down_bottom";
#endif
        //UIControlPrivate::SetGuiEvent2Parent(up_top);
        //UIControlPrivate::SetGuiEvent2Parent(down_top);
        UIControlPrivate::SetGuiEvent2Parent(slider);
        //UIControlPrivate::SetGuiEvent2Parent(up_bottom);
        //UIControlPrivate::SetGuiEvent2Parent(down_bottom);

        UIControlPrivate::SetParentData(up_top,   Type_UpTop);
        UIControlPrivate::SetParentData(down_top, Type_DownTop);
        UIControlPrivate::SetParentData(slider,   Type_Slider);
        UIControlPrivate::SetParentData(up_bottom, Type_UpBottom);
        UIControlPrivate::SetParentData(down_bottom, Type_DownBottom);
        UIControlPrivate::SetFlex(slider, 1.f);

        down_top.SetVisible(false);
        up_bottom.SetVisible(false);
        slider.SetMin(0.f);
    }
}


/// <summary>
/// Initializes a new instance of the <see cref="UIScrollBar" /> class.
/// </summary>
/// <param name="o">The o.</param>
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UIScrollBar::UIScrollBar(AttributeOrient o, UIControl* parent,
    const MetaControl& meta) noexcept : Super(impl::ctor_lock(parent), meta) {
    // 检查长度
    constexpr auto sizeof_private = sizeof(*m_private);
#ifdef LUI_ACCESSIBLE
    // 子控件为本控件的组成部分
    m_pAccCtrl = nullptr;
#endif
    // 私有实现
    m_private = new(std::nothrow) Private{ *this };
    // OOM处理
    this->ctor_failed_if(m_private);
    // 设置方向
    const bool orient = o & 1;
    m_state.orient = orient;
    if (m_private) 
        UIControlPrivate::SetOrient(m_private->slider, orient);
    // 构造锁
    impl::ctor_unlock();
}

/// <summary>
/// Finalizes an instance of the <see cref="UIScrollBar"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIScrollBar::~UIScrollBar() noexcept {
    m_state.destructing = true;
    if (m_private) delete m_private;
}

/// <summary>
/// Renders this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIScrollBar::Update() noexcept {
    //// TODO: disabled 状态修改? 广播给子控件
    //if (m_state.style_state_changed) {
    //    //m_state.style_state_changed = false;
    //    if (m_pAnimation) {

    //    }
    //}
    return Super::Update();
}

/// <summary>
/// Gets the value.
/// </summary>
/// <returns></returns>
auto LongUI::UIScrollBar::GetValue() const noexcept -> float {
    return m_private->slider.GetValue();
}


/// <summary>
/// Sets the value.
/// </summary>
/// <param name="v">The v.</param>
/// <returns></returns>
void LongUI::UIScrollBar::SetValue(float v) noexcept {
    m_private->slider.SetValue(v);
}


/// <summary>
/// Sets the maximum.
/// </summary>
/// <param name="v">The v.</param>
/// <returns></returns>
void LongUI::UIScrollBar::SetMax(float v) noexcept {
    m_private->slider.SetMax(v);
}

/// <summary>
/// Sets the page increment.
/// </summary>
/// <param name="pi">The pi.</param>
/// <returns></returns>
void LongUI::UIScrollBar::SetPageIncrement(float pi) noexcept {
    m_private->slider.SetPageIncrement(pi);
}

/// <summary>
/// Sets the increment.
/// </summary>
/// <param name="pi">The pi.</param>
/// <returns></returns>
void LongUI::UIScrollBar::SetIncrement(float pi) noexcept {
    m_private->slider.increment = pi;
}

/// <summary>
/// Initializes the bar.
/// </summary>
/// <returns></returns>
void LongUI::UIScrollBar::init_bar() noexcept {
    AttributeAppearance aut, adt, asd, ast, aub, adb, ats;
    // 根据方向确定初始化类型
    if (this->GetOrient() == Orient_Horizontal) {
        aut = Appearance_ScrollBarButtonLeft;
        adt = Appearance_ScrollBarButtonRight;
        asd = Appearance_None;
        ast = Appearance_ScrollbarThumbH;
        aub = Appearance_ScrollBarButtonLeft;
        adb = Appearance_ScrollBarButtonRight;
        ats = Appearance_ScrollbarTrackH;
    }
    // 垂直方向
    else {
        aut = Appearance_ScrollBarButtonUp;
        adt = Appearance_ScrollBarButtonDown;
        asd = Appearance_None;
        ast = Appearance_ScrollbarThumbV;
        aub = Appearance_ScrollBarButtonUp;
        adb = Appearance_ScrollBarButtonDown;
        ats = Appearance_ScrollbarTrackV;
    }
    // 设置Appearance类型
    UIControlPrivate::SetAppearanceIfNotSet(m_private->up_top, aut);
    UIControlPrivate::SetAppearanceIfNotSet(m_private->down_top, adt);
    UIControlPrivate::SetAppearanceIfNotSet(m_private->slider, asd);
    UIControlPrivate::SetAppearanceIfNotSet(m_private->slider.thumb, ast);
    UIControlPrivate::SetAppearanceIfNotSet(m_private->up_bottom, aub);
    UIControlPrivate::SetAppearanceIfNotSet(m_private->down_bottom, adb);
    UIControlPrivate::SetAppearanceIfNotSet(*this, ats);
}

/// <summary>
/// Does the event.
/// </summary>
/// <param name="sender">The sender.</param>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UIScrollBar::DoEvent(UIControl * sender,
    const EventArg & e) noexcept -> EventAccept {
    switch (e.nevent)
    {
    case NoticeEvent::Event_Initialize:
        // 初始化
        this->init_bar();
        return Event_Accept;
    case NoticeEvent::Event_UIEvent:
        // Gui事件: 数据修改事件向上传递
    {
        const auto ge = static_cast<const EventGuiArg&>(e).GetEvent();
        assert(sender == &m_private->slider);
        return this->TriggerEvent(ge);
    }
    default:
        // 基类处理
        return Super::DoEvent(sender, e);
    }
}

/// <summary>
/// Does the mouse event.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UIScrollBar::DoMouseEvent(
    const MouseEventArg & e) noexcept -> EventAccept {
    switch (e.type)
    {
    case MouseEvent::Event_LButtonDown:
        // 鼠标左键按下
        // TODO: 持续按下
        if (m_pHovered) {
            // 分类讨论
            switch (UIControlPrivate::GetParentData(*m_pHovered))
            {
            case Private::Type_UpTop:
                // 顶上
                m_private->slider.Decrease();
                break;
            case Private::Type_DownTop:
                // 顶下
                m_private->slider.DecreasePage();
                break;
            case Private::Type_UpBottom:
                // 底上
                m_private->slider.IncreasePage();
                break;
            case Private::Type_DownBottom:
                // 底下
                m_private->slider.Increase();
                break;
            };
        }
        break;
    }
    return Super::DoMouseEvent(e);
}


#ifdef LUI_ACCESSIBLE
#include <accessible/ui_accessible_event.h>
#include <accessible/ui_accessible_type.h>
#include <core/ui_string.h>

/// <summary>
/// Accessibles the specified .
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
auto LongUI::UIScrollBar::accessible(const AccessibleEventArg& args) noexcept -> EventAccept {
    switch (args.event)
    {
        using get0_t = AccessibleGetPatternsArg;
        using get1_t = AccessibleGetCtrlTypeArg;
        using get2_t = AccessibleGetAccNameArg;
    case AccessibleEvent::Event_GetPatterns:
        // + 继承基类行为模型
        Super::accessible(args);
        // + 范围值的行为模型
        static_cast<const get0_t&>(args).patterns |= Pattern_Range;
        return Event_Accept;
    case AccessibleEvent::Event_All_GetControlType:
        // 获取控件类型
        static_cast<const get1_t&>(args).type =
            AccessibleControlType::Type_ScrollBar;
        return Event_Accept;
    case AccessibleEvent::Event_RangeValue_IsReadOnly:
        // 滚动条可以随便改写
        return Event_Ignore;
    case AccessibleEvent::Event_Range_GetValue:
        // 获取当前值
        static_cast<const AccessibleRGetValueArg&>(args).value
            = m_private->slider.GetValue();
        return Event_Accept;
    case AccessibleEvent::Event_Range_SetValue:
        // 设置当前值
        m_private->slider.SetValue(static_cast<float>(
            static_cast<const AccessibleRSetValueArg&>(args).value
            ));
        return Event_Accept;
    case AccessibleEvent::Event_Range_GetMax:
        // 获取最大值
        static_cast<const AccessibleRGetMaxArg&>(args).value
            = m_private->slider.GetMax();
        return Event_Accept;
    case AccessibleEvent::Event_Range_GetMin:
        // 获取最小值
        static_cast<const AccessibleRGetMinArg&>(args).value
            = m_private->slider.GetMin();
        return Event_Accept;
    case AccessibleEvent::Event_Range_GetLargeStep:
        // 获取大步长
        static_cast<const AccessibleRGetLargeStepArg&>(args).value
            = m_private->slider.GetPage();
        return Event_Accept;
    case AccessibleEvent::Event_Range_GetSmallStep:
        // 获取小步长
        static_cast<const AccessibleRGetSmallStepArg&>(args).value
            = m_private->slider.increment;
        return Event_Accept;
    }
    return Super::accessible(args);
}

#endif

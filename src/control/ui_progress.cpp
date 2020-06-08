// Gui
#include <core/ui_ctrlmeta.h>
#include <control/ui_caption.h>
#include <control/ui_progress.h>
#include <control/ui_box.h>
#include <util/ui_little_math.h>
// Debug
#include <debugger/ui_debug.h>
// Private
#include "../private/ui_private_control.h"

#include <algorithm>


// ui namespace
namespace LongUI {
    // UIProgress类 元信息
    LUI_CONTROL_META_INFO(UIProgress, "progressmeter");
}


/// <summary>
/// Initializes a new instance of the <see cref="UICaption" /> class.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UIProgress::UIProgress(const MetaControl& meta) noexcept : Super(meta),
    m_oBar(this), m_oRemainder(this) {
    // 阻隔鼠标事件写入false之前需要写入
    m_oBar.RefInheritedMask() = State_MouseCutInher;
    m_oRemainder.RefInheritedMask() = State_MouseCutInher;
    // 阻隔鼠标事件
    m_state.mouse_continue = false;
#ifdef LUI_ACCESSIBLE
    // 没有逻辑子控件
    m_pAccCtrl = nullptr;
#endif
    // 基本样式
    m_oBox.margin = { 4.f, 2.f, 4.f, 1.f };
    m_oBox.border = { 1.f, 1.f, 1.f, 1.f };
    // 私有实现
#ifndef NDEBUG
    m_oBar.name_dbg = "progress::bar";
    m_oRemainder.name_dbg = "progress::remainder";
#endif
}

/// <summary>
/// Finalizes an instance of the <see cref="UIProgress"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIProgress::~UIProgress() noexcept {
    // 需要提前释放
    m_state.destructing = true;
}



/// <summary>
/// Does the event.
/// </summary>
/// <param name="sender">The sender.</param>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UIProgress::DoEvent(UIControl * sender,
    const EventArg & e) noexcept -> EventAccept {
    // 初始化
    if (e.nevent == NoticeEvent::Event_Initialize) {
        this->init_bar();
        this->adjust_flex();
    }
    // 基类处理
    return Super::DoEvent(sender, e);
}


/// <summary>
/// Updates this instance.
/// </summary>
/// <returns></returns>
//void LongUI::UIProgress::Update() noexcept {
//    // 重新设置
//    if (m_state.custom_data) {
//        m_state.custom_data = false;
//        this->adjust_flex();
//        this->Invalidate();
//    }
//    // 超类处理
//    return Super::Update();
//}

/// <summary>
/// Updates this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIProgress::Update(UpdateReason reason) noexcept {
    // 数据修改
    if (reason & Reason_ValueTextChanged) {
        this->adjust_flex();
        this->Invalidate();
    }
    // 超类处理
    return Super::Update(reason);
}

/// <summary>
/// Adds the attribute.
/// </summary>
/// <param name="key">The key.</param>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::UIProgress::add_attribute(uint32_t key, U8View value) noexcept {
    // 使用列表
    constexpr auto BKDR_MAX         = 0x001cbcf0_ui32;
    constexpr auto BKDR_MODE        = 0x0eb84f77_ui32;
    constexpr auto BKDR_VALUE       = 0x246df521_ui32;

    // 分类处理
    switch (key)
    {
    case BKDR_VALUE:
        // value        : 初始数值
        m_value = value.ToFloat();
        break;
    case BKDR_MAX:
        // max          : 最大数值
        m_max = value.ToFloat();
        break;
    case BKDR_MODE:
        // mode         : determined/undetermined
        if (*value.begin() == 'u')
            m_oStyle.state = m_oStyle.state | State_Indeterminate;
        break;
    default:
        // 超类处理
        return Super::add_attribute(key, value);
    }
}

/// <summary>
/// Sets the maximum.
/// </summary>
/// <param name="max">The maximum.</param>
/// <returns></returns>
void LongUI::UIProgress::SetMax(float max) noexcept {
    assert(max >= 1.f);
    if (m_max != max) {
        m_max = max;
        this->NeedUpdate(Reason_ChildLayoutChanged | Reason_ValueTextChanged);
    }
}

/// <summary>
/// Sets the value.
/// </summary>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::UIProgress::SetValue(float value) noexcept {
    if (m_value != value) {
        m_value = value;
        this->NeedUpdate(Reason_ChildLayoutChanged | Reason_ValueTextChanged);
    }
}

/// <summary>
/// Initializes the bar.
/// </summary>
/// <returns></returns>
void LongUI::UIProgress::init_bar() noexcept {
    AttributeAppearance thisapp, barapp;
    // 根据方向确定初始化类型
    if (this->GetOrient() == Orient_Horizontal) {
        thisapp = Appearance_ProgressBarH;
        barapp = Appearance_ProgressChunkH;
    }
    // 垂直方向
    else {
        thisapp = Appearance_ProgressBarV;
        barapp = Appearance_ProgressChunkV;
    }
    // 设置
    UIControlPrivate::SetAppearanceIfWeakNon(*this, thisapp);
    UIControlPrivate::SetAppearanceIfWeakNon(m_oBar, barapp);
}



/// <summary>
/// Adjusts the flex.
/// </summary>
/// <returns></returns>
void LongUI::UIProgress::adjust_flex() noexcept {
    // 数值设定
    m_max = std::max(m_max, 1.f);
    m_value = detail::clamp(m_value, 0.f, m_max);
    // 不确定情况
    if (m_oStyle.state & State_Indeterminate) {
        UIControlPrivate::SetFlex(m_oBar, 1.f);
        UIControlPrivate::SetFlex(m_oRemainder, 0.f);
    }
    else {
        UIControlPrivate::SetFlex(m_oBar, m_value);
        UIControlPrivate::SetFlex(m_oRemainder, m_max - m_value);
    }
}


/// <summary>
/// Adds the child.
/// </summary>
/// <param name="child">The child.</param>
/// <returns></returns>
//void LongUI::UIProgress::add_child(UIControl& child) noexcept {
//    // 已经连接好
//    if (m_private) {
//        // caption控件?
//        UIControl* target;
//        if (uisafe_cast<UICaption>(&child))
//            target = &m_private->head;
//        else 
//            target = &m_private->body;
//        UIControlPrivate::CallAddChild(*target, child);
//    }
//    // 还未连接好
//    else Super::add_child(child);
//}


#ifdef LUI_ACCESSIBLE

#include <accessible/ui_accessible_callback.h>
#include <accessible/ui_accessible_event.h>
#include <accessible/ui_accessible_type.h>
#include <core/ui_string.h>

/// <summary>
/// Accessibles the specified .
/// </summary>
/// <param name="args">The arguments.</param>
/// <returns></returns>
auto LongUI::UIProgress::accessible(const AccessibleEventArg& args) noexcept -> EventAccept {
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
            AccessibleControlType::Type_ProgressBar;
        return Event_Accept;
    case AccessibleEvent::Event_RangeValue_IsReadOnly:
        // 进度条对于 Accessible: 数据只读
        return Event_Accept;
    case AccessibleEvent::Event_Range_GetValue:
        // 获取当前值
        static_cast<const AccessibleRGetValueArg&>(args).value
            = this->GetValue();
        return Event_Accept;
    case AccessibleEvent::Event_Range_SetValue:
        // 设置当前值
        this->SetValue(static_cast<float>(
            static_cast<const AccessibleRSetValueArg&>(args).value
            ));
        return Event_Accept;
    case AccessibleEvent::Event_Range_GetMax:
        // 获取最大值
        static_cast<const AccessibleRGetMaxArg&>(args).value
            = this->GetMax();
        return Event_Accept;
    case AccessibleEvent::Event_Range_GetMin:
        // 获取最小值
        static_cast<const AccessibleRGetMinArg&>(args).value = 0.;
        return Event_Accept;
    case AccessibleEvent::Event_Range_GetLargeStep:
        // 获取大步长
        static_cast<const AccessibleRGetLargeStepArg&>(args).value = 0.;
        return Event_Accept;
    case AccessibleEvent::Event_Range_GetSmallStep:
        // 获取小步长
        static_cast<const AccessibleRGetSmallStepArg&>(args).value = 0.;
        return Event_Accept;
    }
    return Super::accessible(args);
}

#endif

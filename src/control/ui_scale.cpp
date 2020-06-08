// Gui
#include <core/ui_ctrlmeta.h>
#include <input/ui_kminput.h>
#include <control/ui_scale.h>
#include <control/ui_image.h>
#include <container/pod_hash.h>
#include <util/ui_little_math.h>
#include <resource/ui_image_res.h>
// Private
#include "../private/ui_private_control.h"
// C++
#include <cassert>
#include <algorithm>

#ifndef NDEBUG
#include <debugger/ui_debug.h>
#endif

// ui namespace
namespace LongUI {
    // UIScale类 元信息
    LUI_CONTROL_META_INFO(UIScale, "scale");
}

/// <summary>
/// Initializes a new instance of the <see cref="UIScale"/> class.
/// </summary>
/// <param name="meta">The meta data.</param>
LongUI::UIScale::UIScale(const MetaControl& meta) noexcept : Super(meta), thumb(this) {
    m_state.capturable = true;
    m_state.focusable = true;
    //UIControlPrivate::SetFocusable(this->thumb, false);
#ifndef NDEBUG
    assert(this->thumb.IsFocusable() == false);
    thumb.name_dbg = "scale::thumb";
#endif
    this->setup_weakapp();
}

/// <summary>
/// Finalizes an instance of the <see cref="UIScale"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIScale::~UIScale() noexcept {
    // 额外处理
    m_state.destructing = true;
}

/// <summary>
/// Updates this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIScale::Update(UpdateReason reason) noexcept {
    // 污了
    if (reason & (Reason_SizeChanged | Reason_ValueTextChanged)) {
        // 本类只能存在唯一的子元素(thumb)
        assert(this->GetChildrenCount() == 1 && "thumb ony");
        assert(*this->begin() == this->thumb && "thumb ony");
        this->refresh_thumb_size();
        this->refresh_thumb_postion();
        this->Invalidate();
    }
    // 超类处理
    Super::Update(reason);
}

/// <summary>
/// Updates the thumb postion.
/// </summary>
/// <returns></returns>
void LongUI::UIScale::refresh_thumb_postion() noexcept {
    const auto csize = this->RefBox().GetContentSize();
    const auto ssize = this->thumb.GetSize();
    assert(m_fValue >= m_fMin && m_fValue <= m_fMax && "out of range");
    const auto normalization = (m_fValue - m_fMin) / (m_fMax - m_fMin);
    Point2F pos{ 0 };
    // 水平方向
    if (this->GetOrient() == Orient_Horizontal) {
        const auto width = csize.width - ssize.width;
        pos.x = width * normalization;
    }
    // 垂直方向
    else {
        const auto height = csize.height - ssize.height;
        pos.y = height * normalization;
        //LUIDebug(Hint) << m_fValue << endl;
    }
    // 设置位置
    this->thumb.SetPos(pos);
}

/// <summary>
/// Refreshes the size of the thumb.
/// </summary>
/// <returns></returns>
void LongUI::UIScale::refresh_thumb_size() noexcept {
    const auto length = m_fMax - m_fMin;
    const auto csize = this->RefBox().GetContentSize();
    const auto ratio = m_fPageIncrement / (m_fPageIncrement + length);
    const auto xsize = this->thumb.RefStyle().maxsize;
    Size2F size = this->thumb.GetMinSize();
    // 水平方向
    if (this->GetOrient() == Orient_Horizontal) {
        size.width = std::max(csize.width * ratio, size.width);
        size.width = std::min(xsize.width, size.width);
    }
    // 垂直方向
    else {
        size.height = std::max(csize.height * ratio, size.height);
        size.height = std::min(xsize.height, size.height);
    }
    this->resize_child(this->thumb, size);
}

/// <summary>
/// Sets the value.
/// </summary>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::UIScale::SetValue(float value) noexcept {
    assert(m_fMax >= m_fMin);
    // 整型转换
    if (true) value = LongUI::RoundInGuiLevel(value);
    // 范围检查
    const auto newv = detail::clamp(value, m_fMin, m_fMax);
    // 差不多?
    if (LongUI::IsSameInGuiLevel(newv, m_fValue)) return;
    // 修改数据
    m_fValue = newv;
    // 触发修改GUI事件
    this->TriggerEvent(this->_onChange());
    this->NeedUpdate(Reason_ValueTextChanged);
}

/// <summary>
/// Sets the minimum.
/// </summary>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::UIScale::SetMin(float min_value) noexcept {
    m_fMin = std::min(min_value, m_fMax);
    // 越界检查
    if (m_fMin > m_fValue) this->SetValue(m_fMin);
    else this->NeedUpdate(Reason_ValueTextChanged);
}


/// <summary>
/// Sets the page increment.
/// </summary>
/// <param name="pi">The pi.</param>
/// <returns></returns>
void LongUI::UIScale::SetPageIncrement(float pi) noexcept {
    m_fPageIncrement = pi;
    this->NeedUpdate(Reason_ValueTextChanged);
}

/// <summary>
/// Sets the maximum.
/// </summary>
/// <param name="max_value">The maximum value.</param>
/// <returns></returns>
void LongUI::UIScale::SetMax(float max_value) noexcept {
    m_fMax = std::max(max_value, m_fMin);
    if (m_fMax < m_fValue) this->SetValue(m_fMax);
    this->NeedUpdate(Reason_ChildLayoutChanged);
}

/// <summary>
/// Does the event.
/// </summary>
/// <param name="sender">The sender.</param>
/// <param name="e">The e.</param>
/// <returns></returns>
//auto LongUI::UIScale::DoEvent(UIControl * sender,
//                          const EventArg & e) noexcept -> EventAccept {
//    // 初始化
//    if (e.nevent == NoticeEvent::Event_Initialize) {
//        this->init_slider();
//    }
//    // 基类处理
//    return Super::DoEvent(sender, e);
//}


/// <summary>
/// Does the event.
/// </summary>
/// <param name="sender">The sender.</param>
/// <param name="e">The e.</param>
/// <returns></returns>
void LongUI::UIScale::add_attribute(uint32_t key, U8View value) noexcept {
    constexpr auto BKDR_ORIENT = 0xeda466cd_ui32;
    Super::add_attribute(key, value);
    if (key == BKDR_ORIENT) this->setup_weakapp();
}

/// <summary>
/// Does the mouse event.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UIScale::DoMouseEvent(const MouseEventArg& e) noexcept->EventAccept {
    // 鼠标相对本控件位置
    Point2F pt_this = { e.px, e.py };
    this->MapFromWindow(pt_this);

    // 移动滑块
    auto hold_lbtn_move = [this, &pt_this]() noexcept->EventAccept {
        if (m_pHovered) {
            assert(m_pHovered == &this->thumb);
            const auto csize = this->RefBox().GetContentSize();
            const auto ssize = this->thumb.GetSize();
            const int i = this->GetOrient() == Orient_Horizontal ? 0 : 1;
            const auto width = i[&csize.width] - i[&ssize.width];
            const auto x = i[&pt_this.x] - m_fClickOffset;
            const auto value = (x / width) * (m_fMax - m_fMin) + m_fMin;
            this->SetValue(value);
            return Event_Accept;
        }
        else return Event_Ignore;
    };


    // 基类处理基本消息
    switch (e.type)
    {
    case MouseEvent::Event_MouseMove:
        // 鼠标移动 + 左键按下
        if (e.modifier & LongUI::Modifier_LButton) {
            return hold_lbtn_move();
        }
        break;
    case MouseEvent::Event_LButtonDown:
        this->mouse_click(pt_this);
        break;
    }
    // 基类处理剩余消息
    return Super::DoMouseEvent(e);
}

/// <summary>
/// Mouses the click.
/// </summary>
/// <returns></returns>
void LongUI::UIScale::mouse_click(Point2F pt) noexcept {
    const auto thumb_pos = this->thumb.GetPos();
    // 在滑块的上方
    if (m_pHovered) {
        assert(*m_pHovered == this->thumb);
        if (this->GetOrient() == Orient_Horizontal)
            m_fClickOffset = pt.x - thumb_pos.x;
        else
            m_fClickOffset = pt.y - thumb_pos.y;
    }
    // 在逻辑前方
    else if (pt.x < thumb_pos.x || pt.y < thumb_pos.y) {
        this->DecreasePage();
    }
    // 在逻辑后方
    else {
        this->IncreasePage();
    }
}

/// <summary>
/// setup weak app
/// </summary>
/// <returns></returns>
void LongUI::UIScale::setup_weakapp() noexcept {
    AttributeAppearance this_app, thumbapp;
    if (this->GetOrient() == Orient_Horizontal) {
        this_app = Appearance_WeakApp | Appearance_ScaleH;
        thumbapp = Appearance_WeakApp | Appearance_ScaleThumbH;
    }
    else {
        this_app = Appearance_WeakApp | Appearance_ScaleV;
        thumbapp = Appearance_WeakApp | Appearance_ScaleThumbV;
    }
    UIControlPrivate::SetAppearanceIfWeak(*this, this_app);
    UIControlPrivate::SetAppearanceIfWeak(thumb, thumbapp);
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
auto LongUI::UIScale::accessible(const AccessibleEventArg& args) noexcept -> EventAccept {
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
            AccessibleControlType::Type_Slider;
        return Event_Accept;
    case AccessibleEvent::Event_RangeValue_IsReadOnly:
        // TODO: 数据只读?
        return Event_Ignore;
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
        static_cast<const AccessibleRGetMinArg&>(args).value
            = this->GetMin();
        return Event_Accept;
    case AccessibleEvent::Event_Range_GetLargeStep:
        // 获取大步长
        static_cast<const AccessibleRGetLargeStepArg&>(args).value
            = this->GetPage();
        return Event_Accept;
    case AccessibleEvent::Event_Range_GetSmallStep:
        // 获取小步长
        static_cast<const AccessibleRGetSmallStepArg&>(args).value
            = this->increment;
        return Event_Accept;
    }
    return Super::accessible(args);
}

#endif

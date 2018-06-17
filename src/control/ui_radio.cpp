// Gui
#include <control/ui_radio.h>
#include <debugger/ui_debug.h>
#include <control/ui_ctrlmeta.h>
#include <control/ui_radiogroup.h>
// 子控件
#include <control/ui_image.h>
#include <control/ui_label.h>
#include <control/ui_box_layout.h>
// Private
#include "../private/ui_private_control.h"

// ui namespace
namespace LongUI {
    // UIRadio类 元信息
    LUI_CONTROL_META_INFO(UIRadio, "radio");
    // UIRadioGroup类 元信息
    LUI_CONTROL_META_INFO(UIRadioGroup, "radiogroup");
    // UIRadio私有信息
    struct UIRadio::Private : CUIObject {
        // 构造函数
        Private(UIRadio& btn) noexcept;
#ifndef NDEBUG
        // 调试占位
        void*               placeholder_debug1 = nullptr;
#endif
        // 图像控件
        UIImage             image;
        // 标签控件
        UILabel             label;
    };
    /// <summary>
    /// button privates data/method
    /// </summary>
    /// <param name="btn">The BTN.</param>
    /// <returns></returns>
    UIRadio::Private::Private(UIRadio& btn) noexcept
        : image(&btn), label(&btn) {
        //UIControlPrivate::SetFocusable(image, false);
        //UIControlPrivate::SetFocusable(label, false);
#ifndef NDEBUG
        image.name_dbg = "radio::image";
        label.name_dbg = "radio::label";
        assert(image.IsFocusable() == false);
        assert(label.IsFocusable() == false);
        label.SetText(L"单选框");
#endif
    }
}

/// <summary>
/// Initializes a new instance of the <see cref="UIRadio" /> class.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UIRadio::UIRadio(UIControl* parent, const MetaControl& meta) noexcept
    : Super(parent, meta) {
    m_state.focusable = true;
    this->SetOrient(Orient_Horizontal);
    m_oStyle.align = AttributeAlign::Align_Center;
    // 原子性, 子控件为本控件的组成部分
    m_state.atomicity = true;
    // 没有逻辑子控件
#ifdef LUI_ACCESSIBLE
    m_pAccCtrl = nullptr;
#endif
    // XXX: 硬编码
    m_oBox.margin = { 4, 2, 4, 2 };
    m_oBox.padding = { 4, 1, 2, 1 };
    // 私有实现
    m_private = new(std::nothrow) Private{ *this };
    // OOM处理
    this->ctor_failed_if(m_private);
}


/// <summary>
/// Finalizes an instance of the <see cref="UIRadio"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIRadio::~UIRadio() noexcept {
    // 存在提前释放子控件, 需要标记"在析构中"
    m_state.destructing = true;
    // 释放私有数据
    if (m_private) delete m_private;
}

/// <summary>
/// UIs the radio.
/// </summary>
/// <returns></returns>
void LongUI::UIRadio::Update() noexcept {
    // 父节点修改了?
    if (m_state.parent_changed) {
        // uisafe_cast 空指针安全
        m_pRadioGroup = uisafe_cast<UIRadioGroup>(m_pParent);
    }
    // 父类调用
    Super::Update();
}


/// <summary>
/// Does the event.
/// </summary>
/// <param name="sender">The sender.</param>
/// <param name="arg">The argument.</param>
/// <returns></returns>
auto LongUI::UIRadio::DoEvent(
    UIControl * sender, const EventArg & arg) noexcept -> EventAccept {
    // 初始化
    if (arg.nevent == NoticeEvent::Event_Initialize) {
        this->init_radio();
    }
    return Super::DoEvent(sender, arg);
}

/// <summary>
/// Initializes the radio.
/// </summary>
/// <returns></returns>
void LongUI::UIRadio::init_radio() noexcept {
    if (!m_private) return;
    constexpr auto iapp = Appearance_Radio;
    UIControlPrivate::SetAppearanceIfNotSet(m_private->image, iapp);
}


/// <summary>
/// Does the mouse event.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UIRadio::DoMouseEvent(const MouseEventArg & e) noexcept -> EventAccept {
    // 左键弹起 修改状态
    switch (e.type)
    {
    case LongUI::MouseEvent::Event_LButtonUp:
        // 修改为checked状态
        this->SetChecked(true);
        [[fallthrough]];
    default:
        return Super::DoMouseEvent(e);
    }
}


/// <summary>
/// Sets the seleced.
/// </summary>
/// <param name="checked">if set to <c>true</c> [checked].</param>
void LongUI::UIRadio::SetChecked(bool checked) {
    assert(m_private && "BUG");
    if (this->GetStyle().state.checked == checked) return;
    // 修改状态
    const auto statetp = StyleStateType::Type_Checked;
    this->StartAnimation({ statetp , checked });
    m_private->image.StartAnimation({ statetp , checked });
    // 检查回馈
    if (checked && m_pRadioGroup) {
        m_pRadioGroup->SetChecked(*this);
    }
}

/// <summary>
/// Sets the text.
/// </summary>
/// <param name="str">The string.</param>
/// <param name="len">The length.</param>
/// <returns></returns>
void LongUI::UIRadio::SetText(CUIString&& text) noexcept {
    m_private->label.SetText(std::move(text));
}

/// <summary>
/// Sets the text.
/// </summary>
/// <param name="text">The text.</param>
/// <returns></returns>
void LongUI::UIRadio::SetText(const CUIString & text) noexcept {
    return this->SetText(CUIString{ text });
}


/// <summary>
/// Sets the text.
/// </summary>
/// <param name="text">The text.</param>
/// <returns></returns>
void LongUI::UIRadio::SetText(WcView text) noexcept {
    return this->SetText(CUIString{ text });
}

/// <summary>
/// Gets the text.
/// </summary>
/// <returns></returns>
auto LongUI::UIRadio::GetText() const noexcept -> const wchar_t* {
    return m_private->label.GetText();
}

/// <summary>
/// Gets the text string.
/// </summary>
/// <returns></returns>
auto LongUI::UIRadio::GetTextString() const noexcept -> const CUIString&{
    return m_private->label.GetTextString();
}

// ----------------------------------------------------------------------------
//                             UIRadioGroup
// ----------------------------------------------------------------------------


/// <summary>
/// Finalizes an instance of the <see cref="UIRadioGroup"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIRadioGroup::~UIRadioGroup() noexcept {
}


/// <summary>
/// Updates this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIRadioGroup::Update() noexcept {
    // 清理checked标记
    auto clear_checked = [this]() noexcept {
        // 查找已有的
        for (const auto& x : (*this))
            if (&x == m_pChecked) return;
        // 没有找到
        m_pChecked = nullptr;
        // 修改事件?
        this->TriggrtEvent(_changed());
    };
    // 子节点修改过?
    if (m_state.child_i_changed) {
        // 有才查找
        if (m_pChecked) clear_checked();
    }
    // 父类处理
    Super::Update();
}

/// <summary>
/// Initializes a new instance of the <see cref="UIRadioGroup" /> class.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UIRadioGroup::UIRadioGroup(UIControl * parent, const MetaControl& meta) noexcept
    : Super(parent, meta) {
}

/// <summary>
/// Sets the checked.
/// </summary>
/// <param name="radio">The radio.</param>
/// <returns></returns>
void LongUI::UIRadioGroup::set_checked(UIRadio * radio) noexcept {
    // 无视掉同样的
    if (m_pChecked == radio) return;
    // 取消前面的状态
    if (m_pChecked) m_pChecked->SetChecked(false);
    // 设置当前的状态
    if ((m_pChecked = radio)) m_pChecked->SetChecked(true);
    // 修改事件
    this->TriggrtEvent(_changed());
}

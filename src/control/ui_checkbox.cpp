// Gui
#include <core/ui_ctrlmeta.h>
#include <debugger/ui_debug.h>
#include <control/ui_checkbox.h>
// 子控件
#include <control/ui_boxlayout.h>
#include <control/ui_image.h>
#include <control/ui_label.h>
#include <constexpr/const_bkdr.h>
// Private
#include "../private/ui_private_control.h"

// ui namespace
namespace LongUI {
    // UICheckBox类 元信息
    LUI_CONTROL_META_INFO(UICheckBox, "checkbox");
    // UICheckBox私有信息
    struct UICheckBox::Private : CUIObject {
        // 构造函数
        Private(UICheckBox& btn) noexcept;
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
    UICheckBox::Private::Private(UICheckBox& btn) noexcept
        : image(&btn), label(&btn) {
        //UIControlPrivate::SetFocusable(image, false);
        //UIControlPrivate::SetFocusable(label, false);
#ifndef NDEBUG
        image.name_dbg = "checkbox::image";
        label.name_dbg = "checkbox::label";
        assert(image.IsFocusable() == false);
        assert(label.IsFocusable() == false);
        label.SetText(u"复选框");
#endif
    }
}

PCN_NOINLINE
/// <summary>
/// Sets the indeterminate.
/// </summary>
/// <returns></returns>
void LongUI::UICheckBox::SetIndeterminate() noexcept {
    // 禁用状态
    if (this->IsDisabled()) return;
    // 允许修改
    if (!this->GetIndeterminate()) {
        this->change_indeterminate(true);
        this->changed();
    }
}

PCN_NOINLINE
/// <summary>
/// Sets the checked.
/// </summary>
/// <param name="checked">if set to <c>true</c> [checked].</param>
/// <returns></returns>
void LongUI::UICheckBox::SetChecked(bool checked) noexcept {
    // 禁用状态
    if (this->IsDisabled()) return;
    bool changed_flag = false;
    // 是第三方状态?
    if (this->GetIndeterminate()) {
        this->change_indeterminate(false);
        changed_flag = true;
    }
    // 是相反状态?
    if (this->GetChecked() != checked) {
        changed_flag = true;
        const auto statetp = StyleStateType::Type_Checked;
        this->StartAnimation({ statetp , checked });
        m_private->image.StartAnimation({ statetp , checked });
    }
    // 修改状态
    if (changed_flag) this->changed();
}

/// <summary>
/// Sets the indeterminate.
/// </summary>
/// <param name="">if set to <c>true</c> [].</param>
/// <returns></returns>
void LongUI::UICheckBox::change_indeterminate(bool ndeterminate) noexcept {
    assert(this->GetIndeterminate() != ndeterminate);
    const auto statetp = StyleStateType::Type_Indeterminate;
    this->StartAnimation({ statetp , ndeterminate });
    m_private->image.StartAnimation({ statetp , ndeterminate });
}

/// <summary>
/// Changes this instance.
/// </summary>
/// <returns></returns>
void LongUI::UICheckBox::changed() noexcept {
    this->TriggerEvent(_stateChanged());
    // TODO: ACCESSIBLE
#ifndef LUI_ACCESSIBLE

#endif
}

/// <summary>
/// Initializes a new instance of the <see cref="UICheckBox" /> class.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UICheckBox::UICheckBox(UIControl* parent, const MetaControl& meta) noexcept 
    : Super(parent, meta) {
    // XXX: 硬编码
    m_oBox.margin = { 4, 2, 4, 2 };
    m_oBox.padding = { 4, 1, 2, 1 };
    m_state.focusable = true;
    // 原子性, 子控件为本控件的组成部分
    m_state.atomicity = true;
#ifdef LUI_ACCESSIBLE
    // 没有逻辑子控件
    m_pAccCtrl = nullptr;
#endif
    m_state.orient = Orient_Horizontal;
    m_oStyle.align = AttributeAlign::Align_Center;
    // 私有实现
    m_private = new(std::nothrow) Private{ *this };
    // OOM处理
    this->ctor_failed_if(m_private);
}


/// <summary>
/// Finalizes an instance of the <see cref="UICheckBox"/> class.
/// </summary>
/// <returns></returns>
LongUI::UICheckBox::~UICheckBox() noexcept {
    // 存在提前释放子控件, 需要标记"在析构中"
    m_state.destructing = true;
    // 释放私有数据
    if (m_private) delete m_private;
}



/// <summary>
/// Adds the attribute.
/// </summary>
/// <param name="key">The key.</param>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::UICheckBox::add_attribute(uint32_t key, U8View value) noexcept {
    // 新增属性列表
    constexpr auto BKDR_VALUE = 0x246df521_ui32;
    // 分类讨论
    switch (key)
    {
    case "label"_bkdr:
        // 传递给子控件
        UIControlPrivate::AddAttribute(m_private->label, BKDR_VALUE, value);
        break;
    default:
        // 其他情况, 交给基类处理
        return Super::add_attribute(key, value);
    }
}

/// <summary>
/// Does the event.
/// </summary>
/// <param name="sender">The sender.</param>
/// <param name="arg">The argument.</param>
/// <returns></returns>
auto LongUI::UICheckBox::DoEvent(
    UIControl * sender, const EventArg & arg) noexcept -> EventAccept {
    // 分类讨论
    switch (arg.nevent)
    {
    case NoticeEvent::Event_DoAccessAction:
        // 默认行动
        this->Toggle();
        return Event_Accept;
    case NoticeEvent::Event_Initialize:
        // 初始化
        this->init_checkbox();
        [[fallthrough]];
    default:
        // 基类处理
        return Super::DoEvent(sender, arg);
    }
}

/// <summary>
/// Initializes the checkbox.
/// </summary>
/// <returns></returns>
void LongUI::UICheckBox::init_checkbox() noexcept {
    if (!m_private) return;
    constexpr auto iapp = Appearance_CheckBox;
    UIControlPrivate::SetAppearanceIfNotSet(m_private->image, iapp);
}


/// <summary>
/// Does the mouse event.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UICheckBox::DoMouseEvent(const MouseEventArg & e) noexcept -> EventAccept {
    // 左键弹起 修改状态
    switch (e.type)
    {
    case LongUI::MouseEvent::Event_LButtonUp:
        this->Toggle();
        [[fallthrough]];
    default:
        return Super::DoMouseEvent(e);
    }
}

/// <summary>
/// Gets the text.
/// </summary>
/// <returns></returns>
auto LongUI::UICheckBox::GetText() const noexcept -> const char16_t* {
    assert(m_private && "bad action");
    return m_private->label.GetText();
}

/// <summary>
/// Gets the text string.
/// </summary>
/// <returns></returns>
auto LongUI::UICheckBox::GetTextString() const noexcept -> const CUIString& {
    assert(m_private && "bad action");
    return m_private->label.GetTextString();
}

/// <summary>
/// Sets the text.
/// </summary>
/// <param name="text">The text.</param>
/// <returns></returns>
void LongUI::UICheckBox::SetText(CUIString&& text) noexcept {
    assert(m_private && "bad action");
    m_private->label.SetText(std::move(text));
}

/// <summary>
/// Sets the text.
/// </summary>
/// <param name="text">The text.</param>
/// <returns></returns>
void LongUI::UICheckBox::SetText(const CUIString& text) noexcept {
    this->SetText(CUIString{ text });
}

/// <summary>
/// Sets the text.
/// </summary>
/// <param name="text">The text.</param>
/// <returns></returns>
void LongUI::UICheckBox::SetText(U16View text) noexcept {
    this->SetText(CUIString{ text });
}


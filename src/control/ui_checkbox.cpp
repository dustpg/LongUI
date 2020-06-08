// Gui
#include <core/ui_window.h>
#include <input/ui_kminput.h>
#include <core/ui_ctrlmeta.h>
#include <debugger/ui_debug.h>
#include <control/ui_checkbox.h>
// 子控件
#include <core/ui_unsafe.h>
#include <constexpr/const_bkdr.h>
#include <control/ui_box.h>
// Private
#include "../private/ui_private_control.h"

// ui namespace
namespace LongUI {
    // UICheckBox类 元信息
    LUI_CONTROL_META_INFO(UICheckBox, "checkbox");
    // UICheckBox私有实现
    struct UICheckBox::Private {
        // 设置新的文本
        template<typename T> static auto SetText(UICheckBox& cbox, T && text) noexcept {
            cbox.m_oLabel.SetText(std::forward<T>(text));
            // TODO: Accessible
        }
    };
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
    if (!this->IsIndeterminate()) {
        this->change_indeterminate(true);
        this->changed();
    }
}

/// <summary>
/// Sets the image source.
/// </summary>
/// <param name="src">The source.</param>
/// <returns></returns>
void LongUI::UICheckBox::SetImageSource(U8View src) noexcept {
    if (!m_pImageChild) {
        const auto img = new(std::nothrow) UIImage{ this };
        if (!img) return;
        m_pImageChild = img;
        m_bNewImage = true;
    }
    m_pImageChild->SetSource(src);
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
    if (this->IsIndeterminate()) {
        this->change_indeterminate(false);
        changed_flag = true;
    }
    // 是相反状态?
    if (!this->IsChecked() == checked) {
        changed_flag = true;
        const auto target = checked ? State_Checked : State_Non;
        this->StartAnimation({ State_Checked, target });
    }
    // 修改状态
    if (changed_flag) this->changed();
}

/// <summary>
/// Sets the indeterminate.
/// </summary>
/// <param name="ind">if set Indeterminat to <c>true</c> [].</param>
/// <returns></returns>
void LongUI::UICheckBox::change_indeterminate(bool ind) noexcept {
    assert(!this->IsIndeterminate() == ind);
    const auto statetp = State_Indeterminate;
    this->StartAnimation({ statetp , ind ? statetp : State_Non });
}

/// <summary>
/// Changes this instance.
/// </summary>
/// <returns></returns>
void LongUI::UICheckBox::changed() noexcept {
    this->TriggerEvent(this->_onCommand());
    // TODO: ACCESSIBLE
#ifndef LUI_ACCESSIBLE

#endif
}

/// <summary>
/// Initializes a new instance of the <see cref="UICheckBox" /> class.
/// </summary>
/// <param name="meta">The meta.</param>
LongUI::UICheckBox::UICheckBox(const MetaControl& meta) noexcept : Super(meta),
    m_oImage(this), m_oLabel(this) {
    // XXX: 硬编码
    m_oBox.margin = { 4, 2, 4, 2 };
    m_oBox.padding = { 4, 1, 2, 1 };
    m_state.tabstop = true;
    m_state.focusable = true;
    m_state.capturable = true;
    // 阻隔鼠标事件写入false之前需要写入
    m_oImage.RefInheritedMask() = State_MouseCutInher;
    m_oLabel.RefInheritedMask() = State_MouseCutInher;
    // 阻隔鼠标事件
    m_state.mouse_continue = false;
    this->make_offset_tf_direct(m_oLabel);
#ifdef LUI_ACCESSIBLE
    // 没有逻辑子控件
    m_pAccCtrl = nullptr;
#endif
    m_state.orient = Orient_Horizontal;
    m_oStyle.align = AttributeAlign::Align_Center;
    // 私有实现
    //UIControlPrivate::SetFocusable(image, false);
    //UIControlPrivate::SetFocusable(label, false);
#ifndef NDEBUG
    m_oImage.name_dbg = "checkbox::image";
    m_oLabel.name_dbg = "checkbox::label";
    assert(m_oLabel.IsFocusable() == false);
    assert(m_oImage.IsFocusable() == false);
    //label.SetText(u"复选框");
#endif
    // 设置连接控件
    m_oLabel.SetControl(*this);
    // 设置弱外貌
    UIControlPrivate::SetAppearance(*this, Appearance_WeakApp | Appearance_CheckBoxContainer);
    UIControlPrivate::SetAppearance(m_oImage, Appearance_WeakApp | Appearance_CheckBox);
}


/// <summary>
/// Finalizes an instance of the <see cref="UICheckBox"/> class.
/// </summary>
/// <returns></returns>
LongUI::UICheckBox::~UICheckBox() noexcept {
    // 存在提前释放子控件, 需要标记"在析构中"
    m_state.destructing = true;
}


/// <summary>
/// Adds the attribute.
/// </summary>
/// <param name="key">The key.</param>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::UICheckBox::add_attribute(uint32_t key, U8View value) noexcept {
    // 新增属性列表
    constexpr auto BKDR_SRC         = 0x001E57C4_ui32;
    constexpr auto BKDR_VALUE       = 0x246df521_ui32;
    constexpr auto BKDR_ACCESSKEY   = 0xba56ab7b_ui32;
    // 分类讨论
    switch (key)
    {
    case "label"_bkdr:
        // 传递给子控件
        Unsafe::AddAttrUninited(m_oLabel, BKDR_VALUE, value);
        break;
    case BKDR_ACCESSKEY:
        // 传递给子控件
        Unsafe::AddAttrUninited(m_oLabel, key, value);
        break;
    case BKDR_SRC:
        // src: 使用图片
        this->SetImageSource(value);
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
        this->SetAsDefaultAndFocus();
        this->Toggle();
        return Event_Accept;
    case NoticeEvent::Event_Initialize:
        // XXX: 初始化状态
        UIControlPrivate::RefStyleState(m_oImage) = m_oStyle.state;
        [[fallthrough]];
    default:
        // 基类处理
        return Super::DoEvent(sender, arg);
    }
}



#ifdef LUI_DRAW_FOCUS_RECT


/// <summary>
/// Triggers the event.
/// </summary>
/// <param name="event">The event.</param>
/// <returns></returns>
auto LongUI::UICheckBox::TriggerEvent(GuiEvent event) noexcept -> EventAccept {
    // 由于焦点位置特殊, 针对焦点的处理 
    EventAccept code = Event_Ignore;
    switch (event)
    {
    case LongUI::GuiEvent::Event_OnFocus:
        this->UpdateFocusRect();
        [[fallthrough]];
    case LongUI::GuiEvent::Event_OnBlur:
        this->Invalidate();
        break;
    }
    return Super::TriggerEvent(event);
}

/// <summary>
/// update the focus rect
/// </summary>
/// <returns></returns>
void LongUI::UICheckBox::UpdateFocusRect() const noexcept {
    // 复选框的焦点框在文本边上
    auto rect = m_oLabel.RefBox().GetBorderEdge();
    const auto pos = m_oLabel.GetPos();
    rect.left += pos.x;
    rect.top += pos.y;
    rect.right += pos.x;
    rect.bottom += pos.y;
    m_pWindow->UpdateFocusRect(rect);
}

#endif

/// <summary>
/// render this
/// </summary>
/// <returns></returns>
void LongUI::UICheckBox::Update(UpdateReason reason) noexcept {
    // 将文本消息传递给Label
    if (const auto r = reason & Reason_TextFontChanged)
        m_oLabel.Update(r);
    // 新创建
    if (m_bNewImage) {
        assert(m_pImageChild);
        UIControl::SwapChildren(*m_pImageChild, m_oLabel);
        m_bNewImage = false;
    }
#ifdef LUI_DRAW_FOCUS_RECT
    // 渲染焦点框
    if (this->m_oStyle.state & State_Focus) {
        // 成本较低就不用进一步判断
        assert(m_pWindow);
        this->UpdateFocusRect();
    }
#endif
    Super::Update(reason);
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
/// Does the input event.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UICheckBox::DoInputEvent(InputEventArg e) noexcept -> EventAccept {
    switch (e.event)
    {
    case InputEvent::Event_KeyUp:
        switch (static_cast<CUIInputKM::KB>(e.character))
        {
        case CUIInputKM::KB_SPACE:
            this->Toggle();
            break;
        }
    }
    return Super::DoInputEvent(e);
}

/// <summary>
/// Gets the text.
/// </summary>
/// <returns></returns>
auto LongUI::UICheckBox::GetText() const noexcept -> const char16_t* {
    return m_oLabel.GetText();
}

/// <summary>
/// Gets the text string.
/// </summary>
/// <returns></returns>
auto LongUI::UICheckBox::RefText() const noexcept -> const CUIString& {
    return m_oLabel.RefText();
}

/// <summary>
/// Sets the text.
/// </summary>
/// <param name="text">The text.</param>
/// <returns></returns>
void LongUI::UICheckBox::SetText(CUIString&& text) noexcept {
    Private::SetText(*this, std::move(text));
}

/// <summary>
/// Sets the text.
/// </summary>
/// <param name="text">The text.</param>
/// <returns></returns>
void LongUI::UICheckBox::SetText(const CUIString& text) noexcept {
    this->SetText(text.view());
}

/// <summary>
/// Sets the text.
/// </summary>
/// <param name="text">The text.</param>
/// <returns></returns>
void LongUI::UICheckBox::SetText(U16View text) noexcept {
    Private::SetText(*this, text);
}


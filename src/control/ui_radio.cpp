// Gui
#include <core/ui_unsafe.h>
#include <core/ui_window.h>
#include <core/ui_ctrlmeta.h>
#include <control/ui_radio.h>
#include <debugger/ui_debug.h>
#include <control/ui_radiogroup.h>
// 子控件
#include <control/ui_box.h>
// Private
#include "../private/ui_private_control.h"

// ui namespace
namespace LongUI {
    // UIRadio类 元信息
    LUI_CONTROL_META_INFO(UIRadio, "radio");
    // UIRadioGroup类 元信息
    LUI_CONTROL_META_INFO(UIRadioGroup, "radiogroup");
    // UIRadio类 私有实现
    struct UIRadio::Private {
        // 设置新的字符串
        template<typename T> static auto SetText(UIRadio& cbox, T && text) noexcept {
            cbox.m_oLabel.SetText(std::forward<T>(text));
#ifdef LUI_ACCESSIBLE
            // TODO: ACCESSIBLE
#endif
        }
    };
}

/// <summary>
/// Initializes a new instance of the <see cref="UIRadio" /> class.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UIRadio::UIRadio(const MetaControl& meta) noexcept : Super(meta),
    m_oImage(this), m_oLabel(this) {
    m_state.tabstop = true;
    m_state.focusable = true;
    m_state.capturable = true;
    // 阻隔鼠标事件写入false之前需要写入
    m_oImage.RefInheritedMask() = State_MouseCutInher;
    m_oLabel.RefInheritedMask() = State_MouseCutInher;
    // 阻隔鼠标事件
    m_state.mouse_continue = false;
    this->make_offset_tf_direct(m_oLabel);
    // 没有逻辑子控件
#ifdef LUI_ACCESSIBLE
    m_pAccCtrl = nullptr;
#endif
#ifndef NDEBUG
    m_oImage.name_dbg = "radio::image";
    m_oLabel.name_dbg = "radio::label";
    assert(m_oImage.IsFocusable() == false);
    assert(m_oLabel.IsFocusable() == false);
    //label.SetText(u"单选框");
#endif
    // 设置连接控件
    m_oLabel.SetControl(*this);
    // 设置弱外貌
    m_oStyle.appearance = Appearance_CheckBoxContainer;
    UIControlPrivate::SetAppearance(m_oImage, Appearance_Radio);
}


/// <summary>
/// Finalizes an instance of the <see cref="UIRadio"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIRadio::~UIRadio() noexcept {
    // 存在提前释放子控件, 需要标记"在析构中"
    m_state.destructing = true;
    // 移除UIRadioGroup弱引用
    if (m_pRadioGroup) m_pRadioGroup->RemoveRadio(*this);
}

/// <summary>
/// UIRadio: state changed
/// </summary>
/// <returns></returns>
void LongUI::UIRadio::Update(UpdateReason reason) noexcept {
    // 将文本消息传递给Label
    if (const auto r = reason & Reason_TextFontChanged)
        m_oLabel.Update(r);
    // 新创建
    if (m_bNewImage) {
        assert(m_pImageChild);
        UIControl::SwapChildren(*m_pImageChild, m_oLabel);
        m_bNewImage = false;
    }
    // 父节点修改了?
    if (reason & (Reason_ParentChanged | Reason_WindowChanged)) {
        // XXX: 父节点的父节点修改的话——潜在的野指针m_pRadioGroup
        m_pRadioGroup = nullptr;
        auto node = m_pParent;
        while (node) {
            if (const auto group = uisafe_cast<UIRadioGroup>(node)) {
                m_pRadioGroup = group;
                break;
            }
            node = node->GetParent();
        }
        //m_pRadioGroup = uisafe_cast<UIRadioGroup>(m_pParent);
    }
#ifdef LUI_DRAW_FOCUS_RECT
    // 渲染焦点框 XXX: IsFocus()
    if (this->m_oStyle.state & State_Focus) {
        // 成本较低就不用进一步判断
        assert(m_pWindow);
        this->UpdateFocusRect();
    }
#endif
    // 超类调用
    Super::Update(reason);
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
    switch (arg.nevent)
    {
    case NoticeEvent::Event_Initialize:
        this->init_radio();
        break;
    case NoticeEvent::Event_DoAccessAction:
        // 默认行动
        this->SetAsDefaultAndFocus();
        this->SetChecked(true);
        return Event_Accept;
    }
    return Super::DoEvent(sender, arg);
}

/// <summary>
/// Initializes the radio.
/// </summary>
/// <returns></returns>
void LongUI::UIRadio::init_radio() noexcept {
    // 同步checked-disable状态
    auto& target = UIControlPrivate::RefStyleState(m_oImage);
    target = target | (m_oStyle.state & (State_Checked | State_Disabled));
    // 在attr中设置了checked状态?
    if (m_oStyle.state & State_Checked) {
        if (const auto group = uisafe_cast<UIRadioGroup>(m_pParent)) {
            group->SetChecked(*this);
        }
    }
}




#ifdef LUI_DRAW_FOCUS_RECT
/// <summary>
/// Triggers the event.
/// </summary>
/// <param name="event">The event.</param>
/// <returns></returns>
auto LongUI::UIRadio::FireEvent(GuiEvent event) noexcept -> EventAccept {
    // 由于焦点位置特殊, 针对焦点的处理 
    switch (event)
    {
    case LongUI::GuiEvent::Event_OnFocus:
        this->UpdateFocusRect();
        [[fallthrough]];
    case LongUI::GuiEvent::Event_OnBlur:
        this->Invalidate();
        break;
    }
    return Super::FireEvent(event);
}

/// <summary>
/// update the focus rect
/// </summary>
/// <returns></returns>
void LongUI::UIRadio::UpdateFocusRect() const noexcept {
    // 单选框的焦点框在文本边上
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
/// Adds the attribute.
/// </summary>
/// <param name="key">The key.</param>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::UIRadio::add_attribute(uint32_t key, U8View value) noexcept {
    // 新增属性列表
    constexpr auto BKDR_SRC         = 0x001E57C4_ui32;
    constexpr auto BKDR_LABEL       = 0x74e22f74_ui32;
    constexpr auto BKDR_VALUE       = 0x246df521_ui32;
    constexpr auto BKDR_CHECKED     = 0x091a155f_ui32;
    constexpr auto BKDR_SELECTED    = 0x03481b1f_ui32;
    constexpr auto BKDR_ACCESSKEY   = 0xba56ab7b_ui32;
    switch (key)
    {
    case BKDR_SRC:
        // src: 使用图片
        this->SetImageSource(value);
        break;
    case BKDR_LABEL:
        // 传递给子控件
        Unsafe::AddAttrUninited(m_oLabel, BKDR_VALUE, value);
        break;
    case BKDR_ACCESSKEY:
        // 传递给子控件
        Unsafe::AddAttrUninited(m_oLabel, key, value);
        break;
    case BKDR_SELECTED:
        // selected:  兼容checked
        Super::add_attribute(BKDR_CHECKED, value);
        break;
    default:
        // 交个超类处理
        return Super::add_attribute(key, value);
    }
}

/// <summary>
/// Sets the seleced.
/// </summary>
/// <param name="checked">if set to <c>true</c> [checked].</param>
void LongUI::UIRadio::SetChecked(bool checked) noexcept {
    // 禁用状态
    if (this->IsDisabled()) return;
    if (!!this->IsChecked() == checked) return;
    // 修改状态
    const auto target = checked ? State_Checked : State_Non;
    this->StartAnimation({ State_Checked , target });
    // 检查回馈
    if (checked && m_pRadioGroup) {
        this->FireEvent(this->_onCommand());
        m_pRadioGroup->SetChecked(*this);
    }
}


/// <summary>
/// Sets the image source.
/// </summary>
/// <param name="src">The source.</param>
/// <returns></returns>
void LongUI::UIRadio::SetImageSource(U8View src) noexcept {
    if (!m_pImageChild) {
        const auto img = new(std::nothrow) UIImage{ this };
        if (!img) return;
        m_pImageChild = img;
        m_bNewImage = true;
    }
    m_pImageChild->SetSource(src);
}

/// <summary>
/// Sets the text.
/// </summary>
/// <param name="str">The string.</param>
/// <param name="len">The length.</param>
/// <returns></returns>
void LongUI::UIRadio::SetText(CUIString&& text) noexcept {
    Private::SetText(*this, std::move(text));
}

/// <summary>
/// Sets the text.
/// </summary>
/// <param name="text">The text.</param>
/// <returns></returns>
void LongUI::UIRadio::SetText(const CUIString& text) noexcept {
    return this->SetText(text.view());
}


/// <summary>
/// Sets the text.
/// </summary>
/// <param name="text">The text.</param>
/// <returns></returns>
void LongUI::UIRadio::SetText(U16View text) noexcept {
    Private::SetText(*this, text);
}

/// <summary>
/// Gets the text.
/// </summary>
/// <returns></returns>
auto LongUI::UIRadio::GetText() const noexcept -> const char16_t* {
    return m_oLabel.GetText();
}

/// <summary>
/// Gets the text string.
/// </summary>
/// <returns></returns>
auto LongUI::UIRadio::RefText() const noexcept -> const CUIString&{
    return m_oLabel.RefText();
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
void LongUI::UIRadioGroup::Update(UpdateReason reason) noexcept {
    // 清理checked标记
    auto clear_checked = [this]() noexcept {
        // 查找已有的
        for (const auto& x : (*this))
            if (&x == m_pChecked) return;
        // 没有找到
        m_pChecked = nullptr;
        // 修改事件?
        this->FireEvent(_onCommand());
    };
    // 子节点修改过?
    if (reason & Reason_ChildIndexChanged) {
        // 有才查找
        if (m_pChecked) clear_checked();
    }
    // 超类处理
    Super::Update(reason);
}

/// <summary>
/// Initializes a new instance of the <see cref="UIRadioGroup" /> class.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UIRadioGroup::UIRadioGroup(const MetaControl& meta) noexcept : Super(meta) {
    m_state.orient = Orient_Vertical;
}


/// <summary>
/// remove radio weak ref
/// 移除单选框弱引用
/// </summary>
/// <param name="radio"></param>
/// <returns></returns>
void LongUI::UIRadioGroup::RemoveRadio(UIRadio & radio) noexcept {
    if (&radio == m_pChecked) m_pChecked = nullptr; 
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
    this->FireEvent(this->_onCommand());
}

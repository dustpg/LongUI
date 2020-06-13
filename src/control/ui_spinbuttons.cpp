#include <control/ui_spinbuttons.h>
#include <core/ui_ctrlmeta.h>
#include <debugger/ui_debug.h>
// C++
#include <cassert>
// Private
#include "../private/ui_private_control.h"


// ui namespace
namespace LongUI {
    // UISpinButtons类 元信息
    LUI_CONTROL_META_INFO(UISpinButtons, "spinbuttons");
}


/// <summary>
/// Finalizes an instance of the <see cref="UISpinButtons"/> class.
/// </summary>
/// <returns></returns>
LongUI::UISpinButtons::~UISpinButtons() noexcept {
    m_state.destructing = true;
}


/// <summary>
/// Initializes a new instance of the <see cref="UISpinButtons" /> class.
/// </summary>
/// <param name="meta">The meta.</param>
LongUI::UISpinButtons::UISpinButtons(const MetaControl& meta) noexcept : 
    Super(meta), m_oDecreaseButton(this), m_oIncreaseButton(this) {
#ifdef LUI_ACCESSIBLE
    // 子控件为本控件的组成部分
    m_pAccCtrl = nullptr;
#endif
    // 私有实现
#ifndef NDEBUG
    m_oDecreaseButton.name_dbg = "spinbuttons::decrease";
    m_oIncreaseButton.name_dbg = "spinbuttons::increase";
#endif
    // 能够被拉伸
    UIControlPrivate::SetFlex(m_oDecreaseButton, 1.f);
    UIControlPrivate::SetFlex(m_oIncreaseButton, 1.f);
    // 能够被鼠标捕获
    UIControlPrivate::SetCapturable1(m_oDecreaseButton);
    UIControlPrivate::SetCapturable1(m_oIncreaseButton);
    // 设置弱外貌
    UIControlPrivate::SetAppearance(m_oDecreaseButton, Appearance_SpinnerDownButton);
    UIControlPrivate::SetAppearance(m_oIncreaseButton, Appearance_SpinnerUpButton);
}


/// <summary>
/// Does the mouse event.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UISpinButtons::DoMouseEvent(const MouseEventArg & e) noexcept -> EventAccept {
    switch (e.type)
    {
    case MouseEvent::Event_LButtonDown:
        // TODO: 持续按下
        // 上
        if (m_pHovered == &m_oDecreaseButton && m_oDecreaseButton.IsEnabled())
            this->TriggerEvent(_onDecrease());
        // 下
        else if (m_pHovered == &m_oIncreaseButton && m_oIncreaseButton.IsEnabled())
            this->TriggerEvent(_onIncrease());
        break;
    }
    return Super::DoMouseEvent(e);
}


#if 0

/// <summary>
/// Does the event.
/// </summary>
/// <param name="sender">The sender.</param>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UISpinButtons::DoEvent(UIControl* sender, 
    const EventArg& e) noexcept -> EventAccept {
    return Super::DoEvent(sender, e);
}

/// <summary>
/// Renders this instance.
/// </summary>
/// <returns></returns>
void LongUI::UISpinButtons::Render() const noexcept {
    return Super::Render();
}

/// <summary>
/// update this instance.
/// </summary>
/// <returns></returns>
void LongUI::UISpinButtons::Update(UpdateReason reason) noexcept {
    return Super::Update(reason);
}

/// <summary>
/// Recreates this instance.
/// </summary>
/// <param name="release_only">if set to <c>true</c> [release only].</param>
/// <returns></returns>
auto LongUI::UISpinButtons::Recreate(bool release_only) noexcept -> Result {
    return Super::Recreate(release_only);
}

/// <summary>
/// add child for this
/// </summary>
/// <param name="child"></param>
/// <returns></returns>
void LongUI::UISpinButtons::add_child(UIControl& child) noexcept {
    return Super::add_child(child);
}

#endif
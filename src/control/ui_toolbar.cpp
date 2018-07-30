// Gui
#include <core/ui_ctrlmeta.h>
#include <control/ui_toolbar.h>
#include <control/ui_toolbarbutton.h>
// Private
#include "../private/ui_private_control.h"


// ui namespace
namespace LongUI {
    // UIToolBar类 元信息
    LUI_CONTROL_META_INFO(UIToolBar, "toolbar");
    // UIToolBarButton类 元信息
    LUI_CONTROL_META_INFO(UIToolBarButton, "toolbarbutton");
}


// ----------------------------- UIToolBar





/// <summary>
/// Finalizes an instance of the <see cref="UIToolBar"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIToolBar::~UIToolBar() noexcept {
}

/// <summary>
/// Initializes a new instance of the <see cref="UIToolBar"/> class.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UIToolBar::UIToolBar(UIControl* parent, const MetaControl& meta) noexcept
    :Super(parent, meta) {
    // 水平布局
    m_state.orient = Orient_Horizontal;
    // 居中对齐
    m_oStyle.align = Align_Center;
}





// ----------------------------- UIToolBarButton




/// <summary>
/// Finalizes an instance of the <see cref="UIToolBarButton"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIToolBarButton::~UIToolBarButton() noexcept {

}

/// <summary>
/// Initializes a new instance of the <see cref="UIToolBarButton"/> class.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UIToolBarButton::UIToolBarButton(UIControl* parent, const MetaControl& meta) noexcept 
 : Super(parent, meta) {
    m_bToolBar = true;
    m_oBox.margin = { 0 };
    m_oBox.padding = { 2, 2, 2, 2 };
}



/// <summary>
/// Does the event.
/// </summary>
/// <param name="sender">The sender.</param>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UIToolBarButton::DoEvent(UIControl * sender,
    const EventArg & e) noexcept -> EventAccept {
    // 初始化
    switch (e.nevent)
    {
    case NoticeEvent::Event_Initialize:
        UIControlPrivate::SetAppearanceIfNotSet(*this, Appearance_ToolBarButton);
        //[[fallthrough]];
    }
    // 基类处理
    return Super::DoEvent(sender, e);
}
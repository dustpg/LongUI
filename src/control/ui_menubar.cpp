// Gui Core
#include <core/ui_window.h>
#include <core/ui_ctrlmeta.h>
//#include <debugger/ui_debug.h>
// Control
#include <control/ui_menu.h>
#include <control/ui_menubar.h>
#include <control/ui_menupopup.h>
// Private
#include "../private/ui_private_control.h"


// ui namespace
namespace LongUI {
    // UIMenuBar类 元信息
    LUI_CONTROL_META_INFO(UIMenuBar, "menubar");
    // UIMenu类 元信息
    LUI_CONTROL_META_INFO(UIMenu, "menu");
}


// ----------------------------- UIMenuBar


/// <summary>
/// Finalizes an instance of the <see cref="UIMenuBar"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIMenuBar::~UIMenuBar() noexcept {

}

/// <summary>
/// Initializes a new instance of the <see cref="UIMenuBar"/> class.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UIMenuBar::UIMenuBar(UIControl* parent, const MetaControl& meta) noexcept
    :Super(parent, meta) {
    // 水平布局
    m_state.orient = Orient_Horizontal;
    // 居中对齐
    m_oStyle.align = Align_Center;
}





// ----------------------------- UIMenu




/// <summary>
/// Finalizes an instance of the <see cref="UIMenu"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIMenu::~UIMenu() noexcept {
}

/// <summary>
/// Initializes a new instance of the <see cref="UIMenu"/> class.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UIMenu::UIMenu(UIControl* parent, const MetaControl& meta) noexcept
    : Super(parent, meta) {
    m_bMenuBar = true;
    m_type = UIButton::Type_Menu;
    m_oBox.margin = { 0 };
    m_oBox.padding = { 2, 2, 2, 2 };
}



/// <summary>
/// Does the event.
/// </summary>
/// <param name="sender">The sender.</param>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UIMenu::DoEvent(UIControl * sender,
    const EventArg & e) noexcept -> EventAccept {
    // 初始化
    switch (e.nevent)
    {
    case NoticeEvent::Event_Initialize:
        // 初始化
        UIControlPrivate::SetAppearanceIfNotSet(*this, Appearance_ToolBarButton);
        break;
    case NoticeEvent::Event_PopupBegin:
        // 弹出的是内建的菜单
        if (sender == m_pMenuPopup) {
            assert(m_pParent && "parent must be UIMenuBar");
            const auto bar = longui_cast<UIMenuBar*>(m_pParent);
            bar->SetNowMenu(*this);
            //LUIDebug(Hint) << "Menu On" << endl;
        }
        break;
    case NoticeEvent::Event_PopupEnd:
        // 关闭的是内建的菜单
        if (sender == m_pMenuPopup) {
            assert(m_pParent && "parent must be UIMenuBar");
            const auto bar = longui_cast<UIMenuBar*>(m_pParent);
            bar->ClearNowMenu();
            //LUIDebug(Hint) << "Menu Off" << endl;
        }
        break;
    }
    // 基类处理
    return Super::DoEvent(sender, e);
}

/// <summary>
/// Does the mouse event.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UIMenu::DoMouseEvent(const MouseEventArg& e) noexcept->EventAccept {
    switch (e.type)
    {
    case MouseEvent::Event_MouseEnter:
        // 鼠标移入而且菜单条处于激活模式
    {
        assert(m_pParent && "parent must be UIMenuBar");
        const auto bar = longui_cast<UIMenuBar*>(m_pParent);
        if (bar->HasNowMenu(*this)) this->Click();
        break;
    }
    }
    return Super::DoMouseEvent(e);
}

/// <summary>
/// Adds the child.
/// </summary>
/// <param name="child">The child.</param>
/// <returns></returns>
void LongUI::UIMenu::add_child(UIControl& child) noexcept {
    // UIMenuPopup
    if (const auto ptr = uisafe_cast<UIMenuPopup>(&child)) {
        assert(m_pMenuPopup == nullptr);
        m_pMenuPopup = ptr;
        return;
    }
    // 其他的
    return Super::add_child(child);
}

/// <summary>
/// Adds the attribute.
/// </summary>
/// <param name="key">The key.</param>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::UIMenu::add_attribute(uint32_t key, U8View value) noexcept {
    constexpr auto BKDR_TYPE = 0x0fab1332_ui32;
    // 无视TYPE
    switch (key)
    {
    case BKDR_TYPE: return;
    }
    return Super::add_attribute(key, value);
}

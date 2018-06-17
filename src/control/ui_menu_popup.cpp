// Gui
#include <debugger/ui_debug.h>

#include <control/ui_menupopup.h>
#include <control/ui_menuitem.h>
#include <control/ui_ctrlmeta.h>
// 子控件

// Private
#include "../private/ui_private_control.h"


// ui namespace
namespace LongUI {
    // UIMenuPopup类 元信息
    LUI_CONTROL_META_INFO(UIMenuPopup, "menupopup");
    // Menu Popup私有信息
//    struct PrivateMenuPopup : CUIObject {
//        // 构造函数
//        PrivateMenuPopup(UIMenuPopup& btn) noexcept;
//#ifndef NDEBUG
//        // 调试占位
//        void*               placeholder_debug1 = nullptr;
//#endif
//    };
}


/// <summary>
/// Finalizes an instance of the <see cref="UIMenuPopup"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIMenuPopup::~UIMenuPopup() noexcept {
}

/// <summary>
/// Initializes a new instance of the <see cref="UIMenuPopup" /> class.
/// </summary>
/// <param name="hoster">The hoster.</param>
/// <param name="meta">The meta.</param>
LongUI::UIMenuPopup::UIMenuPopup(UIControl* hoster, const MetaControl& meta) noexcept
    : Super(hoster, CUIWindow::Config_Popup, meta), m_pHoster(hoster) {
    // XXX: 默认是白色
    m_window.SetClearColor({ 1.f, 1.f, 1.f, 1.f });
}


/// <summary>
/// Updates this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIMenuPopup::Update() noexcept {
    if (m_pHovered && m_pPerSelected != m_pHovered) {
        this->change_select(m_pPerSelected, m_pHovered);
        m_pPerSelected = m_pHovered;
    }
    Super::Update();
}


/// <summary>
/// Does the event.
/// </summary>
/// <param name="sender">The sender.</param>
/// <param name="arg">The argument.</param>
/// <returns></returns>
auto LongUI::UIMenuPopup::DoEvent(
    UIControl* sender, const EventArg & arg) noexcept -> EventAccept {
    // 初始化
    switch (arg.nevent)
    {
    case NoticeEvent::Event_Initialize:
        // 初始化: 选择第一个
        this->select([this]() noexcept {
            const auto control = m_pSelected;
            m_pSelected = nullptr;
            return control;
        }());
        m_pPerSelected = m_pSelected;
        break;
    case NoticeEvent::Event_WindowClosed:
        // 不同的选择就归位
        if (m_pPerSelected != m_pSelected) {
            this->change_select(m_pPerSelected, m_pSelected);
            m_pPerSelected = m_pSelected;
        }
        // 提示Honster窗口关闭了
        if (m_pHoster) return m_pHoster->DoEvent(
            this, { NoticeEvent::Event_PopupClosed, 0 });
        break;
    case NoticeEvent::Event_UIEvent:
        // 自己不处理自己的UIEvent 否则就stackoverflow了
        if (sender == this) return Event_Accept;
        // UI 传递事件
        switch (static_cast<const EventGuiArg&>(arg).GetEvent())
        {
        case GuiEvent::Event_Select:
            this->select(sender);
            return Event_Accept;
        }
    }
    // 父类处理
    return Super::DoEvent(sender, arg);
}


/// <summary>
/// Adds the child.
/// </summary>
/// <param name="child">The child.</param>
/// <returns></returns>
void LongUI::UIMenuPopup::add_child(UIControl& child) noexcept {
    // 在未初始化的时候, 记录第一个UIMenuItem
    if (!this->is_inited() && !m_pSelected) {
        // 会在Event_Initialize进行选择
        if (const auto ptr = uisafe_cast<UIMenuItem>(&child)) {
            m_pSelected = ptr;
        }
    }
    return Super::add_child(child);
}

/// <summary>
/// Selects the specified child.
/// </summary>
/// <param name="child">The child.</param>
/// <returns></returns>
void LongUI::UIMenuPopup::select(UIControl* child) noexcept {
    if (child == m_pSelected) return;
    assert(child == nullptr || child->GetParent() == this);
    // 修改状态
    if (child != m_pPerSelected) 
        this->change_select(m_pSelected, child);
    // 修改数据
    m_pSelected = longui_cast<UIMenuItem*>(child);
    m_iSelected = child ? this->calculate_child_index(*child) : -1;
    // 事件触发
    this->TriggrtEvent(_selectedChanged());
    if (m_pHoster) m_pHoster->DoEvent(this, EventGuiArg{ _selectedChanged() });
}

/// <summary>
/// Changes the select.
/// </summary>
/// <param name="old">The old.</param>
/// <param name="now">The now.</param>
/// <returns></returns>
void LongUI::UIMenuPopup::change_select(UIControl* old, UIControl* now) noexcept {
    assert(old || now);
    if (old) old->StartAnimation({ StyleStateType::Type_Selected, false });
    if (now) now->StartAnimation({ StyleStateType::Type_Selected, true });
}

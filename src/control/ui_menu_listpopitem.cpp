// Gui
#include <core/ui_window.h>
#include <debugger/ui_debug.h>
#include <control/ui_ctrlmeta.h>
// Menu
#include <control/ui_menulist.h>
#include <control/ui_menuitem.h>
#include <control/ui_menupopup.h>
#include <core/ui_popup_window.h>
// 子控件
#include <control/ui_box_layout.h>
#include <control/ui_image.h>
#include <control/ui_label.h>
// Private
#include "../private/ui_private_control.h"

// ui namespace
namespace LongUI {
    // UIMenuList类 元信息
    LUI_CONTROL_META_INFO(UIMenuList, "menulist");
    // UIMenuItem类 元信息
    LUI_CONTROL_META_INFO(UIMenuItem, "menuitem");
    // UIMenuPopup类 元信息
    LUI_CONTROL_META_INFO(UIMenuPopup, "menupopup");
    // UIMenuItem私有信息
    struct UIMenuItem::Private : CUIObject {
        // 构造函数
        Private(UIMenuItem& btn) noexcept;
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
    UIMenuItem::Private::Private(UIMenuItem& btn) noexcept
        : image(&btn), label(&btn) {
        //UIControlPrivate::SetFocusable(image, false);
        //UIControlPrivate::SetFocusable(label, false);
#ifndef NDEBUG
        image.name_dbg = "menuitem::image";
        label.name_dbg = "menuitem::label";
        assert(image.IsFocusable() == false);
        assert(label.IsFocusable() == false);
#endif
    }
    // UIMenuList私有信息
    struct UIMenuList::Private : CUIObject {
        // 构造函数
        Private(UIMenuList& btn) noexcept;
#ifndef NDEBUG
        // 调试占位
        void*               placeholder_debug1 = nullptr;
#endif
        // 图像控件
        UIImage             image;
        // 标签控件
        UILabel             label;
        // 下拉标记
        UIImage             marker;
    };
    /// <summary>
    /// button privates data/method
    /// </summary>
    /// <param name="btn">The BTN.</param>
    /// <returns></returns>
    UIMenuList::Private::Private(UIMenuList& btn) noexcept
        : image(&btn), label(&btn), marker(&btn) {
        UIControlPrivate::SetFlex(label, 1.f);
        //UIControlPrivate::SetFocusable(image, false);
        //UIControlPrivate::SetFocusable(label, false);
        //UIControlPrivate::SetFocusable(marker, false);
#ifndef NDEBUG
        image.name_dbg = "menulist::image";
        label.name_dbg = "menulist::label";
        marker.name_dbg = "menulist::marker";
        assert(image.IsFocusable() == false);
        assert(label.IsFocusable() == false);
        assert(marker.IsFocusable() == false);
        label.SetText(L"Combo Box");
#endif
    }
    
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

// --------------------------- UIMenuItem  ---------------------

/// <summary>
/// Initializes a new instance of the <see cref="UIMenuItem" /> class.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UIMenuItem::UIMenuItem(UIControl* parent, const MetaControl& meta) noexcept 
    : Super(parent, meta) {
    m_state.focusable = true;
    this->SetOrient(Orient_Horizontal);
    m_oStyle.align = AttributeAlign::Align_Center;
    //m_oBox.margin = { 4, 2, 4, 2 };
    m_oBox.padding = { 4, 1, 2, 1 };
    // 原子控件
    m_state.atomicity = true;
    // 将事件传送给父节点
    UIControlPrivate::SetGuiEvent2Parent(*this);
    // 私有实现
    m_private = new(std::nothrow) Private{ *this };
    // OOM处理
    this->ctor_failed_if(m_private);
}


/// <summary>
/// Finalizes an instance of the <see cref="UIMenuItem"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIMenuItem::~UIMenuItem() noexcept {
    // 存在提前释放子控件, 需要标记"在析构中"
    m_state.destructing = true;
    // 释放私有数据
    if (m_private) delete m_private;
}

/// <summary>
/// Gets the text.
/// </summary>
/// <returns></returns>
auto LongUI::UIMenuItem::GetText() const noexcept -> const wchar_t * {
    return m_private->label.GetText();
}

/// <summary>
/// Gets the text string.
/// </summary>
/// <returns></returns>
auto LongUI::UIMenuItem::GetTextString() const noexcept -> const CUIString &{
    return m_private->label.GetTextString();
}


/// <summary>
/// Does the event.
/// </summary>
/// <param name="sender">The sender.</param>
/// <param name="arg">The argument.</param>
/// <returns></returns>
auto LongUI::UIMenuItem::DoEvent(
    UIControl * sender, const EventArg & arg) noexcept -> EventAccept {
    // 初始化

    switch (arg.nevent)
    {
    case  NoticeEvent::Event_Initialize:
        this->init_menuitem();
        break;
#ifndef NDEBUG
    case NoticeEvent::Event_RefreshBoxMinSize:
        return Super::DoEvent(sender, arg);
#endif
    }
    return Super::DoEvent(sender, arg);
}


/// <summary>
/// Adds the attribute.
/// </summary>
/// <param name="key">The key.</param>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::UIMenuItem::add_attribute(uint32_t key, U8View value) noexcept {
    constexpr auto BKDR_LABEL = 0x74e22f74_ui32;
    constexpr auto BKDR_VALUE = 0x246df521_ui32;
    switch (key)
    {
    case BKDR_LABEL:
        // 传递给子控件
        UIControlPrivate::AddAttribute(m_private->label, BKDR_VALUE, value);
        break;
    }
    return Super::add_attribute(key, value);
}



/// <summary>
/// Renders this instance.
/// </summary>
/// <returns></returns>
//void LongUI::UIMenuItem::Render() const noexcept {
//    Super::Render();
//}

/// <summary>
/// Initializes the menuitem.
/// </summary>
/// <returns></returns>
void LongUI::UIMenuItem::init_menuitem() noexcept {
    UIControlPrivate::SetAppearanceIfNotSet(*this, Appearance_MenuItem);
    if (!m_private) return;
    //constexpr auto iapp = Appearance_CheckBox;
    //UIControlPrivate::SetAppearanceIfNotSet(m_private->image, iapp);
    // 标签数据
    //auto& label = m_private->label;
    //const auto a = label.GetText();
}


/// <summary>
/// Does the mouse event.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UIMenuItem::DoMouseEvent(const MouseEventArg & e) noexcept -> EventAccept {
    // 左键弹起 修改状态
    switch (e.type)
    {
    /*case LongUI::MouseEvent::Event_MouseEnter:

        this->StartAnimation({ statetp , checked });
        break;
    case LongUI::MouseEvent::Event_MouseLeave*/
    case LongUI::MouseEvent::Event_LButtonUp:
        {
            const auto statetp = StyleStateType::Type_Checked;
            const auto checked = !this->GetStyle().state.checked;
            this->StartAnimation({ statetp , checked });
            m_private->image.StartAnimation({ statetp , checked });
        }
        // 事件
        this->TriggrtEvent(_selected());
        // 关闭弹出窗口
        if (const auto wnd = this->GetWindow()) {
            if (wnd->config & CUIWindow::Config_Popup) {
                assert(wnd->GetParent());
                wnd->GetParent()->ClosePopup();
            }
        }
    }
    return Super::DoMouseEvent(e);
}







// --------------------------- UIMenuList  ---------------------








/// <summary>
/// Gets the text.
/// </summary>
/// <returns></returns>
auto LongUI::UIMenuList::GetText() const noexcept -> const wchar_t* {
    return m_private->label.GetText();
}

/// <summary>
/// Gets the text string.
/// </summary>
/// <returns></returns>
auto LongUI::UIMenuList::GetTextString() const noexcept -> const CUIString&{
    return m_private->label.GetTextString();
}


/// <summary>
/// Initializes a new instance of the <see cref="UIMenuList" /> class.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UIMenuList::UIMenuList(UIControl* parent, const MetaControl& meta) noexcept
    : Super(parent, meta) {
    m_state.focusable = true;
    m_state.defaultable = true;
    // 原子性, 子控件为本控件的组成部分
    m_state.atomicity = true;
    // 默认是处于关闭状态
    m_oStyle.state.closed = true;
    // 布局方向
    m_oStyle.align = Align_Center;
    // 垂直布局
    this->SetOrient(Orient_Horizontal);
    m_oBox.margin = { 5, 5, 5, 5 };
    m_oBox.padding.right = 5;
    // 私有实现
    m_private = new(std::nothrow) Private{ *this };
    // OOM处理
    this->ctor_failed_if(m_private);
}


/// <summary>
/// Finalizes an instance of the <see cref="UIMenuList"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIMenuList::~UIMenuList() noexcept {
    // 存在提前释放子控件, 需要标记"在析构中"
    m_state.destructing = true;
    // XXX: 窗口处于析构状态时无需释放窗口
    if (m_pWindow && m_pWindow->IsInDtor()) {

    }
    else {
        // 释放
        if (m_pMenuPopup) delete m_pMenuPopup;
    }
    // 释放私有数据
    if (m_private) delete m_private;
}

/// <summary>
/// Does the mouse event.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UIMenuList::DoMouseEvent(const MouseEventArg& e) noexcept -> EventAccept {
    // 处理消息
    switch (e.type)
    {
    case LongUI::MouseEvent::Event_LButtonDown:
        //m_pWindow->SetCapture(*this);
        this->ShowPopup();
        [[fallthrough]];
    default:
        return Super::DoMouseEvent(e);
    }
}


/// <summary>
/// Initializes the menulist.
/// </summary>
void LongUI::UIMenuList::init_menulist() {
    UIControlPrivate::SetAppearanceIfNotSet(*this, Appearance_Button);
    auto& marker = m_private->marker;
    UIControlPrivate::SetAppearanceIfNotSet(marker, Appearance_DropDownMarker);
}

/// <summary>
/// Does the event.
/// </summary>
/// <param name="sender">The sender.</param>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UIMenuList::DoEvent(UIControl * sender,
    const EventArg & e) noexcept -> EventAccept {
    switch (e.nevent)
    {
    case NoticeEvent::Event_Initialize:
        // 初始化
        this->init_menulist();
        return Event_Accept;
#ifndef NDEBUG
    case NoticeEvent::Event_RefreshBoxMinSize:
        return Super::DoEvent(sender, e);
#endif
    case NoticeEvent::Event_PopupBegin:
    case NoticeEvent::Event_PopupEnd:
        // 关闭了弹出窗口
        //if (sender == m_pMenuPopup) {
        //    int bk = 9;
        //}
        return Event_Accept;
    case NoticeEvent::Event_UIEvent:
        // UI 传递事件
        switch (static_cast<const EventGuiArg&>(e).GetEvent())
        {
        case UIMenuPopup::_selectedChanged():
            assert(sender == m_pMenuPopup);
            this->on_selected_changed();
            return Event_Accept;
        }
        [[fallthrough]];
    default:
        // 基类处理
        return Super::DoEvent(sender, e);
    }
}

/// <summary>
/// Ons the selected changed.
/// </summary>
/// <returns></returns>
void LongUI::UIMenuList::on_selected_changed() noexcept {
    assert(m_pMenuPopup);
    // 修改选择数据
    m_iSelected = m_pMenuPopup->GetSelectedIndex();
    const auto ctrl = m_pMenuPopup->GetSelected();
    // 修改事件
    if (ctrl) this->SetText(ctrl->GetTextString());
    else this->SetText(CUIString{});
    // 触发事件
    this->TriggrtEvent(_selectedChanged());
}

#ifdef LUI_ACCESSIBLE
#include <accessible/ui_accessible_callback.h>
#include <accessible/ui_accessible_event.h>
#include <accessible/ui_accessible_type.h>
#include <core/ui_string.h>
#endif


/// <summary>
/// Sets the text.
/// </summary>
/// <param name="text">The text.</param>
/// <returns></returns>
void LongUI::UIMenuList::SetText(CUIString&& text) noexcept {
    assert(m_private && "bad action");
    if (m_private->label.SetText(std::move(text))) {
#ifdef LUI_ACCESSIBLE
        LongUI::Accessible(m_pAccessible, Callback_PropertyChanged);
#endif
    }
}

/// <summary>
/// Sets the text.
/// </summary>
/// <param name="text">The text.</param>
/// <returns></returns>
void LongUI::UIMenuList::SetText(WcView text) noexcept {
    this->SetText(CUIString{ text });
}

/// <summary>
/// Sets the text.
/// </summary>
/// <param name="text">The text.</param>
/// <returns></returns>
void LongUI::UIMenuList::SetText(const CUIString& text) noexcept {
    this->SetText(CUIString{ text });
}

/// <summary>
/// Adds the child.
/// </summary>
/// <param name="child">The child.</param>
/// <returns></returns>
void LongUI::UIMenuList::add_child(UIControl& child) noexcept {
    // 检查是不是 Menu Popup
    if (const auto ptr = uisafe_cast<UIMenuPopup>(&child)) {
        m_pMenuPopup = ptr;
        return;
    }
    return Super::add_child(child);
}

/// <summary>
/// Shows the popup.
/// </summary>
/// <returns></returns>
void LongUI::UIMenuList::ShowPopup() noexcept {
    // 有窗口?
    if (m_pMenuPopup) {
        // 出现在左下角
        const auto edge = this->GetBox().GetBorderEdge();
        const auto y = this->GetSize().height - edge.top;
        const auto pos = this->MapToWindowEx({ edge.left, y });
        LongUI::PopupWindowFromViewport(
            *this,
            *m_pMenuPopup,
            pos,
            PopupType::Type_Exclusive
        );
    }

    // 触发修改GUI事件
    //this->TriggrtEvent(_clicked());
#ifdef LUI_ACCESSIBLE
    // TODO: 调用 accessible 接口
#endif
}

#ifdef LUI_ACCESSIBLE


#endif








// -------------------------- UIMenuPopup  ---------------------








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
    : Super(*hoster, CUIWindow::Config_Popup, meta) {
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
    UIControl* sender, const EventArg& arg) noexcept -> EventAccept {
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

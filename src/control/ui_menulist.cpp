// Gui
#include <core/ui_window.h>
#include <core/ui_manager.h>
#include <core/ui_ctrlmeta.h>
#include <debugger/ui_debug.h>
#include <event/ui_group_event.h>
// Menu
#include <control/ui_menu.h>
#include <control/ui_menulist.h>
#include <control/ui_menuitem.h>
#include <control/ui_menupopup.h>
#include <core/ui_popup_window.h>
// 子控件
#include <control/ui_image.h>
#include <control/ui_label.h>
#include <control/ui_boxlayout.h>
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
        const_cast<RectF&>(image.GetBox().margin) = { 2.f, 2.f, 2.f, 2.f };
        this->image.JustSetAsIcon();
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
        label.SetText(u"Combo Box");
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
    m_state.orient = Orient_Horizontal;
    m_oStyle.align = AttributeAlign::Align_Stretcht;
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
/// Sets the text.
/// </summary>
/// <param name="str">The string.</param>
/// <returns></returns>
void LongUI::UIMenuItem::SetText(CUIString&& str) noexcept {
    m_private->label.SetText(std::move(str));
}

/// <summary>
/// Sets the text.
/// </summary>
/// <param name="view">The view.</param>
/// <returns></returns>
void LongUI::UIMenuItem::SetText(U16View view) noexcept {
    m_private->label.SetText(view);
}

/// <summary>
/// Gets the text.
/// </summary>
/// <returns></returns>
auto LongUI::UIMenuItem::GetText() const noexcept -> const char16_t * {
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
    using group_t = const ImplicitGroupGuiArg;
    // 初始化
    switch (arg.nevent)
    {
    case  NoticeEvent::Event_Initialize:
        this->init_menuitem();
        break;
    case NoticeEvent::Event_ImplicitGroupChecked:
        // 组有位成员被点中
        if (sender == this) return Event_Ignore;
        if (this->IsDisabled()) return Event_Ignore;
        if (!this->IsChecked()) return Event_Ignore;
        if (m_type != UIMenuItem::Type_Radio) return Event_Ignore;
        if (m_pName != static_cast<group_t&>(arg).group_name)
            return Event_Ignore;
        // 是CHECKBOX类型?
        this->SetChecked(false);
#ifdef LUI_ACCESSIBLE
        // TODO: ACCESSIBLE
#endif
        break;
#ifndef NDEBUG
    case NoticeEvent::Event_RefreshBoxMinSize:
        return Super::DoEvent(sender, arg);
#endif
    }
    return Super::DoEvent(sender, arg);
}

/// <summary>
/// Sets the checked.
/// </summary>
/// <param name="checked">if set to <c>true</c> [checked].</param>
/// <returns></returns>
void LongUI::UIMenuItem::SetChecked(bool checked) noexcept {
    //if (this->IsChecked() == checked) return;

    const auto statetp = StyleStateType::Type_Checked;
    this->StartAnimation({ statetp , checked });
    m_private->image.StartAnimation({ statetp , checked });
}

/// <summary>
/// Adds the attribute.
/// </summary>
/// <param name="key">The key.</param>
/// <param name="value">The value.</param>
/// <returns></returns>
void LongUI::UIMenuItem::add_attribute(uint32_t key, U8View value) noexcept {
    constexpr auto BKDR_SRC         = 0x001E57C4_ui32;
    constexpr auto BKDR_NAME        = 0x0ed6f72f_ui32;
    constexpr auto BKDR_TYPE        = 0x0fab1332_ui32;
    constexpr auto BKDR_LABEL       = 0x74e22f74_ui32;
    constexpr auto BKDR_VALUE       = 0x246df521_ui32;
    constexpr auto BKDR_SELECTED    = 0x03481b1f_ui32;
    switch (key)
    {
    case BKDR_LABEL:
        // 传递给子控件
        UIControlPrivate::AddAttribute(m_private->label, BKDR_VALUE, value);
        break;
    case BKDR_SRC:
        // src  : 图片名
        UIControlPrivate::AddAttribute(m_private->image, BKDR_SRC, value);
        break;
    case BKDR_NAME:
        // name :  组名
        m_pName = UIManager.GetUniqueText(value);
        break;
    case BKDR_TYPE:
        // type : 类型
        m_type = this->view2type(value);
        break;
    case BKDR_SELECTED:
        // selected: 选择
        if (const auto obj = uisafe_cast<UIMenuPopup>(m_pParent)) {
            obj->select(this);
            obj->m_pPerSelected = this;
        }
        break;
    default:
        // 其他的交给父类处理
        return Super::add_attribute(key, value);
    }
}


/// <summary>
/// View2types the specified view.
/// </summary>
/// <param name="view">The view.</param>
/// <returns></returns>
auto LongUI::UIMenuItem::view2type(U8View view)noexcept->ItemType {
    switch (*view.begin())
    {
    default: return UIMenuItem::Type_Normal;
    case 'c': return UIMenuItem::Type_CheckBox;
    case 'r': return UIMenuItem::Type_Radio;
    }
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
    // 父节点必须是UIMenuPopup
    if (const auto ptr = longui_cast<UIMenuPopup*>(m_pParent)) {
        // 不是COMBOBOX
        if (ptr->HasPaddingForItem()) {
            switch (m_type)
            {
            case LongUI::UIMenuItem::Type_CheckBox:
                UIControlPrivate::SetAppearanceIfNotSet(m_private->image, Appearance_MenuCheckBox);
                UIControlPrivate::RefStyleState(m_private->image).checked = m_oStyle.state.checked;
                break;
            case LongUI::UIMenuItem::Type_Radio:
                UIControlPrivate::SetAppearanceIfNotSet(m_private->image, Appearance_MenuRadio);
                UIControlPrivate::RefStyleState(m_private->image).checked = m_oStyle.state.checked;
                break;
            }
            // XXX: 标准化
            m_oBox.padding.right = float(ICON_WIDTH);
            m_private->image.SetStyleMinSize({ float(ICON_WIDTH), 0.f });
        }
    }


    //constexpr auto iapp = Appearance_CheckBox;
    //UIControlPrivate::SetAppearanceIfNotSet(m_private->image, iapp);
    // 标签数据
    //auto& label = m_private->label;
    //const auto a = label.GetText();
}


/// <summary>
/// Does the checkbox.
/// </summary>
/// <returns></returns>
void LongUI::UIMenuItem::do_checkbox() noexcept {
    const auto statetp = StyleStateType::Type_Checked;
    const auto checked = !this->GetStyle().state.checked;
    this->StartAnimation({ statetp , checked });
    m_private->image.StartAnimation({ statetp , checked });
}

/// <summary>
/// Does the radio.
/// </summary>
/// <returns></returns>
void LongUI::UIMenuItem::do_radio() noexcept {
    if (m_oStyle.state.checked) return;
    this->SetChecked(true);
    LongUI::DoImplicitGroupGuiArg(*this, m_pName);
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
    //case LongUI::MouseEvent::Event_MouseLeave:
    //case LongUI::MouseEvent::Event_MouseIdleHover:
    //    m_pWindow->ClosePopup();
    //    break;
    case LongUI::MouseEvent::Event_LButtonUp:
        // 复选框
        if (m_type == UIMenuItem::Type_CheckBox) {
            this->do_checkbox();
        }
        // 单选框
        else if (m_type == UIMenuItem::Type_Radio) {
            this->do_radio();
        }
        // 事件
#ifdef NDEBUG
        this->TriggerEvent(_selected());
#endif
        m_pWindow->ClosePopupHighLevel();
    }
    return Super::DoMouseEvent(e);
}


// --------------------------- UIMenuList  ---------------------





/// <summary>
/// Gets the text.
/// </summary>
/// <returns></returns>
auto LongUI::UIMenuList::GetText() const noexcept -> const char16_t* {
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
    m_state.orient = Orient_Horizontal;
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
    // 没有选择的?
    if (m_pMenuPopup && !m_pMenuPopup->GetLastSelected()) {
        m_pMenuPopup->SelectFirstItem();
    }
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
#ifdef NDEBUG
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
#endif
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
    const auto ctrl = m_pMenuPopup->GetLastSelected();
    // 修改事件
    if (ctrl) this->SetText(ctrl->GetTextString());
    else this->SetText(CUIString{});
    // 触发事件
#ifdef NDEBUG
    this->TriggerEvent(_selectedChanged());
#endif
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
void LongUI::UIMenuList::SetText(U16View text) noexcept {
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
        // HACK: ptr目前可能只是UIControl, 但是现在修改的正是UIControl
        ptr->save_selected_true();
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
    if (m_pMenuPopup && m_pMenuPopup->GetCount()) {
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
    //this->TriggerEvent(_clicked());
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
    : Super(*hoster, CUIWindow::Config_FixedSize | CUIWindow::Config_Popup, meta) {
    // 保存选择的认为是组合框
    if (this->is_save_selected())
        this->init_clear_color_for_default_combobox();
    // 否则认为是一般菜单
    else
        this->init_clear_color_for_default_ctxmenu();
}

/// <summary>
/// Initializes the clear color for default ctxmenu.
/// </summary>
/// <returns></returns>
void LongUI::UIMenuPopup::init_clear_color_for_default_ctxmenu() noexcept {
    constexpr float single = float(0xf0) / float(0xff);
    ColorF color;
    color.r = single; color.g = single;
    color.b = single; color.a = 1.f;
    m_window.SetClearColor(color);
}

/// <summary>
/// Initializes the clear color for default combobox.
/// </summary>
/// <returns></returns>
void LongUI::UIMenuPopup::init_clear_color_for_default_combobox() noexcept {
    constexpr float single = float(0xff) / float(0xff);
    ColorF color;
    color.r = single; color.g = single;
    color.b = single; color.a = 1.f;
    m_window.SetClearColor(color);
}

/// <summary>
/// Updates this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIMenuPopup::Update() noexcept {
    // 检测是否
    if (m_pHovered || m_bMouseIn) {
        m_bMouseIn = true;
        if (m_pPerSelected != m_pHovered) {
            this->change_select(m_pPerSelected, m_pHovered);
            m_pPerSelected = m_pHovered;
            // 没有m_pDelayClosed? 尝试关闭
            //if (!m_pDelayClosed) SetDelayClosedPopup();
        }
    }
    // 父类更新
    Super::Update();
}


/// <summary>
/// Windows the closed.
/// </summary>
/// <returns></returns>
void LongUI::UIMenuPopup::WindowClosed() noexcept {
    m_bMouseIn = false;
    // 保存
    if (this->is_save_selected()) {
        // 不同的选择就归位
        if (m_pPerSelected != m_pLastSelected) {
            this->change_select(m_pPerSelected, m_pLastSelected);
            m_pPerSelected = m_pLastSelected;
        }
    }
    // 不保存
    else {
        this->ClearSelected();
    }
    return Super::WindowClosed();
}


/// <summary>
/// Does the mouse event.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UIMenuPopup::DoMouseEvent(const MouseEventArg&e)noexcept->EventAccept{
    switch (e.type)
    {
    case MouseEvent::Event_MouseEnter:
        assert(m_pHoster && "hoster must be valid on mouse event");
        if (const auto menu = uisafe_cast<UIMenu>(m_pHoster)) {
            // 嵌套?
            if (const auto mp = uisafe_cast<UIMenuPopup>(menu->GetParent())) {
                mp->MarkNoDelayClosedPopup();
#if 0
                mp->m_bMouseIn = false;
                mp->m_pPerSelected = menu;
                constexpr auto c = StyleStateType::Type_Selected;
                if (!menu->GetStyle().state.selected)
                    menu->StartAnimation({ c, true });
#endif
            }
        }
        // 先检查自己有没有?
        if (m_window.GetNowPopup()) this->SetDelayClosedPopup();
    }
    return Super::DoMouseEvent(e);
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
    //case NoticeEvent::Event_Initialize:
    //    // 没有匹配的
    //    break;
    case NoticeEvent::Event_UIEvent:
        // 自己不处理自己的UIEvent 否则就stackoverflow了
        if (sender == this) return Event_Accept;
        // UI 传递事件
#ifdef NDEBUG
        switch (static_cast<const EventGuiArg&>(arg).GetEvent())
        {
        case GuiEvent::Event_Select:
            this->select(sender);
            return Event_Accept;
        }
#endif
        [[fallthrough]];
    }
    // 父类处理
    return Super::DoEvent(sender, arg);
}

/// <summary>
/// Adds the item.
/// </summary>
/// <param name="label">The label.</param>
/// <returns></returns>
void LongUI::UIMenuPopup::AddItem(CUIString&& label) noexcept {
    if (const auto item = new(std::nothrow) UIMenuItem{ this }) {
        item->SetText(std::move(label));
    }
}


/// <summary>
/// Selects the first item.
/// </summary>
/// <returns></returns>
void LongUI::UIMenuPopup::SelectFirstItem() noexcept {
    for (auto& x : (*this)) {
        if (const auto ptr = uisafe_cast<UIMenuItem>(&x)) {
            this->select(ptr);
            m_pPerSelected = m_pLastSelected;
            return;
        }
    }
}

/// <summary>
/// Clears the selected.
/// </summary>
/// <returns></returns>
void LongUI::UIMenuPopup::ClearSelected() noexcept {
    //if (m_pLastSelected) this->select(nullptr);
    //m_pLastSelected = nullptr;

    if (m_pPerSelected) {
        m_pPerSelected->StartAnimation({ StyleStateType::Type_Selected, false });
    }
    m_pLastSelected = nullptr;
    m_pPerSelected = nullptr;
    m_iSelected = -1;
}


/// <summary>
/// Subs the viewport popup begin.
/// </summary>
/// <param name="vp">The vp.</param>
/// <param name="type">The type.</param>
/// <returns></returns>
void LongUI::UIMenuPopup::SubViewportPopupBegin(UIViewport& vp, PopupType type) noexcept {
    if (m_pDelayClosed) {
        m_pDelayClosed->Terminate();
        m_pDelayClosed = nullptr;
    }
}

/// <summary>
/// Terminates the delay closed popup.
/// </summary>
/// <returns></returns>
void LongUI::UIMenuPopup::MarkNoDelayClosedPopup() noexcept {
    // FIXME: 新窗口WM_MOUSELEAVE WM_MOUSEMOVE前后可能互换
    // 处理KnownIssues#3:
    //  - 先LEAVE: 终止延迟关闭
    //  - 先ENTER, 标记m_bNoClosedOnce

    if (m_pDelayClosed) {
        m_pDelayClosed->Terminate();
        m_pDelayClosed = nullptr;
        m_bNoClosedOnce = false;
    }
    else m_bNoClosedOnce = true;

//#ifndef NDEBUG
//    else {
//        LUIDebug(Warning) << "m_pDelayClosed => null" << endl;
//    }
//#endif // !NDEBUG

    //m_bNoClosedOnce = true;
}

#ifndef NDEBUG
void longui_ui_menulist_1(void* ptr) noexcept {
    LUIDebug(Hint) << ptr << LongUI::endl;
}
#endif

/// <summary>
/// Sets the delay closed popup.
/// </summary>
/// <returns></returns>
void LongUI::UIMenuPopup::SetDelayClosedPopup() noexcept {
    // 一次不关闭
    if (m_bNoClosedOnce) {
        m_bNoClosedOnce = false;
        return;
    }

    // 已经有了?
    if (m_pDelayClosed) {

    }
    else {
        const auto window = &m_window;
        auto& capsule = m_pDelayClosed;
        capsule = UIManager.CreateTimeCapsule([window, &capsule](float t) noexcept {
            if (t < 1.f) return;
            capsule = nullptr;
            //::longui_ui_menulist_1(window);
            window->ClosePopup();
        }, 1.f, this);
    }
}

/// <summary>
/// Adds the child.
/// </summary>
/// <param name="child">The child.</param>
/// <returns></returns>
//void LongUI::UIMenuPopup::add_child(UIControl& child) noexcept {
//    // 在未初始化的时候, 记录第一个UIMenuItem
//    if (!this->is_inited() && !m_pLastSelected) {
//        // 会在Event_Initialize进行选择
//        if (const auto ptr = uisafe_cast<UIMenuItem>(&child)) {
//            m_pLastSelected = ptr;
//        }
//    }
//    return Super::add_child(child);
//}

/// <summary>
/// Selects the specified child.
/// </summary>
/// <param name="child">The child.</param>
/// <returns></returns>
void LongUI::UIMenuPopup::select(UIControl* child) noexcept {
    if (child == m_pLastSelected) return;
    assert(child == nullptr || child->GetParent() == this);
    // 修改状态
    if (child != m_pPerSelected) 
        this->change_select(m_pLastSelected, child);
    // 修改数据
    m_pLastSelected = longui_cast<UIMenuItem*>(child);
    m_iSelected = child ? this->calculate_child_index(*child) : -1;
    // 事件触发
#ifdef NDEBUG
    this->TriggerEvent(_selectedChanged());
    if (m_pHoster) m_pHoster->DoEvent(this, EventGuiArg{ _selectedChanged() });
#endif
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

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
#include <control/ui_boxlayout.h>
// Private
#include <core/ui_unsafe.h>
#include "../private/ui_private_control.h"

// ui namespace
namespace LongUI {
    // UIMenuList类 元信息
    LUI_CONTROL_META_INFO(UIMenuList, "menulist");
    // UIMenuItem类 元信息
    LUI_CONTROL_META_INFO(UIMenuItem, "menuitem");
    // UIMenuPopup类 元信息
    LUI_CONTROL_META_INFO(UIMenuPopup, "menupopup");
}

// --------------------------- UIMenuItem  ---------------------

/// <summary>
/// Initializes a new instance of the <see cref="UIMenuItem" /> class.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UIMenuItem::UIMenuItem(UIControl* parent, const MetaControl& meta) noexcept 
    : Super(impl::ctor_lock(parent), meta),
    m_oImage(this), m_oLabel(this) {
    m_state.focusable = true;
    m_state.orient = Orient_Horizontal;
    m_oStyle.align = AttributeAlign::Align_Stretcht;
    //m_oBox.margin = { 4, 2, 4, 2 };
    m_oBox.padding = { 4, 1, 2, 1 };
    // 阻隔鼠标事件
    m_state.mouse_continue = false;
    this->make_offset_tf_direct(m_oLabel);
    // 将事件传送给父节点
    UIControlPrivate::SetGuiEvent2Parent(*this);
    // 私有实现
    //UIControlPrivate::SetFocusable(image, false);
    //UIControlPrivate::SetFocusable(label, false);
    const_cast<RectF&>(m_oImage.RefBox().margin) = { 2.f, 2.f, 2.f, 2.f };
    m_oImage.JustSetAsIcon();
#ifndef NDEBUG
    m_oImage.name_dbg = "menuitem::image";
    m_oLabel.name_dbg = "menuitem::label";
    assert(m_oImage.IsFocusable() == false);
    assert(m_oLabel.IsFocusable() == false);
#endif
    // 构造锁
    impl::ctor_unlock();
}


/// <summary>
/// Finalizes an instance of the <see cref="UIMenuItem"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIMenuItem::~UIMenuItem() noexcept {
    // 存在提前释放子控件, 需要标记"在析构中"
    m_state.destructing = true;
}

/// <summary>
/// Sets the text.
/// </summary>
/// <param name="str">The string.</param>
/// <returns></returns>
void LongUI::UIMenuItem::SetText(CUIString&& str) noexcept {
    m_oLabel.SetText(std::move(str));
}

/// <summary>
/// Sets the text.
/// </summary>
/// <param name="view">The view.</param>
/// <returns></returns>
void LongUI::UIMenuItem::SetText(U16View view) noexcept {
    m_oLabel.SetText(view);
}

/// <summary>
/// Gets the text.
/// </summary>
/// <returns></returns>
auto LongUI::UIMenuItem::GetText() const noexcept -> const char16_t * {
    return m_oLabel.GetText();
}

/// <summary>
/// Gets the text string.
/// </summary>
/// <returns></returns>
auto LongUI::UIMenuItem::RefText() const noexcept -> const CUIString &{
    return m_oLabel.RefText();
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
    const auto target = checked ? State_Checked : State_Non;
    this->StartAnimation({ State_Checked , target });
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
        Unsafe::AddAttrUninited(m_oLabel, BKDR_VALUE, value);
        break;
    case BKDR_SRC:
        // src  : 图片名
        Unsafe::AddAttrUninited(m_oImage, BKDR_SRC, value);
        break;
    case BKDR_NAME:
        // name :  组名
        m_pName = UIManager.GetUniqueText(value).id;
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
        // 其他的交给超类处理
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
    // 父节点必须是UIMenuPopup
    if (const auto ptr = longui_cast<UIMenuPopup*>(m_pParent)) {
        // 不是COMBOBOX
        if (ptr->HasPaddingForItem()) {
            const auto init_state = [this]() noexcept {
                auto& state = UIControlPrivate::RefStyleState(m_oImage);
                state = state | (m_oStyle.state & State_Checked);
            };
            switch (m_type)
            {
            case LongUI::UIMenuItem::Type_CheckBox:
                UIControlPrivate::SetAppearanceIfNotSet(m_oImage, Appearance_MenuCheckBox);
                init_state();
                break;
            case LongUI::UIMenuItem::Type_Radio:
                UIControlPrivate::SetAppearanceIfNotSet(m_oImage, Appearance_MenuRadio);
                init_state();
                break;
            }
            // XXX: 标准化
            m_oBox.padding.right = float(ICON_WIDTH);
            m_oImage.SetStyleMinSize({ float(ICON_WIDTH), 0.f });
        }
    }

    //constexpr auto iapp = Appearance_CheckBox;
    //UIControlPrivate::SetAppearanceIfNotSet(m_oImage, iapp);
    // 标签数据
    //auto& label = m_oLabel;
    //const auto a = label.GetText();
}


/// <summary>
/// Does the checkbox.
/// </summary>
/// <returns></returns>
void LongUI::UIMenuItem::do_checkbox() noexcept {
    const auto target = ~m_oStyle.state & State_Checked;
    this->StartAnimation({ State_Checked, target });
}

/// <summary>
/// Does the radio.
/// </summary>
/// <returns></returns>
void LongUI::UIMenuItem::do_radio() noexcept {
    if (m_oStyle.state & State_Checked) return;
    this->SetChecked(true);
    LongUI::DoImplicitGroupGuiArg(*this, m_pName);
}


/// <summary>
/// update with reason
/// </summary>
/// <param name="reason"></param>
/// <returns></returns>
void LongUI::UIMenuItem::Update(UpdateReason reason) noexcept {
    // 将文本消息传递给Label
    if (const auto r = reason & Reason_TextFontChanged)
        m_oLabel.Update(r);
    return Super::Update(reason);
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
        if (m_type == UIMenuItem::Type_CheckBox)
            this->do_checkbox();
        // 单选框
        else if (m_type == UIMenuItem::Type_Radio)
            this->do_radio();
        // 事件
        this->TriggerEvent(this->_onCommand());
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
    return m_oLabel.GetText();
}

/// <summary>
/// Gets the text string.
/// </summary>
/// <returns></returns>
auto LongUI::UIMenuList::RefText() const noexcept -> const CUIString&{
    return m_oLabel.RefText();
}


/// <summary>
/// Initializes a new instance of the <see cref="UIMenuList" /> class.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UIMenuList::UIMenuList(UIControl* parent, const MetaControl& meta) noexcept
    : Super(impl::ctor_lock(parent), meta),
    m_oImage(this), m_oLabel(this), m_oMarker(this) {
    m_state.focusable = true;
    m_state.defaultable = true;
    // 阻隔鼠标事件
    m_state.mouse_continue = false;
    this->make_offset_tf_direct(m_oLabel);
    // 默认是处于关闭状态
    m_oStyle.state = m_oStyle.state | State_Closed;
    // 布局方向
    m_oStyle.align = Align_Center;
    // 垂直布局
    m_state.orient = Orient_Horizontal;
    m_oBox.margin = { 5, 5, 5, 5 };
    m_oBox.padding.right = 5;
    // 私有实现
    UIControlPrivate::SetFlex(m_oLabel, 1.f);
    //UIControlPrivate::SetFocusable(image, false);
    //UIControlPrivate::SetFocusable(label, false);
    //UIControlPrivate::SetFocusable(marker, false);
#ifndef NDEBUG
    m_oImage.name_dbg = "menulist::image";
    m_oLabel.name_dbg = "menulist::label";
    m_oMarker.name_dbg = "menulist::marker";
    assert(m_oImage.IsFocusable() == false);
    assert(m_oLabel.IsFocusable() == false);
    assert(m_oMarker.IsFocusable() == false);
    //label.SetText(u"Combo Box");
#endif
    // 构造锁
    impl::ctor_unlock();
}


/// <summary>
/// Finalizes an instance of the <see cref="UIMenuList"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIMenuList::~UIMenuList() noexcept {
    // 存在提前释放子控件, 需要标记"在析构中"
    m_state.destructing = true;
    // 如果没有初始化就释放

    // XXX: 窗口处于析构状态时无需释放窗口
    if (this->is_inited() && m_pWindow && m_pWindow->IsInDtor()) {

    }
    else {
        // 释放
        if (m_pMenuPopup) delete m_pMenuPopup;
    }
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
    auto& marker = m_oMarker;
    UIControlPrivate::SetAppearanceIfNotSet(*this, Appearance_Button);
    UIControlPrivate::SetAppearanceIfNotSet(marker, Appearance_DropDownMarker);
    // 没有选择的?
    if (m_pMenuPopup && !m_pMenuPopup->GetLastSelected()) {
        m_pMenuPopup->SelectFirstItem();
    }
}


/// <summary>
/// update with reason
/// </summary>
/// <param name="reason"></param>
/// <returns></returns>
void LongUI::UIMenuList::Update(UpdateReason reason) noexcept {
    // 将文本消息传递给Label
    if (const auto r = reason & Reason_TextFontChanged)
        m_oLabel.Update(r);
    return Super::Update(reason);
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
        case UIMenuPopup::_onCommand():
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
    const auto ctrl = m_pMenuPopup->GetLastSelected();
    // 修改事件
    if (ctrl) this->SetText(ctrl->RefText());
    else this->SetText(CUIString{});
    // 触发事件
    this->TriggerEvent(this->_onCommand());
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
    if (m_oLabel.SetText(std::move(text))) {
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
    this->SetText(CUIString(text));
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
        ptr->save_selected_true();
        m_pMenuPopup = ptr;
        // 仅仅是弱引用, 直接返回
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
    if (m_pMenuPopup && m_pMenuPopup->GetChildrenCount()) {
        // 出现在左下角
        const auto edge = this->RefBox().GetBorderEdge();
        const auto y = this->GetSize().height - edge.top;
        const auto pos = this->MapToWindowEx({ edge.left, y });
        LongUI::PopupWindowFromViewport(
            *this,
            *m_pMenuPopup,
            pos,
            PopupType::Type_Exclusive,
            m_pMenuPopup->GetPopupPosition()
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
    : Super(*impl::ctor_lock(hoster), CUIWindow::Config_FixedSize | CUIWindow::Config_Popup, meta) {
    // XXX: 由NativeStyle提供
    // 保存选择的认为是组合框
    if (this->is_save_selected())
        this->init_clear_color_for_default_combobox();
    // 否则认为是一般菜单
    else
        this->init_clear_color_for_default_ctxmenu();
    // 构造锁
    impl::ctor_unlock();
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
void LongUI::UIMenuPopup::Update(UpdateReason reason) noexcept {
    // 悬浮控件修改的情况
    if (reason & Reason_HoveredChanged) {
        if (m_pHovered || m_bMouseIn) {
            m_bMouseIn = true;
            if (m_pPerSelected != m_pHovered) {
                this->change_select(m_pPerSelected, m_pHovered);
                m_pPerSelected = m_pHovered;
                // 没有m_pDelayClosed? 尝试关闭
                //if (!m_pDelayClosed) SetDelayClosedPopup();
            }
        }
    }
    // 超类更新
    Super::Update(reason);
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
            //LUIDebug(Hint)
            //    << "m_pPerSelected: " << (void*)m_pPerSelected
            //    << "m_pLastSelected: " << (void*)m_pLastSelected
            //    << endl;
            this->change_select(m_pPerSelected, m_pLastSelected);
            m_pPerSelected = m_pLastSelected;
        }
    }
    // 不保存
    else this->ClearSelected();
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
                if (!menu->RefStyle().state.selected)
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
        switch (static_cast<const EventGuiArg&>(arg).GetEvent())
        {
        case UIMenuItem::_onCommand():
            assert(sender->GetParent() == this);
            this->select(sender);
            return Event_Accept;
        }
        [[fallthrough]];
    }
    // 超类处理
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
    if (const auto per = m_pPerSelected)
        per->StartAnimation({ State_Selected, State_Non });
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
    this->TriggerEvent(this->_onCommand());
    if (m_pHoster) m_pHoster->DoEvent(this, EventGuiArg{ _onCommand() });
}

#ifndef NDEBUG
extern "C" void ui_debug_output_info(char*) noexcept;
#endif // !NDEBUG

/// <summary>
/// add attribute for <seealso cref="UIMenuPopup"/>
/// </summary>
/// <param name="key">key</param>
/// <param name="view">view</param>
/// <returns></returns>
void LongUI::UIMenuPopup::add_attribute(uint32_t key, U8View view) noexcept {
    constexpr auto BKDR_ID          = 0x0000361f_ui32;
    constexpr auto BKDR_POSITION    = 0xd52a25f1_ui32;
    switch (key)
    {
    case BKDR_POSITION:
        m_posPopup = AttrParser::PopupPosition(view);
        return;
#ifndef NDEBUG
    case BKDR_ID:
    {
        char buf[128];
        std::snprintf(
            buf, 128, "0x%p - %.*s\r\n", 
            this, 
            view.end()- view.begin(), 
            view.begin()
        );
        ::ui_debug_output_info(buf);
    }
        return Super::add_attribute(key, view);
#endif // !NDEBUG

    }
    return Super::add_attribute(key,view);
}

/// <summary>
/// Changes the select.
/// </summary>
/// <param name="old">The old.</param>
/// <param name="now">The now.</param>
/// <returns></returns>
void LongUI::UIMenuPopup::change_select(UIControl* old, UIControl* now) noexcept {
    assert(old || now);
    if (old) old->StartAnimation({ State_Selected, State_Non });
    if (now) now->StartAnimation({ State_Selected, State_Selected });
}

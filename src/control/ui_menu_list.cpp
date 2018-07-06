// Gui
#include <util/ui_ctordtor.h>
#include <debugger/ui_debug.h>
#include <control/ui_menulist.h>
#include <control/ui_menuitem.h>
#include <control/ui_ctrlmeta.h>
#include <control/ui_menupopup.h>
// 子控件
#include <control/ui_box_layout.h>
#include <control/ui_image.h>
#include <control/ui_label.h>
// Private
#include "../private/ui_private_control.h"

#include <algorithm>

// ui namespace
namespace LongUI {
    // UIMenuList类 元信息
    LUI_CONTROL_META_INFO(UIMenuList, "menulist");
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
        marker.name_dbg= "menulist::marker";
        assert(image.IsFocusable() == false);
        assert(label.IsFocusable() == false);
        assert(marker.IsFocusable() == false);
        label.SetText(L"Combo Box");
#endif
    }
}

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
    // XXX: 无需(?)释放弹出菜单
    //if (m_pMenuPopup) delete m_pMenuPopup;
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
    case NoticeEvent::Event_PopupClosed:
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
        ptr->init_hoster(this);
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
        auto& window = m_pMenuPopup->RefWindow();
        const auto this_window = this->GetWindow();
        assert(this_window);
        // 计算大小
        auto& popup = m_pMenuPopup->RefWindow();
        // 边框
        auto size = this->GetBox().GetBorderSize();
        // 高度
        size.height = m_pMenuPopup->GetMinSize().height;
        // TODO: DPI缩放

        // 姿势
        const int32_t w = static_cast<int32_t>(size.width);
        const int32_t h = static_cast<int32_t>(size.height);
        popup.Resize({ w, h });

        const auto edge = this->GetBox().GetBorderEdge();
        const auto y = this->GetSize().height - edge.top;
        this_window->PopupWindow(window, this->MapToWindowEx({ edge.left, y }));
    }

    // 触发修改GUI事件
    //this->TriggrtEvent(_clicked());
#ifdef LUI_ACCESSIBLE
    // TODO: 调用 accessible 接口
#endif
}

#ifdef LUI_ACCESSIBLE


#endif

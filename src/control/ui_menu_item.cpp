// Gui
#include <core/ui_window.h>
#include <debugger/ui_debug.h>
#include <control/ui_menuitem.h>
#include <control/ui_ctrlmeta.h>
// 子控件
#include <control/ui_box_layout.h>
#include <control/ui_image.h>
#include <control/ui_label.h>
// Private
#include "../private/ui_private_control.h"

// ui namespace
namespace LongUI {
    // UIMenuItem类 元信息
    LUI_CONTROL_META_INFO(UIMenuItem, "menuitem");
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
        CUIString text;
        text.append(L'框');
        text.append('A' + std::rand() % 10);
        label.SetText(text);
#endif
    }
}

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
    if (arg.nevent == NoticeEvent::Event_Initialize) {
        this->init_menuitem();
    }
    return Super::DoEvent(sender, arg);
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

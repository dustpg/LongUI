#include <core/ui_ctrlmeta.h>
#include <control/ui_richlistbox.h>
#include <control/ui_richlistitem.h>
#include <debugger/ui_debug.h>
#include <core/ui_color_list.h>
#include <constexpr/const_bkdr.h>


// ui namespace
namespace LongUI {
    // UIRichListBox类 元信息
    LUI_CONTROL_META_INFO(UIRichListBox, "richlistbox");
    // UIRichListItem类 元信息
    LUI_CONTROL_META_INFO(UIRichListItem, "richlistitem");
}



/// <summary>
/// Initializes a new instance of the <see cref="UIRichListBox"/> class.
/// </summary>
/// <param name="meta">The meta.</param>
LongUI::UIRichListBox::UIRichListBox(const MetaControl& meta) noexcept : Super(meta) {
    m_state.orient = Orient_Vertical;
}

/// <summary>
/// Finalizes an instance of the <see cref="UIRichListBox"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIRichListBox::~UIRichListBox() noexcept {
}


/// <summary>
/// Items the detached.
/// </summary>
/// <param name="item">The item.</param>
/// <returns></returns>
void LongUI::UIRichListBox::ItemDetached(UIRichListItem& item) noexcept {
    if (m_pSelectedItem == &item) {
        m_pSelectedItem = nullptr;
    }
}

/// <summary>
/// Adds the child.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
void LongUI::UIRichListBox::add_child(UIControl& ctrl) noexcept {
    // 是?
    if (const auto item = uisafe_cast<UIRichListItem>(&ctrl)) {
        if (!m_pSelectedItem && !this->is_inited()) {
            m_pSelectedItem = item;
        }
    }
    return Super::add_child(ctrl);
}


/// <summary>
/// Selects the item.
/// </summary>
/// <param name="item">The item.</param>
/// <returns></returns>
void LongUI::UIRichListBox::SelectItem(UIRichListItem* item) noexcept {
    assert(item || item->GetParent() == this);
    if (item == m_pSelectedItem) return;
    const auto prev_item = m_pSelectedItem;
    m_pSelectedItem = item;
    if (prev_item) prev_item->StartAnimation({ State_Selected ,State_Non });
    if (item) item->StartAnimation({ State_Selected ,State_Selected });
}



/// <summary>
/// Does the event.
/// </summary>
/// <param name="sender">The sender.</param>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UIRichListBox::DoEvent(
    UIControl* sender,  const EventArg& e) noexcept -> EventAccept {
    switch (e.nevent)
    {
    case NoticeEvent::Event_Initialize:
        if (m_pSelectedItem) {
            const auto item = m_pSelectedItem;
            m_pSelectedItem = nullptr;
            this->SelectItem(item);
        }
        break;
    }
    return Super::DoEvent(sender, e);
}






/// <summary>
/// Initializes a new instance of the <see cref="UIRichListItem"/> class.
/// </summary>
/// <param name="meta">The meta.</param>
LongUI::UIRichListItem::UIRichListItem(const MetaControl& meta) noexcept : Super(meta) {
    m_state.orient = Orient_Horizontal;
    m_oStyle.align = Align_Stretcht;
}


/// <summary>
/// Finalizes an instance of the <see cref="UIRichListItem"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIRichListItem::~UIRichListItem() noexcept {
#ifndef NDEBUG
    if (m_pParent)
#endif // !NDEBUG
    if (const auto box = longui_cast<UIRichListBox*>(m_pParent)) {
        box->ItemDetached(*this);
    }
}

/// <summary>
/// Does the mouse event.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UIRichListItem::DoMouseEvent(const MouseEventArg& e) noexcept -> EventAccept {
    switch (e.type)
    {
    case MouseEvent::Event_LButtonDown:
        assert(m_pParent);
        longui_cast<UIRichListBox*>(m_pParent)->SelectItem(this);
        break;
    }
    return Super::DoMouseEvent(e);
}

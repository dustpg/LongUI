#include <control/ui_deck.h>
#include <control/ui_stack.h>
#include <core/ui_ctrlmeta.h>
#include <debugger/ui_debug.h>

#include <algorithm>
#include <cassert>

// ui namespace
namespace LongUI {
    // UIDeck类 元信息
    LUI_CONTROL_META_INFO(UIDeck, "deck");
    // UIStack类 元信息
    LUI_CONTROL_META_INFO(UIStack, "stack");
    // impl namespace
    namespace impl {
        // find greatest child-minsize/ right-buttom
        auto greatest_minsize_rb(UIControl& ctrl) noexcept {
            Size2F size = { 0.f, 0.f };
            for (auto& child : ctrl) {
                const auto pos = child.GetPos();
                const auto thism = child.GetMinSize();
                const Size2F rb{ pos.x + thism.width, pos.y + thism.height };
                size.width = std::max(size.width, rb.width);
                size.height = std::max(size.height, rb.height);
            }
            return size;
        }
    }
}

// ----------------------------------------------------------------------------
// --------------------              Deck             -------------------------
// ----------------------------------------------------------------------------


/// <summary>
/// Finalizes an instance of the <see cref="UIDeck"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIDeck::~UIDeck() noexcept {
}


/// <summary>
/// Initializes a new instance of the <see cref="UIDeck"/> class.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UIDeck::UIDeck(UIControl* parent, const MetaControl& meta) noexcept
    : Super(impl::ctor_lock(parent), meta) {
    // 构造锁
    impl::ctor_unlock();
}

/// <summary>
/// Updates this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIDeck::Update() noexcept {
    // 有变数
    if (m_state.child_i_changed) {
        uint32_t index = 0;
        for (auto& child : (*this)) {
            child.SetVisible(index == m_index);
            ++index;
        }
    }
    // 父类处理
    return Super::Update();
}

/// <summary>
/// Sets the index of the selected.
/// </summary>
/// <param name="index">The index.</param>
/// <returns></returns>
void LongUI::UIDeck::SetSelectedIndex(uint32_t index) noexcept {
    if (index >= this->GetCount()) return;
    if (m_index != index) {
        const auto newitem = this->calculate_child_at(index);
        const auto olditem = this->calculate_child_at(m_index);
        assert(newitem && "404");
        if (olditem) olditem->SetVisible(false);
        newitem->SetVisible(true);
        m_index = index;
    }
}

/// <summary>
/// Ons the index changed.
/// </summary>
/// <returns></returns>
void LongUI::UIDeck::on_index_changed() noexcept {
    // TODO: index changed event;
    //this->TriggerEvent(_selectedChanged());
}

// ----------------------------------------------------------------------------
// --------------------             Stack             -------------------------
// ----------------------------------------------------------------------------


/// <summary>
/// Finalizes an instance of the <see cref="UIStack"/> class.
/// </summary>
/// <returns></returns>
LongUI::UIStack::~UIStack() noexcept {
}


/// <summary>
/// Initializes a new instance of the <see cref="UIStack"/> class.
/// </summary>
/// <param name="parent">The parent.</param>
/// <param name="meta">The meta.</param>
LongUI::UIStack::UIStack(UIControl* parent, const MetaControl& meta) noexcept
    : Super(parent, meta) {

}

/// <summary>
/// Adds the child.
/// </summary>
/// <param name="child">The child.</param>
/// <returns></returns>
//void LongUI::UIStack::add_child(UIControl& child) noexcept {
//    this->set_child_parent_nnrivc(child);
//    return Super::add_child(child);
//}


/// <summary>
/// Updates this instance.
/// </summary>
/// <returns></returns>
void LongUI::UIStack::Update() noexcept {
    // 要求重新布局
    if (this->is_need_relayout()) {
        // 不脏了
        m_state.dirty = false;
        // 重新布局
        this->relayout_stack();
    }
    // 其他的交给父类处理
    Super::Update();
}


/// <summary>
/// Relayouts the stack.
/// </summary>
/// <returns></returns>
void LongUI::UIStack::relayout_stack() noexcept {
    for (auto& child : (*this)) {
        const auto size = child.GetMinSize();
        this->resize_child(child, size);
    }
}


/// <summary>
/// Does the event.
/// </summary>
/// <param name="sender">The sender.</param>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::UIStack::DoEvent(UIControl* sender,
    const EventArg& e) noexcept -> EventAccept {
    switch (e.nevent)
    {
    case NoticeEvent::Event_RefreshBoxMinSize:
        // 更新最小大小
        this->set_contect_minsize(impl::greatest_minsize_rb(*this));
        return Event_Accept;
    default:
        // 其他事件
        return Super::DoEvent(sender, e);
    }
}

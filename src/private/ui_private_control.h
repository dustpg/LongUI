#pragma once

// ui
#include <control/ui_control.h>

// c++
#include <cassert>


/// <summary>
/// private function/data to UIControl
/// </summary>
struct LongUI::UIControlPrivate {
    // need relayout
    static auto IsNeedRelayout(UIControl& ctrl) noexcept { return ctrl.m_state.reason; }
#ifndef LUI_DISABLE_STYLE_SUPPORT
    // ensure bg renderer
    static auto EnsureBgRenderer(UIControl& ctrl) noexcept->CUIRendererBackground*;
    // get bg renderer
    static auto GetBgRenderer(const UIControl& ctrl) noexcept->CUIRendererBackground*;
    // ensure bd renderer
    static auto EnsureBdRenderer(UIControl& ctrl) noexcept->CUIRendererBorder*;
    // get bd renderer
    static auto GetBdRenderer(const UIControl& ctrl) noexcept->CUIRendererBorder*;
#endif
    // refresh min size
    static void RefreshMinSize(UIControl& ctrl) noexcept;
    // update world
    static void UpdateWorld(UIControl& ctrl) noexcept;
    // update world force
    static void UpdateWorldForce(UIControl& ctrl) noexcept;
    // update world - toplevel
    static void UpdateWorldTop(UIControl& ctrl, Size2L) noexcept;
    // Synchronous Init Data
    static void SyncInitData(UIControl& ctrl) noexcept;
    // do mouse enter event
    static auto DoMouseEnter(UIControl& ctrl, const Point2F& pos) noexcept->EventAccept;
    // do mouse leave event
    static auto DoMouseLeave(UIControl& ctrl, const Point2F& pos) noexcept->EventAccept;
    // ----------- INLINE ZONE ----------------
    // ref last end time capsule
    static auto&RefLastEnd(UIControl& ctrl) noexcept { return ctrl.m_pLastEnd; }
    // set focusable
    static void SetFocusable(UIControl& ctrl, bool f) noexcept { ctrl.m_state.focusable = f; }
    // set appearance
    static void SetAppearance(UIControl& ctrl, AttributeAppearance a) noexcept { ctrl.m_oStyle.appearance = a; }
    // set appearance if not set
    static void SetAppearanceIfNotSet(UIControl& ctrl, AttributeAppearance a) noexcept {
        auto& appearance = ctrl.m_oStyle.appearance;
        if (appearance == Appearance_NotSet) ctrl.m_oStyle.appearance = a; 
    }
    // set flex
    static void SetFlex(UIControl& ctrl, float flex) noexcept { ctrl.m_oStyle.flex = flex; }
    // set box minwidth
    static void SetBoxMinWidth(UIControl& ctrl, float minw) noexcept { ctrl.m_oBox.minsize.width = minw; }
    // set orient
    static void SetOrient(UIControl& ctrl, bool o) noexcept { ctrl.m_state.orient = o; }
    // is need rerelayout
    //static auto IsNeedRelayout(const UIControl& ctrl) noexcept { return ctrl.is_need_relayout(); }
    // is mouse_continue
    static auto IsMouseCon(const UIControl& ctrl) noexcept { return ctrl.m_state.mouse_continue; }
    // is dirty
    static auto IsInDirty(const UIControl& ctrl) noexcept { return ctrl.is_in_dirty_list(); }
    // dirty = true
    static auto MarkInDirty(UIControl& ctrl) noexcept { return ctrl.m_state.in_dirty_list = true; }
    // dirty = false
    static auto ClearInDirty(UIControl& ctrl) noexcept { return ctrl.m_state.in_dirty_list = false; }
    // get parent data
    static auto GetParentData(const UIControl& ctrl) noexcept { return ctrl.m_uData4Parent; }
    // ref style state
    static auto&RefStyleState(UIControl& ctrl) noexcept { return ctrl.m_oStyle.state; }
#ifndef LUI_DISABLE_STYLE_SUPPORT
    // ref style matched value
    static auto&RefStyleMatched(UIControl& ctrl) noexcept { return ctrl.m_oStyle.matched; }
    // ref style trigger
    static auto&RefStyleTrigger(UIControl& ctrl) noexcept { return ctrl.m_oStyle.trigger; }
#endif
    // set parent data
    static auto SetParentData(UIControl& ctrl, uint32_t data) noexcept { return ctrl.m_uData4Parent = data; }
    // set gui event to parent
    static void SetGuiEvent2Parent(UIControl& ctrl) noexcept { ctrl.m_state.gui_event_to_parent = true; }
    // prev control
    static auto&Prev(UIControl& ctrl) noexcept { return *ctrl.prev; }
    // next control
    static auto&Next(UIControl& ctrl) noexcept  { return *ctrl.next; }
    // next control
    static auto Next(const UIControl* ctrl) noexcept -> const UIControl* { return ctrl->next; }
    // prev control
    static auto Prev(UIControl* ctrl) noexcept { return ctrl->prev; }
    // next control
    static auto Next(UIControl* ctrl) noexcept { return ctrl->next; }
    // mark window minsize changed
    static void MarkWindowMinsizeChanged(UIControl& ctrl) noexcept { ctrl.mark_window_minsize_changed(); }
    // clear added
    static void ClearAdded(UIControl& ctrl) noexcept { ctrl.m_state.added_to_this = false; }
    // add child
    static void AddChild(UIControl& o, UIControl& c) noexcept { o.add_child(c); }
};

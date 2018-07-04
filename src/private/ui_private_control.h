#pragma once

// ui
#include <control/ui_control.h>

// c++
#include <cassert>


/// <summary>
/// private function/data to UIControl
/// </summary>
struct LongUI::UIControlPrivate {
    // test if world changed
    static bool TestWorldChanged(UIControl& ctrl) noexcept {
        return ctrl.m_state.world_changed;}
    // ensure bgc renderer
    static auto EnsureBgcRenderer(UIControl& ctrl) noexcept->CUIRendererBackground*;
    // get  bgc renderer
    static auto GetBgcRenderer(const UIControl& ctrl) noexcept->CUIRendererBackground*;
    // refresh min size
    static void RefreshMinSize(UIControl& ctrl) noexcept;
    // update world
    static void UpdateWorld(UIControl& ctrl) noexcept;
    // Synchronous Init Data
    static void SyncInitData(UIControl& ctrl) noexcept;
    // do mouse enter event
    static auto DoMouseEnter(UIControl& ctrl, const Point2F& pos) noexcept->EventAccept;
    // do mouse leave event
    static auto DoMouseLeave(UIControl& ctrl, const Point2F& pos) noexcept->EventAccept;
    // ----------- INLINE ZONE ----------------
    // set focusable
    static void SetFocusable(UIControl& ctrl, bool f) noexcept {
        ctrl.m_state.focusable = f; }
    // set appearance
    static void SetAppearance(UIControl& ctrl, AttributeAppearance a) noexcept {
        ctrl.m_oStyle.appearance = a; }
    // set appearance if not set
    static void SetAppearanceIfNotSet(UIControl& ctrl, AttributeAppearance a) noexcept {
        auto& appearance = ctrl.m_oStyle.appearance;
        if (appearance == Appearance_NotSet) ctrl.m_oStyle.appearance = a; }
    // set flex
    static void SetFlex(UIControl& ctrl, float flex) noexcept {
        ctrl.m_oStyle.flex = flex; }
    // set box minwidth
    static void SetBoxMinWidth(UIControl& ctrl, float minw) noexcept {
        ctrl.m_oBox.minsize.width = minw; }
    // set orient
    static void SetOrient(UIControl& ctrl, bool o) noexcept {
        ctrl.m_state.orient = o; }
    // is atomicity
    static auto IsAtomicity(const UIControl& ctrl) noexcept {
        return ctrl.m_state.atomicity; }
    // get parent data
    static auto GetParentData(const UIControl& ctrl) noexcept {
        return ctrl.m_uData4Parent; }
    // get style state
    static auto&GetStyleState(UIControl& ctrl) noexcept {
        return ctrl.m_oStyle.state; }
    // get style matched value
    static auto&GetStyleMatched(UIControl& ctrl) noexcept {
        return ctrl.m_oStyle.matched; }
    // set parent data
    static auto SetParentData(UIControl& ctrl, uint32_t data) noexcept {
        return ctrl.m_uData4Parent = data; }
    // set gui event to parent
    static void SetGuiEvent2Parent(UIControl& ctrl) noexcept {
        ctrl.m_state.gui_event_to_parent = true; }
    // call add child
    static void CallAddChild(UIControl& ctrl, UIControl& child) noexcept {
        ctrl.add_child(child); }
    // call add attribute
    static void AddAttribute(UIControl& ctrl, uint32_t key, U8View value) noexcept {
        ctrl.add_attribute(key, value); }
    // prev control
    static auto&Prev(UIControl& ctrl) noexcept {
        return static_cast<UIControl&>(*ctrl.prev); }
    // next control
    static auto&Next(UIControl& ctrl) noexcept  {
        return static_cast<UIControl&>(*ctrl.next); }
    // prev control
    static auto Prev(UIControl* ctrl) noexcept {
        return static_cast<UIControl*>(ctrl->prev); }
    // next control
    static auto Next(UIControl* ctrl) noexcept  {
        return static_cast<UIControl*>(ctrl->next); }
    // mark window minsize changed
    static void MarkWindowMinsizeChanged(UIControl& ctrl) noexcept {
         ctrl.mark_window_minsize_changed(); }
    // mark font&text changed
    static void MarkTFChanged(UIControl& ctrl) noexcept {
         ctrl.m_state.textfont_changed = true; }
    // mark text color changed
    static void MarkTCChanged(UIControl& ctrl) noexcept {
         ctrl.m_state.textcolor_changed = true; }
};

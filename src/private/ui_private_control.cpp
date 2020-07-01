#include <graphics/ui_bg_renderer.h>
#include <graphics/ui_bd_renderer.h>
#include "ui_private_control.h"

#include <algorithm>

#include <debugger/ui_debug.h>


#ifndef LUI_DISABLE_STYLE_SUPPORT
/// <summary>
/// Gets the BGC renderer.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
auto LongUI::UIControlPrivate::GetBgRenderer(
    const UIControl & ctrl) noexcept -> CUIRendererBackground * {
    return ctrl.m_pBgRender;
}

/// <summary>
/// Gets the bd renderer.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
auto LongUI::UIControlPrivate::GetBdRenderer(
    const UIControl & ctrl) noexcept -> CUIRendererBorder * {
    return ctrl.m_pBdRender;
}

PCN_NOINLINE
/// <summary>
/// Ensures the BG-Color renderer.
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
auto LongUI::UIControlPrivate::EnsureBgRenderer(
    UIControl& ctrl) noexcept ->CUIRendererBackground* {
    // ensure 函数尽量保证不内联
    if (ctrl.m_pBgRender)
        return ctrl.m_pBgRender;
    else
        return ctrl.m_pBgRender = new(std::nothrow) CUIRendererBackground;
}

PCN_NOINLINE
/// <summary>
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
auto LongUI::UIControlPrivate::EnsureBdRenderer(
    UIControl& ctrl) noexcept ->CUIRendererBorder* {
    // ensure 函数尽量保证不内联
    if (ctrl.m_pBdRender)
        return ctrl.m_pBdRender;
    else
        return ctrl.m_pBdRender = new(std::nothrow) CUIRendererBorder;
}
#endif

#if 0
/// <summary>
/// Recursives the minimum size of the refresh.
/// </summary>
/// <returns></returns>
void LongUI::UIControlPrivate::RefreshMinSize(UIControl& ctrl) noexcept {
#ifndef NDEBUG
    if (!std::strcmp(ctrl.name_dbg, "scrollarea::vscrollbar"))
        int bk = 9;
#endif
    // 递归更新子控件
    for (auto& child : ctrl) {
        // 保存现在的大小
        alignas(uint64_t) const auto minsize1 = child.GetMinSize();
        // 更新大小
        UIControlPrivate::RefreshMinSize(child);
        // 保存现在的大小
        alignas(uint64_t) const auto minsize2 = child.GetMinSize();
        // 修改了
        const auto a = reinterpret_cast<const uint64_t&>(minsize1);
        const auto b = reinterpret_cast<const uint64_t&>(minsize2);
        // 在64位下可以只判断一次
        if (a != b) { 
            ctrl.NeedUpdate(Reason_ChildLayoutChanged);
            // XXX: 最小大小?
            child.NeedUpdate(Reason_ChildLayoutChanged);
            //child.NeedUpdate(Reason_MinSizeChanged);
        }
    }
    // 刷新大小
    ctrl.DoEvent(&ctrl, { NoticeEvent::Event_RefreshBoxMinSize });
}
#endif


/// <summary>
/// Updates the world.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <param name="size">The size of window.</param>
/// <returns></returns>
void LongUI::UIControlPrivate::UpdateWorldTop(UIControl& ctrl, Size2L size) noexcept {
    // 根节点
    if (ctrl.m_state.world_changed) {
        ctrl.m_state.world_changed = false;
        // UIViewport::resize_window 中修改了box.visible
        ctrl.m_oBox.visible;
        // 强行刷新子对象
        for (auto& child : ctrl) {
            child.m_state.world_changed = true;
            UIControlPrivate::UpdateWorldForce(child);
        }
        return;
    }
    // 刷新子对象
    for (auto& child : ctrl) UIControlPrivate::UpdateWorld(child);
}


/// <summary>
/// FORCE Updates the world.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
void LongUI::UIControlPrivate::UpdateWorldForce(UIControl & ctrl) noexcept {
    // XXX: 不可视
    //if (!ctrl.IsVisible()) return;
    // XXX: 优化
    ctrl.m_state.world_changed = false;
    Matrix3X2F matrix;
    const auto parent = ctrl.GetParent();
    matrix = parent->RefWorld();
    matrix._31 += ctrl.GetPos().x * matrix._11;
    matrix._32 += ctrl.GetPos().y * matrix._22;
    // 固定位置? 不是!
    if (ctrl.m_state.attachment == Attachment_Scroll) {
        matrix._31 -= parent->m_ptChildOffset.x * matrix._11;
        matrix._32 -= parent->m_ptChildOffset.y * matrix._22;
    }
    ctrl.m_mtWorld = matrix;

    auto& box = ctrl.m_oBox;
    auto ctrl_rect = ctrl.RefBox().GetSafeBorderEdge();
    ctrl.MapToWindow(ctrl_rect);
    // 检查父控件
    const auto ctn = parent->RefBox().visible;
    ctrl_rect.top = std::max(ctn.top, ctrl_rect.top);
    ctrl_rect.left = std::max(ctn.left, ctrl_rect.left);
    ctrl_rect.right = std::min(ctn.right, ctrl_rect.right);
    ctrl_rect.bottom = std::min(ctn.bottom, ctrl_rect.bottom);
    box.visible = ctrl_rect;
    // 强行刷新子对象
    for (auto& child : ctrl) UIControlPrivate::UpdateWorldForce(child);
}

/// <summary>
/// Updates the world.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
void LongUI::UIControlPrivate::UpdateWorld(UIControl& ctrl) noexcept {
    // 需要更新
    if (ctrl.m_state.world_changed) UIControlPrivate::UpdateWorldForce(ctrl);
    // 刷新子对象
    else for (auto& child : ctrl) UIControlPrivate::UpdateWorld(child);
}

/// <summary>
/// Does the mouse enter.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
auto LongUI::UIControlPrivate::DoMouseEnter(
    UIControl& ctrl, const Point2F& pos) noexcept -> EventAccept {
    return ctrl.DoMouseEvent({ MouseEvent::Event_MouseEnter, 0.f, pos.x, pos.y });
}

/// <summary>
/// Does the mouse enter.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
auto LongUI::UIControlPrivate::DoMouseLeave(
    UIControl& ctrl, const Point2F& pos) noexcept -> EventAccept {
    return ctrl.DoMouseEvent({ MouseEvent::Event_MouseLeave, 0.f, pos.x, pos.y });
}
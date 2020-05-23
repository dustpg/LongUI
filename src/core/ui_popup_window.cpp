#include <core/ui_popup_window.h>
#include <control/ui_viewport.h>
#include <control/ui_control.h>
#include <core/ui_manager.h>
#include <text/ui_ctl_arg.h>
#include <core/ui_string.h>

#include <cassert>
#include <algorithm>


// longui namespace
namespace LongUI {
    // BKDR Hash Function
    auto BKDRHash(const char* a, const char* b) noexcept->uint32_t;
    // impl
    namespace impl {
        // flip mode
        enum flip_mode : uint32_t {
            mode_flip_n = 0,
            mode_flip_v = 1 << 0,
            mode_flip_h = 1 << 1,
        };
        // get screen work area size
        RectL work_area_from(HWND) noexcept;
        // adjust via pos
        void adjust_via(RectWHL& inout, const RectL&, AttributePopupPosition) noexcept;
        // adjust via wnd
        void adjust_via(RectWHL& inout, const RectL&, const RectL&, flip_mode mode) noexcept;
    }
}

/// <summary>
/// Popups the name of the window from.
/// </summary>
/// <param name="hoster">The hoster control.</param>
/// <param name="name">The name.</param>
/// <param name="pointer">The pointer position.</param>
/// <param name="type">The type.</param>
/// <param name="position">The position.</param>
/// <returns></returns>
auto LongUI::PopupWindowFromName(
    UIControl& hoster,
    const char* name, 
    Point2F pointer,
    PopupType type,
    AttributePopupPosition position) noexcept ->EventAccept {
    // 查找目标副视口
    UIViewport* target = nullptr;
    if (name) {
        const auto wnd = hoster.GetWindow();
        assert(wnd && "cannot popup from window less control");
        auto& vp = wnd->RefViewport();
        // 优先查找发起者所在窗口
        target = vp.FindSubViewportWithUID(name);
        // 没有就全局查找
        if (!target) target = UIManager.FindSubViewportWithUID(name);
    }
    // 没有就无视掉
    if (!target) return Event_Ignore;
    // 弹出窗口
    LongUI::PopupWindowFromViewport(hoster, *target, pointer, type, position);
    return Event_Accept;
}

/// <summary>
/// Popups the window from viewport.
/// </summary>
/// <param name="hoster">The control.</param>
/// <param name="viewport">The viewport.</param>
/// <param name="pointer">The pointer position.</param>
/// <param name="type">The type.</param>
/// <param name="position">The position.</param>
/// <returns></returns>
void LongUI::PopupWindowFromViewport(
    UIControl& hoster,
    UIViewport& viewport, 
    Point2F pointer,
    PopupType type,
    AttributePopupPosition position) noexcept {
    // 获取窗口数据
    auto& window = viewport.RefWindow();
    const auto this_window = hoster.GetWindow();
    assert(this_window);
    pointer = this_window->MapToScreenEx(pointer);
    auto area = viewport.GetMinSize();
    impl::flip_mode mode = impl::mode_flip_n;
    // 讨论大小
    switch (type)
    {
    case LongUI::PopupType::Type_Exclusive:
        //LUIDebug(Hint) << "Type_Exclusive" << endl;
        area.width = hoster.GetBox().GetBorderSize().width;
        mode = impl::mode_flip_v;
        break;
    case LongUI::PopupType::Type_PopupH:
        //LUIDebug(Hint) << "Type_PopupH" << endl;
        area.width = std::max(area.width, float(DEFAULT_CONTROL_WIDTH));
        mode = impl::mode_flip_h;
        break;
    case LongUI::PopupType::Type_PopupV:
        //LUIDebug(Hint) << "Type_PopupV" << endl;
        area.width = std::max(area.width, float(DEFAULT_CONTROL_WIDTH));
        mode = impl::mode_flip_v;
        break;
    case LongUI::PopupType::Type_Context:
        //LUIDebug(Hint) << "Type_Context" << endl;
        area.width = std::max(area.width, float(DEFAULT_CONTROL_WIDTH));
        break;
    case LongUI::PopupType::Type_Tooltip:
        //LUIDebug(Hint) << "Type_Tooltip" << endl;
        // XXX: 简单增加 字体高度 + 10?
        pointer.y += UIManager.GetDefaultFont().size + 10.f;
        break;
    }
    // 新的主持
    viewport.AssignNewHoster(hoster);
    // 调整大小
    window.ResizeRelative(area);
    // DPI缩放
    const auto zoomed = window.RefViewport().AdjustSize(area);
    RectWHL target;
    target.left = int32_t(pointer.x);
    target.top = int32_t(pointer.y);
    target.width = zoomed.width;
    target.height = zoomed.height;
    // 调整位置
    const auto work = impl::work_area_from(this_window->GetHwnd());
    const auto rect = [=, &hoster]() noexcept {
        RectF rect = hoster.GetBox().visible;
        RectL rv;
        rv.left = int32_t(rect.left);
        rv.top = int32_t(rect.top);
        rv.right = int32_t(rect.right);
        rv.bottom = int32_t(rect.bottom);
        this_window->MapToScreen(rv);
        return rv;
    }();
    impl::adjust_via(target, rect, position);
    impl::adjust_via(target, rect, work, mode);
    // 正式弹出
    this_window->PopupWindow(window, { target.left, target.top }, type);
}


/// <summary>
/// Popups the window from tooltip text.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <param name="text">The text.</param>
/// <param name="pointer">The pointer.</param>
/// <param name="position">The position.</param>
/// <returns></returns>
void LongUI::PopupWindowFromTooltipText(
    UIControl& ctrl, 
    const char* text, 
    Point2F pointer,
    AttributePopupPosition position) noexcept {
    assert(text && "can not send null");
    const auto window = ctrl.GetWindow();
    assert(window && "window cannot be null if tooltip");
    const auto ptr = window->TooltipText(CUIString::FromUtf8(text));
    if (!ptr) return;
    UIManager.CreateTimeCapsule([=, &ctrl](float) noexcept {
        constexpr auto type = PopupType::Type_Tooltip;
        LongUI::PopupWindowFromViewport(ctrl, *ptr, pointer, type, position);
        //const auto name = UIManager.GetUniqueText("moretip"_sv);
        //LongUI::PopupWindowFromName(ctrl, name, pos, type);
    }, 0.f, &ctrl);
}

/// <summary>
/// Popups the window close tooltip.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
void LongUI::PopupWindowCloseTooltip(UIControl& ctrl) noexcept {
    const auto window = ctrl.GetWindow();
    assert(window && "cannot close toolip for null");
    window->CloseTooltip();
}

/// <summary>
/// adjust from
/// </summary>
/// <param name="inout"></param>
/// <param name="area"></param>
/// <param name="pos"></param>
/// <returns></returns>
void LongUI::impl::adjust_via(RectWHL& inout, const RectL& area, AttributePopupPosition pos) noexcept {
#ifndef NDEBUG
    if (pos == AttributePopupPosition::Position_Default) return;
#endif
    switch (pos)
    {
    case LongUI::AttributePopupPosition::Position_Default:
        break;
    case LongUI::AttributePopupPosition::Position_BeforeStart:
        inout.left = area.left;
        inout.top = area.top - inout.height;
        break;
    case LongUI::AttributePopupPosition::Position_BeforeEnd:
        inout.left = area.right - inout.width;
        inout.top = area.top - inout.height;
        break;
    case LongUI::AttributePopupPosition::Position_AfterStart:
        inout.left = area.left;
        inout.top = area.bottom;
        break;
    case LongUI::AttributePopupPosition::Position_AfterEnd:
        inout.left = area.right - inout.width;
        inout.top = area.bottom;
        break;
    case LongUI::AttributePopupPosition::Position_StartBefore:
        inout.left = area.left - inout.width;
        inout.top = area.top;
        break;
    case LongUI::AttributePopupPosition::Position_StartAfter:
        inout.left = area.left - inout.width;
        inout.top = area.bottom - inout.height;
        break;
    case LongUI::AttributePopupPosition::Position_EndBefore:
        inout.left = area.right;
        inout.top = area.top;
        break;
    case LongUI::AttributePopupPosition::Position_EndStart:
        inout.left = area.right;
        inout.top = area.bottom - inout.height;
        break;
    case LongUI::AttributePopupPosition::Position_Overlap:
        inout.left = area.left;
        inout.top = area.top;
        break;
    case LongUI::AttributePopupPosition::Position_AtPointer:
        break;
    case LongUI::AttributePopupPosition::Position_AfterPointer:
        inout.top = area.bottom;
        break;
    }
}

/// <summary>
/// adjust rect via screen
/// </summary>
/// <param name="inout"></param>
/// <param name="hoster"></param>
/// <param name="work"></param>
/// <param name="mode">the flip mode</param>
/// <returns></returns>
void LongUI::impl::adjust_via(RectWHL& inout, const RectL& hoster, const RectL& work, flip_mode mode) noexcept {
    const auto rassert = []() noexcept { assert(!"some bad"); };
    // 工作区域过于小
    if (work.right - work.left < DEFAULT_CONTROL_WIDTH) return rassert();
    if (work.bottom - work.top < DEFAULT_CONTROL_HEIGHT) return rassert();
    // TODO: 弹出窗口比屏幕还长/宽怎么处理

    // 允许垂直翻转(初级菜单)?
    if (mode & LongUI::impl::mode_flip_v) {
        // Y座标超过一半并且显示越过下线 -> 翻转到上面
        if (inout.top > (work.top + work.bottom) / 2)
            if (inout.top + inout.height > work.bottom)
                inout.top = hoster.top - inout.height;
    }
    else {
        inout.top = std::min(inout.top, work.bottom - inout.height);
        inout.top = std::max(inout.top, work.top);
    }

    // 允许水平翻转(次级菜单)?
    if (mode & LongUI::impl::mode_flip_h) {
        // X座标高度超过一半并且显示越过右线 -> 翻转到左边
        if (inout.left > (work.left + work.right) / 2)
            if (inout.left + inout.width > work.right)
                inout.left = hoster.left - inout.width;
    }
    else {
        inout.left = std::min(inout.left, work.right - inout.width);
        inout.left = std::max(inout.left, work.left);
    }
}

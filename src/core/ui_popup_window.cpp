#include <core/ui_popup_window.h>
#include <control/ui_viewport.h>
#include <control/ui_control.h>
#include <core/ui_manager.h>
#include <core/ui_string.h>

#include <cassert>
#include <algorithm>


// longui namespace
namespace LongUI {
    /// <summary>
    /// Popup - adjust the suggested position
    /// </summary>
    /// <param name="inout">input and ouput of position</param>
    /// <param name="hoster">popop hoster control</param>
    void PopupAdjustSuggestedPosition(RectWHF& inout, UIControl& hoster) {

    }
}

/// <summary>
/// Popups the name of the window from.
/// </summary>
/// <param name="hoster">The hoster control.</param>
/// <param name="name">The name.</param>
/// <param name="suggest">The suggested position.</param>
/// <param name="type">The type.</param>
/// <returns></returns>
auto LongUI::PopupWindowFromName(
    UIControl& hoster,
    const char* name, 
    Point2F suggest,
    PopupType type) noexcept ->EventAccept {
    // 查找目标副视口
    UIViewport* target = nullptr;
    if (name) {
        const auto wnd = hoster.GetWindow();
        assert(wnd && "cannot popup from window less control");
        auto& vp = wnd->RefViewport();
        // 优先查找发起者所在窗口
        target = vp.FindSubViewportWithUnistr(name);
        // 没有就全局查找
        if (!target) target = UIManager.FindSubViewportWithUnistr(name);
    }
    // 没有就无视掉
    if (!target) return Event_Ignore;
    // 弹出窗口
    LongUI::PopupWindowFromViewport(hoster, *target, suggest, type);
    return Event_Accept;
}

/// <summary>
/// Popups the window from viewport.
/// </summary>
/// <param name="hoster">The control.</param>
/// <param name="viewport">The viewport.</param>
/// <param name="suggested">The suggested position.</param>
/// <param name="type">The type.</param>
/// <returns></returns>
void LongUI::PopupWindowFromViewport(
    UIControl& hoster,
    UIViewport& viewport, 
    Point2F suggested,
    PopupType type) noexcept {
    // 获取窗口数据
    auto& window = viewport.RefWindow();
    const auto this_window = hoster.GetWindow();
    assert(this_window);
    //LUIDebug(Hint) << pos << endl;
    RectWHF position;
    position = suggested;
    position = viewport.GetMinSize();
    // 讨论大小
    switch (type)
    {
    case LongUI::PopupType::Type_Exclusive:
        position.width = hoster.GetBox().GetBorderSize().width;
        break;
    case LongUI::PopupType::Type_Popup:
        position.width = std::max(position.width, float(DEFAULT_CONTROL_WIDTH));
        break;
    case LongUI::PopupType::Type_Context:
        position.width = std::max(position.width, float(DEFAULT_CONTROL_WIDTH));
        break;
    case LongUI::PopupType::Type_Tooltip:
        position.top += 10.f;
        break;
    }
    // TODO: DPI缩放

    // 设置新的
    viewport.AssignNewHoster(hoster);
    // 调整座标
    LongUI::PopupAdjustSuggestedPosition(position, hoster);
    // 调整大小
    window.ResizeRelative(position.size());
    // 正式弹出
    this_window->PopupWindow(window, position.point(), type);
}


/// <summary>
/// Popups the window from tooltip text.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <param name="text">The text.</param>
/// <param name="pos">The position.</param>
/// <returns></returns>
void LongUI::PopupWindowFromTooltipText(
    UIControl& ctrl, 
    const char* text, 
    Point2F suggest) noexcept {
    assert(text && "can not send null");
    const auto window = ctrl.GetWindow();
    assert(window && "window cannot be null if tooltip");
    const auto ptr = window->TooltipText(CUIString::FromUtf8(text));
    if (!ptr) return;
    UIManager.CreateTimeCapsule([&ctrl, ptr, suggest](float) noexcept {
        constexpr auto type = PopupType::Type_Tooltip;
        LongUI::PopupWindowFromViewport(ctrl, *ptr, suggest, type);
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

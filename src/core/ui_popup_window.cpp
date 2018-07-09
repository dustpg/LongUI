#include <core/ui_popup_window.h>
#include <control/ui_viewport.h>
#include <control/ui_control.h>
#include <core/ui_manager.h>

#include <cassert>
#include <algorithm>

/// <summary>
/// Popups the name of the window from.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <param name="name">The name.</param>
/// <param name="pos">The position.</param>
/// <param name="type">The type.</param>
/// <returns></returns>
auto LongUI::PopupWindowFromName(
    UIControl& ctrl, 
    const char* name, 
    Point2F pos,
    PopupType type) noexcept ->EventAccept {
    // 查找目标副视口
    UIViewport* target = nullptr;
    if (name) {
        const auto wnd = ctrl.GetWindow();
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
    LongUI::PopupWindowFromViewport(ctrl, *target, pos, type);
    return Event_Accept;
}

/// <summary>
/// Popups the window from viewport.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <param name="viewport">The viewport.</param>
/// <param name="pos">The position.</param>
/// <param name="type">The type.</param>
/// <returns></returns>
void LongUI::PopupWindowFromViewport(
    UIControl& ctrl, 
    UIViewport& viewport, 
    Point2F pos,
    PopupType type) noexcept {
    // 获取窗口数据
    auto& window = viewport.RefWindow();
    const auto this_window = ctrl.GetWindow();
    assert(this_window);
    //LUIDebug(Hint) << pos << endl;
    // 讨论大小
    auto size = viewport.GetMinSize();
    switch (type)
    {
    case LongUI::PopupType::Type_Exclusive:
        size.width = ctrl.GetBox().GetBorderSize().width;
        break;
    default:
        size.width = std::max(size.width, float(DEFAULT_CONTROL_WIDTH));
        break;
#if 0
    case LongUI::PopupType::Type_Popup:
        break;
    case LongUI::PopupType::Type_Context:
        break;
    case LongUI::PopupType::Type_Tooltip:
        break;
#endif
    }
    // TODO: DPI缩放

    // 设置新的
    viewport.AssignNewHoster(ctrl);
    // 调整大小
    const int32_t w = static_cast<int32_t>(size.width);
    const int32_t h = static_cast<int32_t>(size.height);
    window.Resize({ w, h });
    // 正式弹出
    this_window->PopupWindow(window, pos, type);
}


/// <summary>
/// Popups the window from tooltip text.
/// </summary>
/// <param name="ctrl">The control.</param>
/// <param name="text">The text.</param>
/// <param name="pos">The position.</param>
/// <returns></returns>
auto LongUI::PopupWindowFromTooltipText(
    UIControl& ctrl, 
    const char* text, 
    Point2F pos) noexcept->EventAccept {
    assert(text && "can not send null");
    if (!text[0]) return Event_Ignore;

    return Event_Accept;
}
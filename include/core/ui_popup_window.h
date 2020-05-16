#pragma once

// ui
#include "ui_basic_type.h"
#include "ui_core_type.h"
// c++
#include <cstddef>

namespace LongUI {
    // popup type
    enum class PopupType : uint32_t {
        // type exclusive : invoke via exclusive hoster, will keep same width as hoster
        Type_Exclusive = 0,
        // type popup     : maybe by active control(left click)
        Type_Popup,
        // type context   : maybe by context menu(right click)
        Type_Context,  
        // type tooltip   : maybe by hover(leave to release)
        Type_Tooltip,
    };
    // popup window from name
    auto PopupWindowFromName(
        UIControl& hoster,
        const char* name,
        Point2F suggest,
        PopupType type
    ) noexcept ->EventAccept;
    // popup window from viewport
    void PopupWindowFromViewport(
        UIControl& hoster,
        UIViewport& viewport,
        Point2F suggest,
        PopupType type
    ) noexcept;
    // popup window from tooltip text
    void PopupWindowFromTooltipText(
        UIControl& hoster,
        const char* text,
        Point2F suggest
    ) noexcept;
    // close tooltip window
    void PopupWindowCloseTooltip(
        UIControl& hoster
    ) noexcept;
}
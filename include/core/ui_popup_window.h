#pragma once

// ui
#include "ui_basic_type.h"
#include "ui_core_type.h"
// c++
#include <cstddef>

namespace LongUI {
    // position type
    enum class AttributePopupPosition : uint32_t;
    // popup type
    enum class PopupType : uint32_t {
        // type exclusive : invoke via exclusive hoster, will keep same width as hoster
        Type_Exclusive = 0,
        // type popup-h   : maybe by active control(left click)
        Type_PopupH,
        // type popup-v   : maybe by active control(left click)
        Type_PopupV,
        // type context   : maybe by context menu(right click)
        Type_Context,  
        // type tooltip   : maybe by hover(leave to release)
        Type_Tooltip,
    };
    // popup window from name
    auto PopupWindowFromName(
        UIControl& hoster,
        const char* name,
        Point2F pointer,
        PopupType type,
        AttributePopupPosition position
    ) noexcept ->EventAccept;
    // popup window from viewport
    void PopupWindowFromViewport(
        UIControl& hoster,
        UIViewport& viewport,
        Point2F pointer,
        PopupType type,
        AttributePopupPosition position
    ) noexcept;
    // popup window from tooltip text
    void PopupWindowFromTooltipText(
        UIControl& hoster,
        const char* text,
        Point2F pointer,
        AttributePopupPosition position
    ) noexcept;
    // close tooltip window
    void PopupWindowCloseTooltip(
        UIControl& hoster
    ) noexcept;
}
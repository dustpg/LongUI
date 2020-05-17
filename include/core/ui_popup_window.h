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
        // type popup-h   : maybe by active control(left click)
        Type_PopupH,
        // type popup-v   : maybe by active control(left click)
        Type_PopupV,
        // type context   : maybe by context menu(right click)
        Type_Context,  
        // type tooltip   : maybe by hover(leave to release)
        Type_Tooltip,
    };
    // popup pos
    enum class PopupPosition : uint32_t {
        // default
        Position_Default = 0,
        // BEFORE_START
        Position_BeforeStart,
        // BEFORE_END
        Position_BeforeEnd,
        // AFTER_START
        Position_AfterStart,
        // AFTER_END
        Position_AfterEnd,
        // START_BEFORE
        Position_StartBefore,
        // START_AFTER
        Position_StartAfter,
        // END_BEFORE
        Position_EndBefore,
        // END_AFTER
        Position_EndStart,
        // OVERLAP
        Position_Overlap,
        // AT_POINTER
        Position_AtPointer,
        // AFTER_POINTER
        Position_AfterPointer,
    };
    // popup window from name
    auto PopupWindowFromName(
        UIControl& hoster,
        const char* name,
        Point2F pointer,
        PopupType type,
        PopupPosition position
    ) noexcept ->EventAccept;
    // popup window from viewport
    void PopupWindowFromViewport(
        UIControl& hoster,
        UIViewport& viewport,
        Point2F pointer,
        PopupType type,
        PopupPosition position
    ) noexcept;
    // popup window from tooltip text
    void PopupWindowFromTooltipText(
        UIControl& hoster,
        const char* text,
        Point2F pointer,
        PopupPosition position
    ) noexcept;
    // close tooltip window
    void PopupWindowCloseTooltip(
        UIControl& hoster
    ) noexcept;
}
#pragma once

#include <cstdint>

namespace LongUI {
    /// <summary>
    /// notice event type
    /// </summary>
    enum class NoticeEvent : uint32_t {
        // initialize
        Event_Initialize = 0,
        // refresh min size
        Event_RefreshBoxMinSize,
        // do default action
        Event_DoDefaultAction,
        // viewport/ direct-managed-parent only
        Event_UIEvent,
        // viewport only
        Event_WindowClosed,
        // popup hoster only
        Event_PopupClosed,
        // image-child clicked 
        Event_ImageChildClicked,
        // splitter drag
        Event_Splitter,
    };
    /// <summary>
    /// event argument
    /// </summary>
    struct EventArg {
        // event type
        NoticeEvent     nevent;
        // derived used data
        uint32_t        derived;
    };
}
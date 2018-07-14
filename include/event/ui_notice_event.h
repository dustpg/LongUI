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
        // do default/access action
        Event_DoAccessAction,
        // show access key  [derived: 1/0 show/hide]
        Event_ShowAccessKey,
        // viewport/ uievent-to-parent only
        Event_UIEvent,
        // popup hoster only
        Event_PopupBegin,
        // popup hoster only
        Event_PopupEnd,
        // image-child clicked 
        Event_ImageChildClicked,
        // splitter drag
        Event_Splitter,
        // implicit radio group member checked
        Event_ImplicitGroupChecked
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
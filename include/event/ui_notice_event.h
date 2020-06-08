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
        // viewport/ uievent-to-parent only
        Event_UIEvent,
        // show access key  [derived: 1/0 show/hide]
        Event_ShowAccessKey,
        // popup hoster only
        Event_PopupBegin,
        // popup hoster only
        Event_PopupEnd,
        // splitter drag
        Event_Splitter,
        // implicit radio group member checked
        Event_ImplicitGroupChecked,
        // get indirect TextFont pointer
        Event_IndirectTextFont,


        // ----------------
        // timer event 0~7
        // ----------------

        // timer event #0
        Event_Timer0,
        // timer event #1
        Event_Timer1,
        // timer event #2
        Event_Timer2,
        // timer event #3
        Event_Timer3,
        // timer event #4
        Event_Timer4,
        // timer event #5
        Event_Timer5,
        // timer event #6
        Event_Timer6,
        // timer event #7
        Event_Timer7,
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
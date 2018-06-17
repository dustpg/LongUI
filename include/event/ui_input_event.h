#pragma once

#include <cstdint>


namespace LongUI {
    /// <summary>
    /// normal input event
    /// </summary>
    enum class InputEvent : uint32_t {
        // event char
        Event_Char = 0,
        // event key
        Event_KeyDown,

        // --------------------- Unified Input -----------------

        // event-left 
        Event_TurnLeft,
        // event-top
        Event_TurnUp,
        // event-right
        Event_TurnRight,
        // event-down
        Event_TurnDown,
        // event-next       : RB R1
        Event_TurnNext,
        // event-prev       : LB L1
        Event_TurnPrev,
        // event-ok         : A ○
        Event_Ok,
        // event-cancel     : B ×
        Event_Cancel,
        // event-function   : X □
        Event_Function,
        // event-action     : Y △
        Event_Action,
    };
    /// <summary>
    /// Argument for mouse event
    /// </summary>
    struct InputEventArg {
        // input event type
        InputEvent      event;
        // character
        char32_t        character;
    };
}
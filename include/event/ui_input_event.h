#pragma once

#include <cstdint>


namespace LongUI {
    /// <summary>
    /// normal input event
    /// </summary>
    enum class InputEvent : uint16_t {
        // event char
        Event_Char = 0,
        // event key
        Event_KeyDown,
#if 0
        // --------------------- Unified Input -----------------
        // event-left 
        Event_TurnLeft,
        // event-top
        Event_TurnUp,
        // event-right
        Event_TurnRight,
        // event-down
        Event_TurnDown,
        // event-next       : RB R1 TAB
        Event_TurnNext,
        // event-prev       : LB L1 Shift+TAB
        Event_TurnPrev,
        // event-ok         : A  ○  Enter
        Event_Ok,
        // event-cancel     : B  ×  ESC
        Event_Cancel,
        // event-function   : X  □  CtxMenu
        Event_Function,
        // event-action     : Y  △  
        Event_Action,
#endif
    };
    /// <summary>
    /// Argument for mouse event
    /// </summary>
    struct InputEventArg {
        // input event type
        InputEvent      event;
        // sequence id  N...3->2->1->[0] in same frame
        uint16_t        sequence;
        // character
        char32_t        character;
    };
}
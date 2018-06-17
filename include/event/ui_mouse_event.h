#pragma once

#include <cstdint>


namespace LongUI {
    /// <summary>
    /// event for longui control event
    /// </summary>
    enum class MouseEvent : uint32_t {
        // mouse wheel in v-dir
        Event_MouseWheelV,
        // mouse wheel in h-dir
        Event_MouseWheelH,
        // mouse enter, send this event even control disabled
        Event_MouseEnter,
        // mouse leave, send this event even control disabled
        Event_MouseLeave,
        // mouse move
        Event_MouseMove,
        // left-button down
        Event_LButtonDown,
        // left-button up
        Event_LButtonUp,
        // right-button down
        Event_RButtonDown,
        // right-button up
        Event_RButtonUp,
        // middle-button down
        Event_MButtonDown,
        // middle-button up
        Event_MButtonUp,
        // mouse idle hover
        Event_MouseHoverTime,
        // event unknown
        Event_Unknown,
    };

    /// <summary>
    /// modifier
    /// </summary>
    enum InputModifier : uint32_t {
        Modifier_None = 0,
        Modifier_LButton    = 1 << 0,
        Modifier_RButton    = 1 << 1,
        Modifier_Shift      = 1 << 2,
        Modifier_Control    = 1 << 3,
        Modifier_MButton    = 1 << 4,
    };
    /// <summary>
    /// Argument for mouse event
    /// </summary>
    struct MouseEventArg {
        // mouse event type
        MouseEvent      type;
        // wheel data
        float           wheel;
        // mouse windows point x
        float           px;
        // mouse windows point y
        float           py;
        // modifier
        InputModifier   modifier;
    };
}
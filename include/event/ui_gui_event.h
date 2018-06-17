#pragma once

#include <cstdint>
#include "ui_notice_event.h"

namespace LongUI {
    /// <summary>
    /// gui event type
    /// </summary>
    enum class GuiEvent : uint32_t {
        // [oncommand]  command
        Event_Command = 0,
        // [onclick]    click(mouse-like-device only)
        Event_Click,
        // [onchange]   value changed
        Event_Change,
        // [onblur]     killed focus
        Event_Blur,
        // [onfocus]    set focus
        Event_Focus,
        // [onselect]   selected
        Event_Select,
    };
    // gui event, EventAccept: have no effect 
    struct EventGuiArg : EventArg {
        // ctor
        EventGuiArg(GuiEvent e) noexcept {
            this->nevent = NoticeEvent::Event_UIEvent;
            this->derived = static_cast<int32_t>(e);
        }
        // get event
        auto GetEvent() const noexcept {
            return static_cast<GuiEvent>(this->derived) ;
        }
    };
}
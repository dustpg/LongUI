#pragma once

#include <cstdint>
#include "ui_notice_event.h"

namespace LongUI {
    /// <summary>
    /// gui event type
    /// </summary>
    /// <seealso cref="https://developer.mozilla.org/en-US/docs/Mozilla/Tech/XUL/Events"/>
    enum class GuiEvent : uint32_t {
        // [unknown]
        Event_Unknown = 0,



        // [onblur] The opposite of the focus event, the blur event is passed after an element loses the focus.
        Event_OnBlur,
        // [onchange] An onchange attribute is an event listener to the object for the Event change.
        // - TextBox            When [pressed enter key, or killed-focus] if text changed
        // - Select List        When the selected item is changed
        // - Scale(Slider)      When value changed
        Event_OnChange,
        // [onclick] This event handler is called when the object is clicked. This event is also sent when the user double-clicks with the mouse.
        Event_OnClick,
        // [ondblclick][unsupport yet] This event handler is called when the object is double-clicked.
        Event_OnDblClick,
        // [onfocus] called on focus
        Event_OnFocus,
        // [onselect] This event is sent to a listbox or tree when an item is selected.
        Event_OnSelect,


        // [oncommand] This event handler is called when the command is activated. 
        // - Radio/Check Box    When the state is changed
        // - Button             When the button (lbtn) clicked
        Event_OnCommand,
        // [oninput] This event is sent when a user enters text in a textbox.
        Event_OnInput,
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
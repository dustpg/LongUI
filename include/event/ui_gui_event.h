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
        // [onfocus] called on focus
        Event_OnFocus,
        // [onclick] This event handler is called when the object is clicked. This event is also sent when the user double-clicks with the mouse.
        Event_OnClick,
        // [ondblclick][unsupport yet] This event handler is called when the object is double-clicked.
        Event_OnDblClick,


        // [onchange] An onchange attribute is an event listener to the object for the Event change.
        // - TextBox            When [pressed enter key, or killed-focus] if text changed
        // - Select List        When the selected item is changed
        // - Scale(Slider)      When value changed
        Event_OnChange,

        // [onselect] This event is sent to a listbox or tree when an item is selected.
        // - MenuList(ComboBox) When the item is selected
        //                    - [unsupported yet(same as oncommand?)]
        // - RadioGroup         When the item is selected
        //                    - [unsupported yet(same as oncommand?)]
        // - UITabBox           When the index is changed
        //                    - [unsupported yet(same as oncommand?)]
        Event_OnSelect,

        // [oncommand] This event handler is called when the command is activated. 
        // - RadioGroup         When the selected item changed
        // - Radio/Check Box    When the state is changed
        // - MenuList(ComboBox) When the item is changed
        // - MenuPopup          When the item is changed
        // - MenuItem           When the item is selected
        // - Button             When the button (lbtn) clicked
        // - UITabBox           When the index is changed
        Event_OnCommand,

        // [oninput] This event is sent when a user enters text in a textbox.
        // - TextBox            When text changed by user-input
        Event_OnInput,


        // [LONGUI INNER][ondecrease]
        // - UISpinButtons
        Event_OnDecrease,

        // [LONGUI INNER][ondecrease]
        // - UISpinButtons
        Event_OnIncrease,
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
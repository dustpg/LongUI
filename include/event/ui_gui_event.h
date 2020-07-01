#pragma once

#include <cstdint>
#include "ui_notice_event.h"

namespace LongUI {
    // control
    class UIControl;
    /// <summary>
    /// gui event type
    /// </summary>
    /// <seealso cref="https://developer.mozilla.org/en-US/docs/Mozilla/Tech/XUL/Events"/>
    enum class GuiEvent : uint32_t {
        // [unknown]
        // [unknown]
        Event_Unknown = 0,

        // [onblur] The opposite of the focus event, the blur event is passed after an element loses the focus.
        // [GuiEventArg]
        Event_OnBlur,
        // [onfocus] called on focus
        // [GuiEventArg]
        Event_OnFocus,
        // [onclick] This event handler is called when the object is clicked. This event is also sent when the user double-clicks with the mouse.
        // [GuiEventArg]
        Event_OnClick,
        // [ondblclick] This event handler is called when the object is double-clicked.
        // [GuiEventArg]
        // [unsupport yet] 
        Event_OnDblClick,


        // [onchange] An onchange attribute is an event listener to the object for the Event change.
        // - TextBox            When [pressed enter key, or killed-focus] if text changed
        // - Select List        When the selected item is changed
        // - Scale(Slider)      When value changed
        // [GuiEventArg]
        Event_OnChange,

        // [onselect] This event is sent to a listbox or tree when an item is selected.
        // - MenuList(ComboBox) When the item is selected
        //                    - [unsupported yet(same as oncommand?)]
        // - RadioGroup         When the item is selected
        //                    - [unsupported yet(same as oncommand?)]
        // - UITabBox           When the index is changed
        //                    - [unsupported yet(same as oncommand?)]
        // [GuiEventArg]
        Event_OnSelect,

        // [oncommand] This event handler is called when the command is activated. 
        // - RadioGroup         When the selected item changed
        // - Radio/Check Box    When the state is changed
        // - MenuList(ComboBox) When the item is changed
        // - MenuPopup          When the item is changed
        // - MenuItem           When the item is selected
        // - Button             When the button (lbtn) clicked
        // - UITabBox           When the index is changed
        // [GuiEventArg]
        Event_OnCommand,

        // [oninput] This event is sent when a user enters text in a textbox.
        // - TextBox            When text changed by user-input
        // [GuiEventArg]
        Event_OnInput,


        // [LONGUI INNER][ondecrease]
        // - UISpinButtons
        // [GuiEventArg]
        Event_OnDecrease,

        // [LONGUI INNER][ondecrease]
        // - UISpinButtons
        // [GuiEventArg]
        Event_OnIncrease,
    };
    // UIControl pointer but NOT NULL
    using UIControlPtrN2 = UIControl * ;
    // gui event
    struct GuiEventArg : EventArg {
        // orignal target
        UIControl&              target;
        // current target, It's possible this has been changed along the way through retargeting.
        mutable UIControlPtrN2  current;
        // ctor
        GuiEventArg(GuiEvent e, UIControl& t) noexcept : target(t), current(&t) {
            this->nevent = NoticeEvent::Event_UIEvent;
            this->derived = static_cast<int32_t>(e);
        }
        // get event type
        auto GetType() const noexcept { return static_cast<GuiEvent>(this->derived); }
    };
}
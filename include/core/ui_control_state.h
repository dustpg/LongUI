#pragma once
/**
* Copyright (c) 2014-2018 dustpg   mailto:dustpg@gmail.com
*
* Permission is hereby granted, free of charge, to any person
* obtaining a copy of this software and associated documentation
* files (the "Software"), to deal in the Software without
* restriction, including without limitation the rights to use,
* copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following
* conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
* OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
* HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/

#include <cstdint>

namespace LongUI {
#if 0
    // state changed
    enum CCState : uint8_t {
        // non changed was acceptable
        State_NonChanged            =  0,
        // parent changed
        State_ParentChanged         = 1 << 0,
        // child index changed(include child removed/added)
        State_ChildIndexChanged     = 1 << 1,
        // size changed
        State_SizeChanged           = 1 << 2,
        // world matrix changed[include UIScrollArea scroll]
        State_WorldChanged          = 1 << 3,
        // text-font attr changed
        State_TextFontChanged       = 1 << 4,
        // text/value changed, use this if you support text display for optimization
        //  - textbox   : text  changed
        //  - scale     : value/max/min changed
        //  - progress  : value/max changed
        State_ValueTextChanged      = 1 << 5,
    };
    // relayout reason
    enum RelayoutReason : uint8_t {
        // text layout(e.g. font-name) changed
        Reason_TextLayoutChanged = 1 << 0,
        // size changed
        Reason_SizeChanged = 1 << 1,
        // minsize changed(e.g. text changed) changed
        Reason_MinSizeChanged = 1 << 3,
        // text/value changed
        Reason_ValueTextChanged = 1 << 5,
    };
#endif
    // update reason
    enum UpdateReason : uint16_t {
        // non-changed was acceptable
        Reason_NonChanged               = 0,
        // parent changed
        Reason_ParentChanged            = 1 << 0,
        // child index changed(include child removed/added/swapped)
        Reason_ChildIndexChanged        = 1 << 1,
        // size changed
        Reason_SizeChanged              = 1 << 2,
        // box changed
        Reason_BoxChanged               = 1 << 3,
        // text-font display attribute(e.g. text color) changed
        Reason_TextFontDisplayChanged   = 1 << 4,
        // text-font layout attribute(e.g. font-name) changed
        Reason_TextFontLayoutChanged    = 1 << 5,
        // text/value changed
        //  - textbox   : text  changed
        //  - scale     : value/max/min changed
        //  - progress  : value/max changed
        Reason_ValueTextChanged         = 1 << 6,
        // child layout changed
        Reason_ChildLayoutChanged       = 1 << 7,
        // m_pHovered changed
        Reason_HoveredChanged           = 1 << 8,


        // basic relayout reason
        Reason_BasicRelayout 
            = Reason_ParentChanged
            | Reason_ChildIndexChanged
            | Reason_SizeChanged
            | Reason_BoxChanged
            | Reason_ChildLayoutChanged
        ,
    };
    // control state
    struct alignas(uint32_t) CtrlState {
        // ctor
        void Init() noexcept;
        // relayout reason
        UpdateReason    reason;
        // tree level
        uint8_t         level;
        // timer used flag
        uint8_t         timer;
        // accesskey char
        char            accessKey;
        // BYTE#0

        // gui event to parent[true to parent, false to viewport]
        bool        gui_event_to_parent : 1;
        // defaultable
        bool        defaultable : 1;
        // atomicity (children will keep same input-state with parent)
        bool        atomicity : 1;
        // attachment 
        bool        attachment : 1;
        // focusable
        bool        focusable : 1;
        // layout direction
        bool        direction : 1;
        // has been inited
        bool        inited : 1;
        // orientation
        bool        orient : 1;

        // BYTE#1

        // has inline style
        bool        has_inline_style : 1;
        // in dtor
        bool        destructing : 1;
        // tooltip shown?
        bool        tooltip_shown : 1;
        // visible
        bool        visible : 1;
        // is delete later?
        bool        delete_later : 1;
        // in render-dirty list
        bool        in_dirty_list : 1;
        // in update list
        bool        in_update_list : 1;
        // in animation
        bool        in_basic_animation : 1;

        // BYTE#2
        // world changed
        bool        world_changed : 1;
        // dbg output
        bool        dbg_output : 1;
        // debug in update
        bool        dbg_in_update : 1;
    };
    // operator |
    inline constexpr UpdateReason operator|(UpdateReason a, UpdateReason b) noexcept {
        using reason_t = uint16_t;
        static_assert(sizeof(reason_t) == sizeof(a), "bad type");
        return UpdateReason(reason_t(a) | reason_t(b));
    }
    // operator &
    inline constexpr UpdateReason operator&(UpdateReason a, UpdateReason b) noexcept {
        using reason_t = uint16_t;
        static_assert(sizeof(reason_t) == sizeof(a), "bad type");
        return UpdateReason(reason_t(a) & reason_t(b));
    }
    // operator ~
    inline constexpr UpdateReason operator~(UpdateReason a) noexcept {
        using reason_t = uint16_t;
        static_assert(sizeof(reason_t) == sizeof(a), "bad type");
        return UpdateReason(~reason_t(a));
    }
}

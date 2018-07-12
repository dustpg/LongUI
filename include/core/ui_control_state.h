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
    // control state
    struct CtrlState {
        // ctor
        void Init() noexcept;
        // tree level
        uint8_t     level;
        // ------------ life

        // has been inited
        bool        inited : 1;
        // ctor failed
        bool        ctor_failed : 1;
        // in dtor
        bool        destructing : 1;

        // ------------ attr

        // attachment 
        bool        attachment : 1;
        // focusable
        bool        focusable : 1;
        // orientation
        bool        orient : 1;
        // layout direction
        bool        dir : 1;

        // ---------- unstable interface

        // is delete later?
        bool        delete_later : 1;
        // gui event to parent[true, to parent, false to viewport]
        bool        gui_event_to_parent : 1;
        // in animation
        bool        in_basic_animation : 1;
        // defaultable
        bool        defaultable : 1;
        // atomicity (children will keep same input-state with parent)
        bool        atomicity   : 1;
        // tooltip shown?
        bool        tooltip_shown : 1;


        // ----------- custom data

        // custom data, defined via control self, donot use this if you donot know
        bool        custom_data : 1;


        // ----------- state

        // textcolor changed
        bool        textcolor_changed : 1;
        // text&font(except color) chnaged
        bool        textfont_changed : 1;
        // layout dirty
        bool        dirty : 1;
        // visible                          [S-falg]
        bool        visible : 1;
        // world matrix changed             [N-flag]
        bool        world_changed : 1;
        // state changed if [animated]      [N-flag]
        bool        style_state_changed : 1;
        // in update list                   [O-flag]
        bool        in_update_list : 1;
        // in render-dirty list             [O-flag]
        bool        in_dirty_list : 1;
        // child index changed(+ - child)   [N-flag]
        bool        child_i_changed : 1;
        // parent changed                   [N-flag]
        bool        parent_changed : 1;

    };
}
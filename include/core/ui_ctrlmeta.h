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

namespace LongUI {
    // control
    class UIControl;
    // using
    using luisize_t = decltype(sizeof(0));
    // inner control flag
    enum class InnerFlag : luisize_t {
        // unknown control
        Flag_Non            = 0 ,
        // is menu item
        Flag_UIMenuItem     = 1 << 0x00,
        // is caption
        Flag_UICaption      = 1 << 0x01,
        // is list item
        Flag_UIListItem     = 1 << 0x02,
        // is list cols
        Flag_UIListCols     = 1 << 0x03,
        // is list head
        Flag_UIListHead     = 1 << 0x04,
        // is list col 
        Flag_UIListCol      = 1 << 0x05,
        // is menu popup
        Flag_UIMenuPopup    = 1 << 0x06,
        // is menu bar
        Flag_UIMenuBar      = 1 << 0x07,
        // is menu
        Flag_UIMenu         = 1 << 0x08,
        // is radio group
        Flag_UIRadioGroup   = 1 << 0x09,
        // is rich list item
        Flag_UIRichListItem = 1 << 0x0A,
        // is tab
        Flag_UITab          = 1 << 0x0B,
        // is tabs
        Flag_UITabs         = 1 << 0x0C,
        // is tab panels
        Flag_UITabPanels    = 1 << 0x0D,
        // is scroll bar
        Flag_UIScrollBar    = 1 << 0x0E,


        // is tree cols
        Flag_UITreeCols     = 1 << 0x10,
        // is tree children
        Flag_UITreeChildren = 1 << 0x11,
        // is tree item
        Flag_UITreeItem     = 1 << 0x12,
        // is tree col
        Flag_UITreeCol      = 1 << 0x13,
        // is tree row
        Flag_UITreeRow      = 1 << 0x14,
    };
    // control meta info
    struct MetaControl {
        // super class
        const MetaControl*  super_class;
        // element name
        const char*         element_name;
        // create control
        auto (*create_func)(UIControl*) noexcept->UIControl*;
        // inner flags to test control type in O(1)
        InnerFlag           inner_flags;
    };
}

// control meta info 
#define LUI_CONTROL_META_INFO(T, ele) \
    static UIControl* create_##T(UIControl* p) noexcept {\
        return new(std::nothrow) T{ p };\
    }\
    const MetaControl T::s_meta = {\
        &T::Super::s_meta,\
        ele,\
        create_##T,\
        InnerFlag::Flag_Non \
    };

// control meta info 
#define LUI_CONTROL_META_INFO_NO(T, ele) \
    const MetaControl T::s_meta = {\
        &T::Super::s_meta,\
        ele,\
        nullptr,\
        InnerFlag::Flag_Non \
    };

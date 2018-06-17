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

// c++
#include <cstdint>

// ui namespace
namespace LongUI {
    // accessible event
    enum class AccessibleControlType : uint32_t {
        // custom type
        Type_Custom = 0,
        // button
        Type_Button,
        // calendar
        Type_Calendar,
        // check-box
        Type_CheckBox,
        // combo-box
        Type_ComboBox,
        // text-edit
        Type_TextEdit,
        // hyperlink
        Type_Hyperlink,
        // image
        Type_Image,
        // list
        Type_List,
        // list item
        Type_ListItem,
        // menu
        Type_Menu,
        // menu bar
        Type_MenuBar,
        // menu item
        Type_MenuItem,
        // progress bar
        Type_ProgressBar,
        // radio button
        Type_RadioButton,
        // scroll bar
        Type_ScrollBar,
        // slider
        Type_Slider,
        // spinner
        Type_Spinner,
        // tab
        Type_Tab,
        // tab item
        Type_TabItem,
        // text
        Type_Text,
        // tool bar
        Type_ToolBar,
        // tooltip
        Type_Tooltip,
        // tree
        Type_Tree,
        // tree item
        Type_TreeItem,
        // group
        Type_Group,
        // thumb
        Type_Thumb,
        // header
        Type_Header,
        // header item
        Type_HeaderItem,
        // table
        Type_Table,
        // NONE
        Type_None,
    };
}
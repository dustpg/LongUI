#pragma once
/**
* Copyright (c) 2014-2020 dustpg   mailto:dustpg@gmail.com
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

#include "ui_button.h"

// ui namespace
namespace LongUI {
    // menu
    class UIMenu : public UIButton {
        // super class
        using Super = UIButton;
        // private impl
        struct Private;
    public:
        // class meta
        static const  MetaControl   s_meta;
        // dtor
        ~UIMenu() noexcept;
        // ctor
        explicit UIMenu(UIControl* parent = nullptr) noexcept : UIMenu(UIMenu::s_meta) { this->final_ctor(parent); }
    protected:
        // ctor
        UIMenu(const MetaControl&) noexcept;
    public:
        // do normal event
        auto DoEvent(UIControl*, const EventArg&)noexcept->EventAccept override;
        // mouse event
        auto DoMouseEvent(const MouseEventArg& e) noexcept->EventAccept override;
    protected:
        // initialize
        void initialize() noexcept override;
        // add child
        void add_child(UIControl& child) noexcept override;
        // add attribute : key = bkdr hashed string key, this method valid before inited
        void add_attribute(uint32_t key, U8View value) noexcept override;
        // try show next level menu
        void try_show_next_level_menu() noexcept;
    };
    // get meta info for UIMenu
    LUI_DECLARE_METAINFO(UIMenu);
}
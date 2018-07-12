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

#include "ui_boxlayout.h"

// ui namespace
namespace LongUI {
    // menuitem
    class UIMenuItem : public UIBoxLayout {
        // super class
        using Super = UIBoxLayout;
        // private impl
        struct Private;
        // init menuitem
        void init_menuitem() noexcept;
    protected:
        // ctor
        UIMenuItem(UIControl* parent, const MetaControl& ) noexcept;
    public:
        // selected event
        static inline constexpr auto _selected() noexcept { return GuiEvent::Event_Select; }
    public:
        // class meta
        static const  MetaControl   s_meta;
        // dtor
        ~UIMenuItem() noexcept;
        // ctor
        UIMenuItem(UIControl* parent = nullptr) noexcept : UIMenuItem(parent, UIMenuItem::s_meta) {}
    public:
        // do normal event
        auto DoEvent(UIControl* sender, const EventArg& arg) noexcept->EventAccept override;
        // do mouse event
        auto DoMouseEvent(const MouseEventArg& e) noexcept->EventAccept override;
        // render
        //void Render() const noexcept override;
    protected:
        // add attribute
        void add_attribute(uint32_t key, U8View value) noexcept override;
        // re-layout
        //void relayout() noexcept override;
    public:
        // get text
        auto GetText() const noexcept->const wchar_t*;
        // get text object
        auto GetTextString() const noexcept->const CUIString&;
    private:
        // private data
        Private*            m_private = nullptr;
    };
    // get meta info for UIMenuItem
    LUI_DECLARE_METAINFO(UIMenuItem);
}
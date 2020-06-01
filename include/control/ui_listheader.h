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

// super
#include "ui_boxlayout.h"
#include "../core/ui_core_type.h"
// base
#include "ui_image.h"
#include "ui_label.h"

// ui namespace
namespace LongUI {
    // listheader
    class UIListHeader : public UIBoxLayout {
        // super class
        using Super = UIBoxLayout;
        // private impl
        struct Private;
    public:
        // class meta
        static const  MetaControl   s_meta;
        // dtor
        ~UIListHeader() noexcept;
        // ctor
        explicit UIListHeader(UIControl* parent = nullptr) noexcept : UIListHeader(UIListHeader::s_meta) { this->final_ctor(parent); }
    protected:
        // ctor
        UIListHeader(const MetaControl&) noexcept;
    public:
        // clicked event
        //static inline constexpr auto _clicked() noexcept { return GuiEvent::Event_Click; }
    public:
        // get text
        //auto GetText() const noexcept ->const wchar_t*;
        //// get text- string object
        //auto RefTextString() const noexcept -> const CUIString&;
        // set text
        void SetText(const CUIString& text) noexcept;
        // set text
        void SetText(CUIString&& text) noexcept;
        // set text
        void SetText(U16View text) noexcept;
    public:
        // do event
        //auto DoEvent(UIControl * sender, const EventArg & e) noexcept->EventAccept override;
        // render
        //void Render() const noexcept override;
        // mouse event
        //auto DoMouseEvent(const MouseEventArg& e) noexcept->EventAccept override;
    protected:
        // add attribute
        void add_attribute(uint32_t key, U8View value) noexcept override;
        // add child
        //void add_child(UIControl&) noexcept override;
        // add private child
        //void add_private_child() noexcept;
#ifdef LUI_ACCESSIBLE
    protected:
        // accessible event
        //auto accessible(const AccessibleEventArg&) noexcept->EventAccept override;
#endif
    private:// private impl
        // private image
        UIImage             m_oImage;
        // private label
        UILabel             m_oLabel;
        // private sort-direction marker
        UIImage             m_oSortDir;
    };
    // get meta info for UIListHeader
    LUI_DECLARE_METAINFO(UIListHeader);
}
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
    // image control
    class UIImage;
    // redio group
    class UIRadioGroup;
    // radio
    class UIRadio : public UIBoxLayout {
        // super class
        using Super = UIBoxLayout;
        // private impl
        struct Private;
        // init radio
        void init_radio() noexcept;
    public:
        // on commnad event
        static constexpr auto _onCommand() noexcept { return GuiEvent::Event_OnCommand; }
    public:
        // set checked
        void SetChecked(bool) noexcept;
        // set selected(sameas checked)
        void SetSelected(bool sel) noexcept { this->SetChecked(sel); }
        // set image source
        void SetImageSource(U8View src) noexcept;
        // get text
        auto GetText() const noexcept ->const char16_t*;
        // get text- string object
        auto GetTextString() const noexcept -> const CUIString&;
        // set text
        void SetText(const CUIString& text) noexcept;
        // set text
        void SetText(CUIString&& text) noexcept;
        // set text
        void SetText(U16View text) noexcept;
    protected:
        // ctor
        UIRadio(UIControl* parent, const MetaControl&) noexcept;
    public:
        // class meta
        static const  MetaControl   s_meta;
        // dtor
        ~UIRadio() noexcept;
        // ctor
        UIRadio(UIControl* parent = nullptr) noexcept : UIRadio(parent, UIRadio::s_meta) {}
        // update
        void Update() noexcept override;
        // do normal event
        auto DoEvent(UIControl* sender, const EventArg& arg) noexcept->EventAccept override;
        // do mouse event
        auto DoMouseEvent(const MouseEventArg& e) noexcept->EventAccept override;
    protected:
        // add attribute
        void add_attribute(uint32_t key, U8View value) noexcept override;
    private:
        // radio group
        UIRadioGroup*       m_pRadioGroup = nullptr;
        // private data
        Private*            m_private = nullptr;
        // image child
        UIImage*            m_pImageChild = nullptr;
    };
    // get meta info for UIRadio
    LUI_DECLARE_METAINFO(UIRadio);
}
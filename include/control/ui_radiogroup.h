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

#include "ui_boxlayout.h"

// ui namespace
namespace LongUI {
    // radio control
    class UIRadio;
    // radiogroup
    class UIRadioGroup : public UIBoxLayout {
        // super class
        using Super = UIBoxLayout;
        // private impl
        struct Private;
    public:
        // command selected changed
        static constexpr auto _onCommand() noexcept { return GuiEvent::Event_OnCommand; }
        // select: item selected
        //static constexpr auto _onSelect() noexcept { return GuiEvent::Event_OnSelect; }
    public:
        // get checked radio
        auto GetChecked() const noexcept { return m_pChecked; }
        // set checked radio
        void SetChecked(UIRadio& radio) noexcept { this->set_checked(&radio); }
        // set checked radio to null
        void SetChecked(std::nullptr_t) noexcept { this->set_checked(nullptr); }
    protected:
        // ctor
        UIRadioGroup(UIControl* parent, const MetaControl&) noexcept;
    public:
        // class meta
        static const  MetaControl   s_meta;
        // dtor
        ~UIRadioGroup() noexcept;
        // ctor
        UIRadioGroup(UIControl* parent = nullptr) noexcept : UIRadioGroup(parent, UIRadioGroup::s_meta) {}
        // update
        void Update() noexcept override;
        //// do normal event
        //auto DoEvent(UIControl* sender, const EventArg& arg) noexcept->EventAccept override;
        //// do mouse event
        //auto DoMouseEvent(const MouseEventArg& e) noexcept->EventAccept override;
    private:
        // set checked radio
        void set_checked(UIRadio* radio) noexcept;
    protected:
        // checked radio
        UIRadio*            m_pChecked = nullptr;
    };
    // get meta info for UIRadioGroup
    LUI_DECLARE_METAINFO(UIRadioGroup);
}
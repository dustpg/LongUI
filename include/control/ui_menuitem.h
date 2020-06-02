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
#include "../style/ui_behavior_type.h"
// base
#include "ui_image.h"
#include "ui_label.h"

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
        UIMenuItem(const MetaControl& ) noexcept;
    public:
        // command: this item is selected
        static constexpr auto _onCommand() noexcept { return GuiEvent::Event_OnCommand; }
    public:
        // set check (if in checkbox/radio mode)
        void SetChecked(bool checked) noexcept;
        // is checked?
        auto IsChecked() const noexcept { return m_oStyle.state & State_Checked; }
        // get text
        auto GetText() const noexcept->const char16_t*;
        // get text object
        auto RefText() const noexcept->const CUIString&;
        // set text
        void SetText(CUIString&&) noexcept;
        // set text
        void SetText(U16View) noexcept;
        // set text
        void SetText(const CUIString&) noexcept;
    public:
        // ICON WIDTH
        enum : uint32_t { ICON_WIDTH = 28 };
        // class meta
        static const  MetaControl   s_meta;
        // dtor
        ~UIMenuItem() noexcept;
        // ctor
        explicit UIMenuItem(UIControl* parent = nullptr) noexcept : UIMenuItem(UIMenuItem::s_meta) { this->final_ctor(parent); }
    public:
        // do normal event
        auto DoEvent(UIControl* sender, const EventArg& arg) noexcept->EventAccept override;
        // do mouse event
        auto DoMouseEvent(const MouseEventArg& e) noexcept->EventAccept override;
        // update
        void Update(UpdateReason) noexcept override;
    protected:
        // add attribute
        void add_attribute(uint32_t key, U8View value) noexcept override;
        // re-layout
        //void relayout() noexcept override;
        // do check box
        void do_checkbox() noexcept;
        // do radio
        void do_radio() noexcept;
        // add acceltext
        void add_acceltext(U8View) noexcept;
    private: // private control impl
        // private image
        UIImage             m_oImage;
        // private label
        UILabel             m_oLabel;
    protected:
        // acceltext label
        UILabel*            m_pAcceltext = nullptr;
        // group name
        const char*         m_pName = nullptr;
        // item type
        BehaviorType        m_type = BehaviorType::Type_Normal;
        // selected before init
        bool                m_bSelInit = false;
    };
    // get meta info for UIMenuItem
    LUI_DECLARE_METAINFO(UIMenuItem);
}
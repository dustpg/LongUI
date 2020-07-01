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
#include "ui_box.h"
// base
#include "ui_image.h"
#include "ui_label.h"

//#define LUI_NO_MENULIST_EDITABLE

// ui namespace
namespace LongUI {
    // text field
    class UITextField;
    // menu popup
    class UIMenuPopup;
    // menulist/combobox
    class UIMenuList : public UIBoxLayout {
        // super class
        using Super = UIBoxLayout;
        // private impl
        struct Private;
    public:
        // command selected changed
        static constexpr auto _onCommand() noexcept { return GuiEvent::Event_OnCommand; }
        // select: item selected
        //static constexpr auto _onSelect() noexcept { return GuiEvent::Event_OnSelect; }
    protected:
        // ctor
        UIMenuList(const MetaControl&) noexcept;
    public:
        // class meta
        static const  MetaControl   s_meta;
        // dtor
        ~UIMenuList() noexcept;
        // ctor
        explicit UIMenuList(UIControl* parent = nullptr) noexcept : UIMenuList(UIMenuList::s_meta) { this->final_ctor(parent); }
    public:
        // get popup pointer
        auto GetPopupObj() const noexcept { return m_pMenuPopup; }
        // show popup
        void ShowPopup() noexcept;
        // get text
        auto GetText() const noexcept ->const char16_t*;
        // get text- string object
        auto RefText() const noexcept -> const CUIString&;
        // set text
        void SetText(const CUIString& text) noexcept;
        // set text
        void SetText(CUIString&& text) noexcept;
        // set text
        void SetText(U16View text) noexcept;
        // get selected index
        long GetSelectedIndex() const noexcept { return m_iSelected; }
        // set selected index
        auto SetSelectedIndex(long) noexcept ->EventAccept;
    public:
        // update
        void Update(UpdateReason) noexcept override;
        // do event
        auto DoEvent(UIControl * sender, const EventArg & e) noexcept->EventAccept override;
        // render
        //void Render() const noexcept override;
        // mouse event
        auto DoMouseEvent(const MouseEventArg& e) noexcept->EventAccept override;
        // input event
        auto DoInputEvent(InputEventArg e) noexcept->EventAccept override;
    protected:
        // initialize
        void initialize() noexcept override;
        // add child
        void add_child(UIControl& child) noexcept override;
#ifndef LUI_NO_MENULIST_EDITABLE
        // add ad
        void add_attribute(uint32_t key, U8View value) noexcept override;
        // create textfield
        void create_textfield() noexcept;
        // private textfield
        UITextField*            m_pTextField = nullptr;
    public:
        // trigger the event
        auto FireEvent(const GuiEventArg& event) noexcept->EventAccept override;
    protected:
#endif
#ifdef LUI_ACCESSIBLE
        // accessible event
        auto accessible(const AccessibleEventArg&) noexcept->EventAccept override;
#endif
    protected:
        // on popup selected changed
        void on_selected_changed() noexcept;
    private: // private impl
        // private image
        UIImage                 m_oImage;
        // private label
        UILabel                 m_oLabel;
        // private marker
        UIImage                 m_oMarker;
    private:
        // popup
        UIMenuPopup*            m_pMenuPopup = nullptr;
        // selected index
        long                    m_iSelected = -1;
    };
    // get meta info for UIMenuList
    LUI_DECLARE_METAINFO(UIMenuList);
}
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
    // menu popup
    class UIMenuPopup;
    // menulist/combobox
    class UIMenuList : public UIBoxLayout {
        // super class
        using Super = UIBoxLayout;
        // private impl
        struct Private;
        // menu list
        void init_menulist();
    public:
        // selected changed
        //static inline constexpr auto _selectedChanged() noexcept { return GuiEvent::Event_Change; }
    protected:
        // ctor
        UIMenuList(UIControl* parent, const MetaControl&) noexcept;
    public:
        // class meta
        static const  MetaControl   s_meta;
        // dtor
        ~UIMenuList() noexcept;
        // ctor
        UIMenuList(UIControl* parent = nullptr) noexcept : UIMenuList(parent, UIMenuList::s_meta) {}
    public:
        // clicked event
        //static inline constexpr auto _clicked() noexcept { return GuiEvent::Event_Click; }
    public:
        // get popup pointer
        auto GetPopupObj() const noexcept { return m_pMenuPopup; }
        // show popup
        void ShowPopup() noexcept;
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
        // get selection index
        auto GetSelectionIndex() const noexcept { return m_iSelected; }
    public:
        // update
        //void Update() noexcept override;
        // do event
        auto DoEvent(UIControl * sender, const EventArg & e) noexcept->EventAccept override;
        // render
        //void Render() const noexcept override;
        // mouse event
        auto DoMouseEvent(const MouseEventArg& e) noexcept->EventAccept override;
    protected:
        // add child
        void add_child(UIControl& child) noexcept override;
#ifdef LUI_ACCESSIBLE
        // accessible event
        //auto accessible(const AccessibleEventArg&) noexcept->EventAccept override;
#endif
    protected:
        // on popup selected changed
        void on_selected_changed() noexcept;
    private:
        // popup
        UIMenuPopup*            m_pMenuPopup = nullptr;
        // private data
        Private*                m_private = nullptr;
        // selected index
        int                     m_iSelected = -1;
    };
    // get meta info for UIMenuList
    LUI_DECLARE_METAINFO(UIMenuList);
}
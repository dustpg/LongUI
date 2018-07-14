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

// ui
#include "ui_viewport.h"

// ui namespace
namespace LongUI {
    // ui meta typedef
    struct UIMetaTypeDef;
    // menu list
    class UIMenuList;
    // menu item
    class UIMenuItem;
    // menupopup control
    class UIMenuPopup : public UIViewport {
        // super class
        using Super = UIViewport;
        // friend class
        friend UIMenuList;
        // friend class
        friend UIMenuItem;
        // friend class
        friend UIMetaTypeDef;
    public:
        // selected changed
        static inline constexpr auto _selectedChanged() noexcept { return GuiEvent::Event_Change; }
    protected:
        // ctor
        UIMenuPopup(UIControl* hoster, const MetaControl&) noexcept;
        // init hoster
        void init_hoster(UIControl* hoster) noexcept { m_pHoster = hoster; }
        // set selected
        //void set_selected(UIControl* hoster) noexcept { m_pHoster = hoster; }
    public:
        // class meta
        static const  MetaControl   s_meta;
        // dtor
        ~UIMenuPopup() noexcept;
        // ctor
        UIMenuPopup(UIControl* hoster) noexcept : UIMenuPopup(hoster, UIMenuPopup::s_meta) {}
        // update
        void Update() noexcept override;
        // do normal event
        auto DoEvent(UIControl* sender, const EventArg& arg) noexcept->EventAccept override;
        // window closed
        void WindowClosed() noexcept override;
        // popup begin
        void SubViewportPopupBegin(UIViewport&, PopupType) noexcept override;
    public:
        // get last selected
        auto GetLastSelected() const noexcept { return m_pLastSelected; }
        // get selected index
        auto GetSelectedIndex() const noexcept { return m_iSelected; }
        // select first item
        void SelectFirstItem() noexcept;
        // clear select
        void ClearSelected() noexcept;
        // set delay closed popup
        void SetDelayClosedPopup() noexcept;
    protected:
        // add child
        //void add_child(UIControl& child) noexcept override;
        // select child
        void select(UIControl* child) noexcept;
        // change select
        static void change_select(UIControl* old, UIControl* now) noexcept;
        // save selected?
        auto is_save_selected() const noexcept { return m_state.custom_data; }
        // save selected: true
        auto save_selected_true() noexcept { m_state.custom_data = true; }
        // save selected: false
        auto save_selected_false() noexcept { m_state.custom_data = false; }
        // selected before init
        //void selected_before_init(UIControl&c) noexcept { m_pPerSelected = &c; }
    public:
        // has padding for menuitem
        bool HasPaddingForItem() const noexcept { return !is_save_selected(); }
    protected:
        // last-selected
        UIMenuItem*             m_pLastSelected = nullptr;
        // pre-selected
        UIControl*              m_pPerSelected = nullptr;
        // delay closed time capsule
        CUITimeCapsule*         m_pDelayClosed = nullptr;
        // selected index
        int32_t                 m_iSelected = -1;
        // mouse in
        bool                    m_bMouseIn = false;
    };
    // get meta info for UIMenuPopup
    LUI_DECLARE_METAINFO(UIMenuPopup);
}
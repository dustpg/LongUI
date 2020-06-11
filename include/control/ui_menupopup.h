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

// ui
#include "ui_viewport.h"
#include "../core/ui_popup_window.h"

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
        // command selected changed
        static constexpr auto _onCommand() noexcept { return GuiEvent::Event_OnCommand; }
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
        // ctor form UIViewport
        UIMenuPopup(CUIWindow::WindowConfig config) noexcept : Super(nullptr, config, UIMenuPopup::s_meta) {};
        // dtor
        ~UIMenuPopup() noexcept;
        // ctor
        UIMenuPopup(UIControl* hoster) noexcept : UIMenuPopup(hoster, UIMenuPopup::s_meta) { this->final_ctor(hoster); }
        // update
        void Update(UpdateReason) noexcept override;
        // do normal event
        auto DoEvent(UIControl* sender, const EventArg& arg) noexcept->EventAccept override;
        // do mouse event
        auto DoMouseEvent(const MouseEventArg& e) noexcept->EventAccept;
        // window closed
        void WindowClosed() noexcept override;
        // popup begin
        void SubViewportPopupBegin(UIViewport&, PopupType) noexcept override;
    public:
        // add item
        void AddItem(CUIString&& label) noexcept;
        // get last selected
        auto GetLastSelected() const noexcept { return m_pLastSelected; }
        // get selected index
        auto GetSelectedIndex() const noexcept { return m_iSelected; }
        // get popup positon
        auto GetPopupPosition() const noexcept { return m_posPopup; }
        // select first item
        void SelectFirstItem() noexcept;
        // clear select
        void ClearSelected() noexcept;
        // set delay closed popup
        void SetDelayClosedPopup() noexcept;
        // mark no delay closed popup
        void MarkNoDelayClosedPopup() noexcept;
    protected:
        // add attribute
        void add_attribute(uint32_t key, U8View view) noexcept override;
        // select child
        void select(UIControl* child) noexcept;
        // change select
        static void change_select(UIControl* old, UIControl* now) noexcept;
        // save selected?
        auto is_save_selected() const noexcept { return m_bSaveSel; }
        // save selected: true
        auto save_selected_true() noexcept { m_bSaveSel = true; }
        // save selected: false
        auto save_selected_false() noexcept { m_bSaveSel = false; }
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
        // popup postion
        AttributePopupPosition  m_posPopup = AttributePopupPosition::Position_Default;
        // selected index
        int32_t                 m_iSelected = -1;
        // mouse in
        bool                    m_bMouseIn = false;
        // no delay closed once
        bool                    m_bNoClosedOnce = false;
        // save sel
        bool                    m_bSaveSel = false;
    };
    // get meta info for UIMenuPopup
    LUI_DECLARE_METAINFO(UIMenuPopup);
}
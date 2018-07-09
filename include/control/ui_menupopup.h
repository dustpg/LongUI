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
        // get selected
        auto GetSelected() const noexcept { return m_pSelected; }
        // get selected index
        auto GetSelectedIndex() const noexcept { return m_iSelected; }
    protected:
        // add child
        void add_child(UIControl& child) noexcept override;
        // select child
        void select(UIControl* child) noexcept;
        // change select
        static void change_select(UIControl* old, UIControl* now) noexcept;
    protected:
        // selected control
        UIMenuItem*             m_pSelected = nullptr;
        // pre-selected
        UIControl*              m_pPerSelected = nullptr;
        // selected index
        int32_t                 m_iSelected = -1;
    private:
        // hoster
        // private data
        //PrivateMenuPopup*        m_private = nullptr;
    };
    // get meta info for UIMenuPopup
    LUI_DECLARE_METAINFO(UIMenuPopup);
}
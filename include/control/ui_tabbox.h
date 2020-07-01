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

#include "ui_control.h"
#include "ui_box.h"
//#include "../util/ui_double_click.h"

// ui namespace
namespace LongUI {
    // tabs
    class UITabs;
    // tabpanels 
    class UITabPanels;
    // tabbox container
    class UITabBox : public UIBoxLayout {
        // super class
        using Super = UIBoxLayout;
    public:
        // command: selected changed
        static constexpr auto _onCommand() noexcept { return GuiEvent::Event_OnCommand; }
    public:
        // set selected index
        void SetSelectedIndex(uint32_t index) noexcept;
        // get selected index
        auto GetSelectedIndex() const noexcept { return m_index; }
    protected:
        // ctor
        UITabBox(const MetaControl&) noexcept;
    public:
        // class meta
        static const  MetaControl   s_meta;
        // dtor
        ~UITabBox() noexcept;
        // ctor
        explicit UITabBox(UIControl* parent = nullptr) noexcept : UITabBox(UITabBox::s_meta) { this->final_ctor(parent); }
    public:
        // do normal event
        //auto DoEvent(UIControl* sender, const EventArg& e) noexcept->EventAccept override;
        // do mouse event
        //auto DoMouseEvent(const MouseEventArg& e) noexcept->EventAccept override;
        // update
        //void Update(UpdateReason) noexcept override;
        // render this control only, [Global rendering and Incremental rendering]
        //void Render() const noexcept override;
        // recreate/init device(gpu) resource
        //auto Recreate() noexcept->Result override;
    protected:
        // initialize
        void initialize() noexcept override;
        // add child
        void add_child(UIControl& child) noexcept override;
        // relayout
        //void relayout() noexcept ;
        // refresh fitting
        //void refresh_fitting() noexcept;
    protected:
        // tabs
        UITabs*                 m_pTabs = nullptr;
        // tabpanels
        UITabPanels*            m_pTabPanels = nullptr;
        // current selected index
        uint32_t                m_index = 0;
    private:
    };
    // get meta info for UITabBox
    LUI_DECLARE_METAINFO(UITabBox);
}

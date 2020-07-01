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

namespace LongUI {
    // scroll bar
    class UIScrollBar;
    // scroll area
    class UIScrollArea : public UIControl {
        // super class
        using Super = UIControl;
    protected:
        // base ctor
        UIScrollArea(const MetaControl&) noexcept;
    public:
        // meta info
        static const MetaControl    s_meta;
        // dtor
        ~UIScrollArea() noexcept override;
        // outer ctor
        explicit UIScrollArea(UIControl* parent = nullptr) noexcept : UIScrollArea(UIScrollArea::s_meta) { this->final_ctor(parent); }
        // do normal event
        auto DoEvent(UIControl* sender, const EventArg& arg) noexcept->EventAccept override;
        // do mouse event
        auto DoMouseEvent(const MouseEventArg& e) noexcept->EventAccept override;
        // Update
        //void Update(UpdateReason) noexcept override;
    public:
        // set auto overflow
        void SetAutoOverflow() noexcept;
        // add spacer
        void AddSpacer(Size2F size, float flex) noexcept;
        // get layout direcition
        auto GetLayoutDirection() const noexcept ->AttributeDir { return AttributeDir(m_state.direction); }
    protected:
        // layout the scroll bar
        auto layout_scroll_bar(/*Size2F cs*/) noexcept->Size2F;
        // synchronize the scroll bar
        void sync_scroll_bar(Point2F& offset) noexcept;
        // get layout position
        auto get_layout_position() const noexcept->Point2F;
        // add child
        void add_child(UIControl&) noexcept override;
    private:
        // on state dirty
        //void on_state_dirty() noexcept;
        // do wheel
        auto do_wheel(int index, float wheel) noexcept->EventAccept;
        // create scroll bar
        auto create_scrollbar(AttributeOrient) noexcept->UIScrollBar*;
        // layout the scroll bar
        auto layout_scrollbar(bool notenough, bool index) noexcept->float;
        // layout corner
        void layout_corner(bool visible, Size2F size) noexcept;
    public:
        // line size
        Size2F              line_size;
    protected:
        // min scroll size
        Size2F              m_minScrollSize;
        // max scroll size
        Size2F              m_maxScrollSize;
        // scroll bar - horizontal
        UIScrollBar*        m_pScrollBarHor = nullptr;
        // scroll bar - vertical
        UIScrollBar*        m_pScrollBarVer = nullptr;
        // corner
        UIControl*          m_pCorner = nullptr;
        // final end itr
        UIControl*          m_pFinalEnd;
    };
    // get meta info for UIScrollArea
    LUI_DECLARE_METAINFO(UIScrollArea);
}
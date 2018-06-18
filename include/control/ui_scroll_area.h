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

#include "ui_control.h"

namespace LongUI {
        // scroll bar
    class UIScrollBar;
    // scroll area
    class UIScrollArea : public UIControl {
        // super class
        using Super = UIControl;
    protected:
        // ctor
        UIScrollArea(UIControl* parent, const MetaControl&) noexcept;
    public:
        // meta info
        static const MetaControl    s_meta;
        // dtor
        ~UIScrollArea() noexcept;
        // ctor
        UIScrollArea(UIControl* parent = nullptr) noexcept : UIScrollArea(parent, UIScrollArea::s_meta) {}
        // do normal event
        auto DoEvent(UIControl* sender, const EventArg& arg) noexcept->EventAccept override;
        // do mouse event
        auto DoMouseEvent(const MouseEventArg& e) noexcept->EventAccept override;
        // update
        void Update() noexcept override;
    private:
        // [NEW] relayout
        virtual void relayout() noexcept;
    public:
        // set auto overflow
        void SetAutoOverflow() noexcept;
        // force update scroll size
        void ForceUpdateScrollSize(Size2F ss) noexcept;
        // add spacer
        void AddSpacer(Size2F size, float flex) noexcept;
        // get min scroll size
        auto GetMinScrollSize() const noexcept { return m_minScrollSize; }
        // get layout direcition
        auto GetLayoutDirection() const noexcept ->AttributeDir { return AttributeDir(m_state.dir); }
        // get vertical ScrollBar
        auto GetVerticalScrollBar() noexcept -> UIScrollBar* { return m_pVerticalSB; }
        // get horizontal ScrollBar
        auto GetHorizontalScrollBar() noexcept -> UIScrollBar* { return m_pHorizontalSB; }
        // get vertical ScrollBar | const overload
        auto GetVerticalScrollBar() const noexcept -> const UIScrollBar*{ return m_pVerticalSB; }
        // get horizontal ScrollBar | const overload
        auto GetHorizontalScrollBar() const noexcept -> const UIScrollBar*{ return m_pHorizontalSB; }
    protected:
        // get child flex sum
        auto sum_children_flex() const noexcept -> float;
        // synchronize the scroll bar
        void sync_scroll_bar() noexcept;
        // layout the scroll bar
        auto layout_scroll_bar() noexcept->Size2F;
        // get layout position
        auto get_layout_position() const noexcept->Point2F;
    private:
        // on state dirty
        void on_state_dirty() noexcept;
        // do wheel
        auto do_wheel(int index, float wheel) noexcept->EventAccept;
        // create scroll bar
        auto create_scrollbar(AttributeOrient) noexcept->UIScrollBar*;
        // layout the scroll bar - h
        auto layout_vscrollbar(bool notenough) noexcept->float;
        // layout the scroll bar - v
        auto layout_hscrollbar(bool notenough) noexcept->float;
    protected:
        // min scroll size
        Size2F              m_minScrollSize;
        // line size
        Size2F              m_szLine;
        // horizontal scroll bar
        UIScrollBar*        m_pHorizontalSB = nullptr;
        // vertical scroll bar
        UIScrollBar*        m_pVerticalSB = nullptr;
    };
    // get meta info for UIScrollArea
    LUI_DECLARE_METAINFO(UIScrollArea);
}
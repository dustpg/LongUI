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
// base
#include "ui_scale.h"
#include "ui_image.h"

namespace LongUI {
    // scroll bar
    class UIScrollBar final : public UIBoxLayout {
        // super class
        using Super = UIBoxLayout;
        // init bar
        void init_bar() noexcept;
    protected:
        // ctor
        UIScrollBar(AttributeOrient o, UIControl* parent, const MetaControl&) noexcept;
    public:
        // class meta
        static const  MetaControl   s_meta;
        // ctor
        UIScrollBar(AttributeOrient o, UIControl* parent = nullptr) noexcept : UIScrollBar(o, parent, UIScrollBar::s_meta) {}
        // ctor
        UIScrollBar(UIControl* parent) noexcept : UIScrollBar(Orient_Vertical, parent, UIScrollBar::s_meta) {}
        // dtor
        ~UIScrollBar() noexcept;
        // update
        void Update() noexcept override;
        // do normal event
        auto DoEvent(UIControl * sender, const EventArg & e) noexcept ->EventAccept override;
        // do mouse event
        auto DoMouseEvent(const MouseEventArg& e) noexcept->EventAccept override;
    public:
        // get value
        auto GetValue() const noexcept -> float;
    public:
        // set value
        void SetValue(float v) noexcept;
        // set max
        void SetMax(float v) noexcept;
        // set page increment
        void SetPageIncrement(float pi) noexcept;
        // set increment
        void SetIncrement(float pi) noexcept;
#ifdef LUI_ACCESSIBLE
    protected:
        // accessible event
        auto accessible(const AccessibleEventArg&) noexcept->EventAccept override;
#endif
    private: // private impl
        // up - top button
        UIImage         m_oUpTop;
        // down - top button
        UIImage         m_oDownTop;
        // slider
        UIScale         m_oSlider;
        // up - bottom button
        UIImage         m_oUpBottom;
        // down - bottom button
        UIImage         m_oDownBottom;
    };
    // get meta info for UIScrollBar
    LUI_DECLARE_METAINFO(UIScrollBar);
}
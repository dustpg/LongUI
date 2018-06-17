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

#include "ui_box_layout.h"
#include "../core/ui_window.h"

namespace LongUI {
    // viewport, logic window viewport
    class UIViewport : public UIBoxLayout {
        // super class
        using Super = UIBoxLayout;
        // friend window
        friend CUIWindow;
    public:
        // recreate/init device(gpu) resource
        auto Recreate() noexcept->Result override;
    protected:
        // add attribute
        void add_attribute(uint32_t key, U8View view) noexcept override;
    protected:
        // ctor for control
        UIViewport(UIControl* parent, CUIWindow::WindowConfig, const MetaControl&) noexcept;
    public:
        // ctor
        UIViewport(
            CUIWindow* parent = nullptr,
            CUIWindow::WindowConfig config = CUIWindow::Config_Default
        ) noexcept;
        // dtor
        ~UIViewport() noexcept;
        // ref the window
        auto&RefWindow() noexcept { return m_window; }
    private:
        // resize window
        void resize_window(Size2F size) noexcept;
    protected:
        // window
        CUIWindow           m_window;
    };
}
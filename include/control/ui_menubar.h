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

#include "ui_boxlayout.h"

// ui namespace
namespace LongUI {
    // menu
    class UIMenu;
    // menubar
    class UIMenuBar : public UIBoxLayout {
        // super class
        using Super = UIBoxLayout;
    public:
        // class meta
        static const  MetaControl   s_meta;
        // dtor
        ~UIMenuBar() noexcept;
        // ctor
        UIMenuBar(UIControl* parent = nullptr) noexcept : UIMenuBar(parent, UIMenuBar::s_meta) {}
    protected:
        // lui std ctor
        UIMenuBar(UIControl* parent, const MetaControl&) noexcept;
        // now popup menu
        UIMenu*             m_pPopupNow = nullptr;
    public:
        // has now menu?
        auto HasNowMenu(UIMenu& m) const noexcept { return m_pPopupNow && m_pPopupNow != &m; }
        // set now menu
        void SetNowMenu(UIMenu& m) noexcept { m_pPopupNow = &m; }
        // clear now menu
        void ClearNowMenu() noexcept { m_pPopupNow = nullptr; }
    };
    // get meta info for UIBoxLayout
    LUI_DECLARE_METAINFO(UIMenuBar);
}
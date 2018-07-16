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
#include "../core/ui_window.h"

namespace LongUI {
    // viewport, logic window viewport
    class UIViewport : public UIBoxLayout {
        // super class
        using Super = UIBoxLayout;
        // viewports
        using Viewports = POD::Vector<UIViewport*>;
        // friend window
        friend CUIWindow;
    public:
        // class meta
        static const  MetaControl   s_meta;
        // [NEW] on window closed
        virtual void WindowClosed() noexcept;
        // [NEW] on subview popup
        virtual void SubViewportPopupBegin(UIViewport&, PopupType) noexcept;
        // [NEW] on subview popup closed
        virtual void SubViewportPopupEnd(UIViewport&, PopupType) noexcept;
    protected:
        // add attribute
        void add_attribute(uint32_t key, U8View view) noexcept override;
        // ctor for control
        UIViewport(UIControl& pseudo_parent, CUIWindow::WindowConfig, const MetaControl&) noexcept;
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
        // get hoster
        auto GetHoster() const noexcept { return m_pHoster; }
        // assin new hoster
        void AssignNewHoster(UIControl& h) noexcept { m_pHoster = &h; }
        // PopupBegin to hoster
        void HosterPopupBegin() noexcept;
        // PopupEnd to hoster
        void HosterPopupEnd() noexcept;
        // add subviewport
        void AddSubViewport(UIViewport& sub) noexcept;
        // find subviewport with unique string
        auto FindSubViewportWithUnistr(const char*) const noexcept->UIViewport*;
        // find subviewport with normal string
        auto FindSubViewport(U8View view) const noexcept->UIViewport*;
        // just reset zoom
        void JustResetZoom(float x, float y) noexcept;
        // get real size(size * scale)
        auto GetRealSize() const noexcept { return m_szReal; }
        // Adjust size
        auto AdjustSize(Size2F) const noexcept->Size2L;
        // Adjust size
        auto AdjustZoomedSize(Size2F, Size2L) const noexcept->Size2L;
    private:
        // resize window
        void resize_window(Size2F size) noexcept;
    protected:
        // real size
        Size2F              m_szReal = {};
        // window
        CUIWindow           m_window;
        // last hoster, will set null after closed
        UIControl*          m_pHoster = nullptr;
        // sub-viewports
        Viewports           m_subviewports;
    };
    // get meta info for UIViewport
    LUI_DECLARE_METAINFO(UIViewport);
}
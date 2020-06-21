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

#include "ui_box.h"
#include "../core/ui_window.h"

namespace LongUI {
    // window manager
    class CUIWndMgr;
    // viewport, logic window viewport
    class UIViewport : public UIBoxLayout {
        // super class
        using Super = UIBoxLayout;
        // friend window
        friend CUIWindow; friend CUIWndMgr;
    public:
        // class meta
        static const  MetaControl   s_meta;
        // update
        //void Update(UpdateReason) noexcept override;
        // do event
        //auto DoEvent(UIControl* sender, const EventArg& e) noexcept->EventAccept override;
        // [NEW] on window closed
        virtual void WindowClosed() noexcept;
        // [NEW] on subview popup
        virtual void SubViewportPopupBegin(UIViewport&, PopupType) noexcept;
        // [NEW] on subview popup closed
        virtual void SubViewportPopupEnd(UIViewport&, PopupType) noexcept;
    protected:
        // initialize
        void initialize() noexcept override;
        // add attribute
        void add_attribute(uint32_t key, U8View view) noexcept override;
        // ctor for control
        UIViewport(
            const MetaControl& meta,
            UIControl& pseudo_parent,
            CUIWindow::WindowConfig config
        ) noexcept;
    public:
        // ctor
        UIViewport(
            CUIWindow* parent = nullptr,
            CUIWindow::WindowConfig config = CUIWindow::Config_Default,
            const MetaControl& meta = UIViewport::s_meta
        ) noexcept;
        // dtor
        ~UIViewport() noexcept override;
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
        // just reset zoom
        void JustResetZoom(float x, float y) noexcept;
        // get real size(size * scale)
        auto GetRealSize() const noexcept { return m_szReal; }
        // Adjust size
        auto AdjustSize(Size2F) const noexcept->Size2L;
        // Adjust size
        auto AdjustZoomedSize(Size2F, Size2L) const noexcept->Size2L;
    public:
        // find subviewport with unique string
        static auto FindSubViewport(UIControl*, const Node<UIControl>&, const char*) noexcept->UIViewport*;
        // find subviewport with normal string
        auto FindSubViewport(U8View view) const noexcept->UIViewport*;
        // find subviewport with  with UID-String
        auto FindSubViewportWithUID(const char* str) const noexcept {
            return UIViewport::FindSubViewport(m_nSubview.next, m_nSubview, str);
        }
    private:
        // resize window
        void resize_window(Size2F size) noexcept;
    protected:
        // real size
        Size2F              m_szReal = {};
        // last hoster, will set null after closed 
        UIControl*          m_pHoster = nullptr;
        // subview node
        Node<UIControl>     m_nSubview;
        // window
        CUIWindow           m_window;
    };
    // get meta info for UIViewport
    LUI_DECLARE_METAINFO(UIViewport);
}
#pragma once
/**
* Copyright (c) 2014-2016 dustpg   mailto:dustpg@gmail.com
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

// LongUI namespace
namespace LongUI {
    // base window
    class XUIBaseWindow;
    // system window
    class CUISystemWindow;
    // vector for window
    using WindowVector = EzContainer::PointerVector<XUIBaseWindow>;
    // vector for system window
    using SystemWindowVector = EzContainer::PointerVector<XUIBaseWindow>;
    // window for longui
    class XUIBaseWindow {
        // super class
        using Super = void;
    public:
        // string allocator
        using StringAllocator = CUIShortStringAllocator<>;
        // ctor
        XUIBaseWindow() noexcept;
        // dtor
        ~XUIBaseWindow() noexcept;
    public:
        // index of BitArray
        enum BitArrayIndex : uint32_t {
            // exit on close
            Index_ExitOnClose = 0,
            // close when focus killed
            Index_CloseOnFocusKilled,
            // caret in(true) or out?
            Index_CaretIn,
            // do caret?
            Index_DoCaret,
            // in draging?
            Index_InDraging,
            // window rendered in last time, or want to render in this time
            Index_Rendered,
            // new size?
            Index_NewSize,
            // skip render
            Index_SkipRender,
            // do full-render this frame?
            Index_FullRenderThisFrame,
            // count of this
            INDEX_COUNT,
        };
    public:
        // dispose
        virtual void Dispose() noexcept = 0;
        // render: call UIControl::Render
        virtual void Render() const noexcept;
        // update: call UIControl::Update
        virtual void Update() noexcept;
        // recreate: call UIControl::Render
        virtual auto Recreate() noexcept ->HRESULT;
        // link viewport
        virtual void LinkViewport(UIViewport* view) noexcept;
        // move window relative to parent
        virtual void MoveWindow(int32_t x, int32_t y) noexcept = 0;
        // resize window
        virtual void Resize(uint32_t w, uint32_t h) noexcept = 0;
        // set cursor
        virtual void SetCursor(LongUI::Cursor cursor) noexcept = 0;
    public:
        // reset cursor
        void ResetCursor() noexcept { this->SetCursor(Cursor::Cursor_Default); }
        // get window handle
        auto GetHwnd() const noexcept { return m_hwnd; }
        // get top
        auto GetTop() const noexcept { return m_rcWindow.top; }
        // get left
        auto GetLeft() const noexcept { return m_rcWindow.left; }
        // get right
        auto GetRight() const noexcept { return m_rcWindow.right; }
        // get bottom
        auto GetBottom() const noexcept { return m_rcWindow.bottom; }
        // show window
        void ShowWindow(int cmd) noexcept { ::ShowWindow(m_hwnd, cmd); };
        // get text anti-mode 
        auto GetTextAntimode() const noexcept { return static_cast<D2D1_TEXT_ANTIALIAS_MODE>(m_textAntiMode); }
        // get text anti-mode 
        void SetTextAntimode(D2D1_TEXT_ANTIALIAS_MODE mode) noexcept { m_textAntiMode = static_cast<decltype(m_textAntiMode)>(mode); }
        // is mouse captured control?
        auto IsCapturedControl(UIControl* c) noexcept { return m_pCapturedControl == c; };
        // is rendered
        auto IsRendered() const noexcept { return m_baBoolWindow.Test(XUIBaseWindow::Index_Rendered); }
        // copystring for control in this winddow
        auto CopyString(const char* str) noexcept { return m_oStringAllocator.CopyString(str); }
        // copystring for control in this winddow in safe way
        auto CopyStringSafe(const char* str) noexcept { auto s = this->CopyString(str); return s ? s : ""; }
        // render window in next frame
        void InvalidateWindow() noexcept { this->set_full_render_this_frame(); }
        // clear render info
        void ClearRenderInfo() noexcept { this->clear_full_render_this_frame(); m_uUnitLength = 0; }
    public:
        // do event
        bool DoEvent(const EventArgument& arg) noexcept;
        // render control in next frame
        void Invalidate(UIControl* ctrl) noexcept;
        // set the caret
        void SetCaretPos(UIControl* ctrl, float x, float y) noexcept;
        // create the caret
        void CreateCaret(UIControl* ctrl, float width, float height) noexcept;
        // show the caret
        void ShowCaret() noexcept;
        // hide the caret
        void HideCaret() noexcept;
        // set focus control
        void SetFocus(UIControl* ctrl) noexcept;
        // set hover track control
        void SetHoverTrack(UIControl* ctrl) noexcept;
        // find control
        auto FindControl(const char* name) noexcept ->UIControl*;
        // add control with name
        void AddNamedControl(UIControl* ctrl) noexcept;
        // set mouse capture
        void SetCapture(UIControl* control) noexcept;
        // release mouse capture
        void ReleaseCapture() noexcept;
    protected:
        // is SkipRender
        bool is_skip_render() const noexcept { return m_baBoolWindow.Test(Index_SkipRender); }
        // is CloseOnFocusKilled
        bool is_close_on_focus_killed() const noexcept { return m_baBoolWindow.Test(Index_CloseOnFocusKilled); }
        // is FullRenderingThisFrame
        bool is_full_render_this_frame() const noexcept { return m_baBoolWindow.Test(Index_FullRenderThisFrame); }
    protected:
        // set SkipRender to true
        void set_skip_render() noexcept { m_baBoolWindow.SetTrue(Index_SkipRender); }
        // clear SkipRender
        void clear_skip_render() noexcept { m_baBoolWindow.SetFalse(Index_SkipRender); }
        // set FullRenderingThisFrame to true
        void set_full_render_this_frame() noexcept { m_baBoolWindow.SetTrue(Index_FullRenderThisFrame); }
        // clear FullRenderingThisFrame
        void clear_full_render_this_frame() noexcept { m_baBoolWindow.SetFalse(Index_FullRenderThisFrame);  }
    protected:
        // implement longui-window
        UIViewport*             m_pImplement = nullptr;
        // parent window
        XUIBaseWindow*          m_pParent = nullptr;
        // children
        WindowVector            m_vChildren;
        // window handle
        HWND                    m_hwnd = nullptr;
        // TODO: mini size
        D2D1_SIZE_U             m_miniSize = D2D1::SizeU(64, 64);
        // now hover track control(only one)
        UIControl*              m_pHoverTracked = nullptr;
        // now focused control (only one)
        UIControl*              m_pFocusedControl = nullptr;
        // now dragdrop control (only one)
        UIControl*              m_pDragDropControl = nullptr;
        // now captured control (only one)
        UIControl*              m_pCapturedControl = nullptr;
        // window rect
        RectLTRB_L              m_rcWindow;
        // string allocator
        StringAllocator         m_oStringAllocator;
        // will use BitArray instead of them
        Helper::BitArray32      m_baBoolWindow;
        // mode for text anti-alias
        uint32_t                m_textAntiMode = D2D1_TEXT_ANTIALIAS_MODE_DEFAULT;
        // data length of m_apUnits
        size_t                  m_uUnitLength;
        // data for unit
        UIControl*              m_apUnit[LongUIDirtyControlSize];
        // dirty rects
        RECT                    m_dirtyRects[LongUIDirtyControlSize];
        // track mouse event: end with DWORD
        TRACKMOUSEEVENT         m_csTME;
        // current STGMEDIUM: begin with DWORD
        STGMEDIUM               m_curMedium;
        // control name ->map-> control pointer
        StringTable             m_hashStr2Ctrl;
    public:
        // debug info
#ifdef _DEBUG
        bool                    test_D2DERR_RECREATE_TARGET = false;
        bool                    debug_show = false;
        bool                    debug_unused[6];
        uint32_t                full_render_counter = 0;
        uint32_t                dirty_render_counter = 0;
#endif
        // last mouse point
        D2D1_POINT_2F           last_point = D2D1::Point2F(-1.f, -1.f);
        // clear color
        D2D1::ColorF            clear_color = D2D1::ColorF(D2D1::ColorF::White);
    };
    // system window
    class XUISystemWindow : public XUIBaseWindow {
        // super class
        using Super = XUIBaseWindow;
    public:
        // ctor
        XUISystemWindow() noexcept;
        // dtor
        ~XUISystemWindow() noexcept;
    public:
        // move window
        virtual void MoveWindow(int32_t x, int32_t y) noexcept override;
        // resize window
        virtual void Resize(uint32_t w, uint32_t h) noexcept override;
    public:

    protected:
        // message id for TaskbarBtnCreated
        static const UINT s_uTaskbarBtnCreatedMsg;
    };
    // create builtin system window
    auto CreateBuiltinSystemWindow() noexcept ->XUISystemWindow*;
    // create builtin inset window
    auto CreateBuiltinInsetWindow() noexcept ->XUIBaseWindow*;
}
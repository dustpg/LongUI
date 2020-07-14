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

#include "../util/ui_ostype.h"
#include "../graphics/ui_dcomp.h"
#include "ui_platform.h"

// Windows API
#define NOMINMAX
#include <Windows.h>

namespace LongUI {
    // viewport
    class UIViewport;
    // window
    class CUIWindow;
    // longui platform for windows
    class CUIPlatformWin final : public CUIPlatform {
        // register class
        static void register_class() noexcept;
        // using direct composition?
        bool is_direct_composition() const noexcept { return m_bDcompSupport; }
        // is skip render?
        bool is_skip_render() const noexcept { return m_bSystemSkipRendering; }
        // update adjust
        void update_adjust(uint32_t, uint32_t, uint32_t dpi) noexcept;
        // begin rendering
        void begin_render() noexcept;
        // end rendering
        auto end_render() noexcept ->Result;
        // resize window buffer
        void resize_window_buffer() noexcept;
        // dcomp
        using dcomp = impl::dcomp_window_buf;
    public:
        // win proc function on real
        static LRESULT WINAPI PrcoReal(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept;
        // win proc function on null
        static LRESULT WINAPI PrcoNull(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept;
        // destroy window
        static void Destroy(HWND) noexcept;
        // ctor
        CUIPlatformWin() noexcept;
        // dtor
        ~CUIPlatformWin() noexcept;
        // do msg
        auto DoMsg(HWND, UINT, WPARAM, LPARAM) noexcept->LRESULT;
        // when create
        void OnCreate(HWND) noexcept;
        // [thread safe] on resize
        void OnResizeTs(Size2U) noexcept;
        // on IME
        bool OnIme(HWND) const noexcept;
    public: // interface 
        // init
        void Init(CUIWindow* parent, uint16_t flag) noexcept;
        // render
        auto Render() noexcept->Result;
        // dispose
        void Dispose() noexcept;
        // release device data
        void ReleaseDeviceData() noexcept;
        // recreate
        auto Recreate() noexcept->Result;
        // after titlename set
        void AfterTitleName() noexcept;
        // after position set
        void AfterPosition() noexcept;
        // after absrect set
        void AfterAbsRect() noexcept;
        // close window
        void CloseWindow() noexcept;
        // show window
        void ShowWindow(int) noexcept;
        // resize
        void ResizeAbsolute(Size2L) noexcept;
        // work area that this window worked
        auto GetWorkArea() const noexcept->RectL;
        // get raw handle
        auto GetRawHandle() const noexcept { return reinterpret_cast<uintptr_t>(m_hWnd); }
        // enable the window
        void EnableWindow(bool enable) noexcept;
        // active window(the focus to window self)
        void ActiveWindow() noexcept;
    protected:
        // drop target impl
        uintptr_t           m_oDropTargetImpl = 0;
        // mouse track data
        TRACKMOUSEEVENT     m_oTrackMouse;
        // swap chian
        I::Swapchan*        m_pSwapchan = nullptr;
        // bitmap buffer
        I::Bitmap*          m_pBitmap = nullptr;
        // hwnd for this
        HWND                m_hWnd = nullptr;
        // dcomp support
        dcomp               m_dcomp;
        // window buffer logical size
        Size2L              m_szWndbufLogical = {};
    protected:
        // d2d text antialias
        uint16_t            m_uTextAntialias;
        // visible window
        //bool                m_bWindowVisible = false;
        // in creating
        bool                m_bInCreating = false;
        // ma return code
        uint8_t             m_uMaRevalue = 3;
        // sized
        bool                m_bFlagSized : 1;
        // mouse enter
        bool                m_bMouseEnter : 1;
        // dcomp support
        bool                m_bDcompSupport : 1;
        // accessibility called
        bool                m_bAccessibility : 1;
        // moving or resizing
        bool                m_bMovingResizing : 1;
        // mouse left down
        bool                m_bMouseLeftDown : 1;
        // skip window via system
        bool                m_bSystemSkipRendering : 1;
        // layered window support
        bool                m_bLayeredWindowSupport : 1;
    public:
#ifndef NDEBUG
        // full render counter
        uint32_t        dbg_full_render_counter = 0;
        // dirty render counter
        uint32_t        dbg_dirty_render_counter = 0;
#endif
    };
}

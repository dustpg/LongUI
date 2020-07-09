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
#include "../dropdrag/ui_dropdrag_impl.h"
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
    class CUIPlatformWin final : public CUIPlatform,
#ifndef LUI_NO_DROPDRAG
        public I::DropTarget
#endif
    {
#ifndef LUI_NO_DROPDRAG
    public: // IUnkown
        // add ref-count
        ULONG UNICALL AddRef() noexcept { return 2; };
        // release ref-count
        ULONG UNICALL Release() noexcept { return 1; };
        // query the interface
        HRESULT UNICALL QueryInterface(const IID&, void **ppvObject) noexcept override;
    public:
        // drag enter
        HRESULT UNICALL DragEnter(IDataObject*, DWORD, POINTL, DWORD*) noexcept override;
        // drag over
        HRESULT UNICALL DragOver(DWORD, POINTL, DWORD*) noexcept override;
        // drag leave
        HRESULT UNICALL DragLeave(void)  noexcept override;
        // drop
        HRESULT UNICALL Drop(IDataObject*, DWORD, POINTL, DWORD*) noexcept override;
    private:
#else
        // unused pointer
        void*           m_unused;
#endif
        // register class
        static void register_class() noexcept;
        // using direct composition?
        bool is_direct_composition() const noexcept { return m_bDcompSupport; }
        // full-rendering this frame?
        bool is_fr_for_update() const noexcept { return m_bFullRenderingUpdate; }
        // mark as full-rendering
        void mark_fr_for_update() noexcept { m_bFullRenderingUpdate = true; }
        // clear full-rendering 
        void clear_fr_for_update() noexcept { m_bFullRenderingUpdate = false; }
        // full-rendering this frame?
        bool is_fr_for_render() const noexcept { return dirty_count_presenting == DIRTY_RECT_COUNT + 1; }
        // will render in this frame?
        auto is_r_for_render() const noexcept { return dirty_count_presenting; }
        // mark as full-rendering
        void mark_fr_for_render() noexcept { dirty_count_presenting = DIRTY_RECT_COUNT + 1; }
        // mark as full-rendering
        void clear_fr_for_render() noexcept { dirty_count_presenting = 0; }
        // using direct composition?
        bool is_direct_composition() const noexcept { return m_bDcompSupport; }
        // is skip render?
        bool is_skip_render() const noexcept { return m_bSystemSkipRendering; }
    public:
        // win proc function on real
        static LRESULT WINAPI PrcoReal(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept;
        // win proc function on null
        static LRESULT WINAPI PrcoNull(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) noexcept;
        // ctor
        CUIPlatformWin() noexcept;
        // dtor
        ~CUIPlatformWin() noexcept;
        // init for this
        HWND Init(HWND, uint16_t flag) noexcept;
        // do msg
        auto DoMsg(HWND, UINT, WPARAM, LPARAM) noexcept->LRESULT;
        // when create
        void OnCreate(HWND) noexcept;
        // [thread safe] on resize
        void OnResizeTs(Size2U) noexcept;
        // on IME
        bool OnIme(HWND) const noexcept;
    public:
    protected:
        // mouse track data
        TRACKMOUSEEVENT     m_oTrackMouse;
    protected:
        // visible window
        bool                m_bWindowVisible = false;
        // full renderer in update
        bool                m_bFullRenderingUpdate = false;
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
    };
}

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

// basic type
#include "ui_basic_type.h"
#include "ui_core_type.h"
#include <type_traits>

// ui namespace
namespace LongUI {
    // basic dpi
    enum : uint32_t { BASIC_DPI = 96, };
    // priavte wndmgr
    struct PrivateWndMgr;
    // window list
    struct WindowVector;
    // detail namespace
    namespace detail {
        // private data for manager
        template<size_t> struct private_wndmgr;
        // 32bit
        template<> struct private_wndmgr<4> { enum { size = 56, align = 8 }; };
        // 64bit
        template<> struct private_wndmgr<8> { enum { size = 72, align = 8 }; };
    }
    // UI Window Manager
    class CUIWndMgr {
        // friend
        friend CUIWindow;
    public:
        // get main dpi x
        auto GetMainDpiX() const noexcept { return m_uMainDpiX; }
        // get main dpi y
        auto GetMainDpiY() const noexcept { return m_uMainDpiY; }
        // mark window minsize changed, donothing if null
        void MarkWindowMinsizeChanged(CUIWindow* window) noexcept;
        // get window list
        auto GetWindowList() const noexcept->const WindowVector&;
    protected:
        // close helper
        void close_helper(CUIWindow& wnd) noexcept;
        // add a window
        void add_window(CUIWindow&) noexcept;
        // remove a window
        void remove_window(CUIWindow&) noexcept;
        // before all windows 
        void before_render_windows() noexcept;
        // render all windows
        auto render_windows() noexcept->Result;
        // recreate_device all windows
        auto recreate_windows() noexcept->Result;
        // update delta time
        auto update_delta_time() noexcept -> float;
        // sleep for vblank
        void sleep_for_vblank() noexcept;
        // refresh window minsize
        void refresh_window_minsize() noexcept;
        // refresh control world in each window
        void refresh_window_world() noexcept;
    private:
        // is quit on last window closed
        static bool is_quit_on_last_window_closed() noexcept;
        // exit
        static void exit() noexcept;
    protected:
        // main monitor dpi x
        uint16_t                m_uMainDpiX = 96;
        // main monitor dpi y
        uint16_t                m_uMainDpiY = 96;
        // ununsed
        uint32_t                m_dwDisplayFrequency = 0;
        // vsync count
        uint32_t                m_dwWaitVSCount = 0;
        // vsync start time
        uint32_t                m_dwWaitVSStartTime = 0;
    private:
        // private data
        std::aligned_storage<
            detail::private_wndmgr<sizeof(void*)>::size,
            detail::private_wndmgr<sizeof(void*)>::align
        >::type                 m_private;
        // wm
        auto& wm() noexcept { return reinterpret_cast<PrivateWndMgr&>(m_private); }
        // wm
        auto& wm() const noexcept { return reinterpret_cast<const PrivateWndMgr&>(m_private); }
    protected:
        // refresh display frequency
        void refresh_display_frequency() noexcept;
        // ctor
        CUIWndMgr(Result& out) noexcept;
        // ctor
        CUIWndMgr(const CUIWndMgr&) noexcept = delete;
        // ctor
        CUIWndMgr(CUIWndMgr&&) noexcept = delete;
        // dtor
        ~CUIWndMgr() noexcept;
    };
}

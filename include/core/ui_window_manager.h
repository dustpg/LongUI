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

// basic type
#include "ui_basic_type.h"
#include "ui_core_type.h"
#include "ui_node.h"
#include <type_traits>

// ui namespace
namespace LongUI {
    // window list
    struct WindowVector;
    // detail namespace
    namespace detail {
        // private data for manager
        template<size_t> struct private_wndmgr;
        // 32bit
        template<> struct private_wndmgr<4> { enum { size = 40, align = 8 }; };
        // 64bit
        template<> struct private_wndmgr<8> { enum { size = 56, align = 8 }; };
    }
    // UI Window Manager
    class CUIWndMgr {
        // itr
        using Iterator = Node<CUIWindow>::Iterator;
        // friend
        friend CUIWindow;
        // private impl
        struct Private;
    public:
        // get main dpi x
        auto GetMainDpiX() const noexcept { return m_uMainDpiX; }
        // get main dpi y
        auto GetMainDpiY() const noexcept { return m_uMainDpiY; }
        // move subviewport to global
        void MoveSubViewToGlobal(UIViewport&) noexcept;
        // find subviewport with unique string
        auto FindSubViewportWithUnistr(const char*) const noexcept->UIViewport*;
        // mark window minsize changed, donothing if null
        static void MarkWindowMinsizeChanged(CUIWindow* window) noexcept;
    protected:
        // delete all window
        void delete_all_window() noexcept;
        // close helper
        void close_helper(CUIWindow& wnd) noexcept;
        // add a top-window
        void add_topwindow(CUIWindow&) noexcept;
        // add a window
        void add_to_allwindow(CUIWindow&) noexcept;
        // remove a window
        void remove_from_allwindow(CUIWindow&) noexcept;
        // update delta time
        auto update_delta_time() noexcept -> float;
        // sleep for vblank
        void sleep_for_vblank() noexcept;
        // refresh control world in all windows
        void refresh_window_world() noexcept;
        // refresh all given windows(and children) minsize
        void refresh_window_minsize() noexcept;
        // recreate_device all given windows(and children)
        static auto recreate_windows(Iterator, Iterator) noexcept->Result;
        // before all given windows(and children) render
        static void before_render_windows(Iterator, Iterator) noexcept;
        // render all given windows(and children)
        static auto render_windows(Iterator, Iterator) noexcept->Result;
    private:
        // is quit on last window closed
        static bool is_quit_on_last_window_closed() noexcept;
        // exit
        static void exit() noexcept;
    public:
        // top lv end iterator
        auto end()noexcept->Iterator;
        // top lv begin iterator
        auto begin()noexcept->Iterator;
    protected:
        // head
        Node<CUIWindow>         m_oHead;
        // tail
        Node<CUIWindow>         m_oTail;
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
        auto& wm() noexcept { return reinterpret_cast<Private&>(m_private); }
        // wm
        auto& wm() const noexcept { return reinterpret_cast<const Private&>(m_private); }
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

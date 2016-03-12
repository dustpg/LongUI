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
    // window
    class XUIBaseWindow {
    public:
        // ctor
        XUIBaseWindow() noexcept;
        // dtor
        ~XUIBaseWindow() noexcept;
        // do render
        void DoRender() const noexcept;
        // do update
        void DoUpdate() noexcept;
    public:
        // dispose
        virtual void Dispose() noexcept = 0;
        // move window
        virtual void MoveWindow(int32_t x, int32_t y) noexcept = 0;
        // resize window
        virtual void Resize(uint32_t w, uint32_t h) noexcept = 0;
    public:
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
    protected:
        // implement longui-window
        UIWindow*           m_pImplement = nullptr;
        // parent window
        XUIBaseWindow*      m_pParent = nullptr;
        // children
        WindowVector        m_vChildren;
        // window handle
        HWND                m_hwnd = nullptr;
        // window rect
        RectLTRB_L          m_rcWindow;
    };
    // system window
    class XUISystemWindow : public XUIBaseWindow {
        // super class
        using Super = XUIBaseWindow;
    public:
        // move window
        virtual void MoveWindow(int32_t x, int32_t y) noexcept override;
        // resize window
        virtual void Resize(uint32_t w, uint32_t h) noexcept override;
    public:
        // window proc
        virtual auto WndProc(UINT message, WPARAM wParam, LPARAM lParam) noexcept -> LRESULT = 0;
    protected:
    };
    // create builtin system window
    auto CreateBuiltinSystemWindow() noexcept ->XUISystemWindow*;
}
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
namespace LongUI { namespace Window {
    // window
    class Window {
    public:
        // ctor
        Window() noexcept;
        // dtor
        ~Window() noexcept;
        // do render
        void DoRender() noexcept;
    public:
        // dispose
        virtual void Dispose() noexcept = 0;
        // move window
        virtual void MoveWindow(int32_t x, int32_t y) noexcept = 0;
        // resize window
        virtual void Resize(uint32_t w, uint32_t h) noexcept = 0;
    public:
        // get window handle
        auto GetHwnd() const noexcept { m_hwnd; }
        // get top
        auto GetTop() const noexcept { return m_iTop; }
        // get left
        auto GetLeft() const noexcept { return m_iLeft; }
        // get right
        auto GetRight() const noexcept { return m_iRight; }
        // get bottom
        auto GetBottom() const noexcept { return m_iBottom; }
    protected:
        // implement longui-window
        UIWindow*           m_pImplement = nullptr;
        // parent window
        Window*             m_pParent = nullptr;
        // prev window
        Window*             m_pPrev = nullptr;
        // next window
        Window*             m_pNext = nullptr;
        // first child window
        Window*             m_pChild = nullptr;
        // window handle
        HWND                m_hwnd = nullptr;
        // left of window
        int32_t             m_iLeft = 0;
        // top of window
        int32_t             m_iTop = 0;
        // right of window
        int32_t             m_iRight = 0;
        // bottom of window
        int32_t             m_iBottom = 0;
    };
    // system window
    class System : public Window {
    public:
    protected:
    };
}}
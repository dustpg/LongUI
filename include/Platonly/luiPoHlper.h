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

#include <Windows.h>
#include <cstdint>

// longui::helper namespace
namespace LongUI { namespace Helper {
    // find files to buffer
    auto FindFilesToBuffer(wchar_t* buf, size_t buf_len, const wchar_t* folder, const wchar_t* name = L"*.*") noexcept -> wchar_t*;
    // GlobalAlloc a string data: char overload
    auto GlobalAllocString(const char* str, size_t len) noexcept ->HGLOBAL;
    // GlobalAlloc a string data: wchar_t overload
    auto GlobalAllocString(const wchar_t* str, size_t len) noexcept ->HGLOBAL;
#if 0
    // GlobalAlloc a string data: inline const wchar_t* overload
    inline auto GlobalAllocString(const wchar_t* str) noexcept {
        return GlobalAllocString(str, static_cast<size_t>(std::wcslen(str)));
    }
    // GlobalAlloc a string data: inline const char* overload
    inline auto GlobalAllocString(const char* str) noexcept {
        return GlobalAllocString(str, static_cast<size_t>(std::strlen(str)));
    }
#endif
    // double click helper
    /*
        Helper::DoubleClick leftdb(500);
        auto a = leftdb.Click(); // false
        ::Sleep(1000);
        auto b = leftdb.Click(); // false
        ::Sleep(50);
        auto c = leftdb.Click(); // true
    */
    struct DoubleClick {
        // ctor
        DoubleClick(uint32_t t = ::GetDoubleClickTime()) noexcept : time(t) {};
#ifdef LONGUI_DOUBLECLICK_WITH_POINT
        // click, return true if double clicked
        bool Click(float x, float y) noexcept;
#else
        // click, return true if double clicked
        bool Click() noexcept;
#endif
        // time
        uint32_t        time;
        // last click time
        uint32_t        last = 0;
#ifdef LONGUI_DOUBLECLICK_WITH_POINT
        // mouse point x
        float           ptx = -1.f;
        // mouse point y
        float           pty = -1.f;
#endif
    };
    // timer-helper for ui
    class Timer {
    public:
        // ctor
        Timer(uint32_t elapse) noexcept : m_dwTime(elapse) { }
        // update, return true if it is time
        bool Update() noexcept;
        // reset
        void Reset() noexcept { m_dwLastCount = ::timeGetTime(); }
        // reset
        void Reset(uint32_t elapse) noexcept { m_dwTime = elapse; this->Reset(); }
    private:
        // time for elapse
        uint32_t        m_dwTime;
        // last tick-count
        uint32_t        m_dwLastCount = ::timeGetTime();
    };
}}


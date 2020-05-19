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
#include "../core/ui_basic_type.h"
#include "../core/ui_core_type.h"
//#include "../util/ui_ostype.h"

#ifndef NDEBUG
#include "../filesystem/ui_file.h"
#define LUI_FRAMEID << LongUI::get_frame_id() << "_F "
namespace LongUI { auto get_frame_id() noexcept->uint32_t; }
#else 
#define LUI_FRAMEID
#endif

// helper marco
#define _lui_inter_debug LongUI::CUIDebug::GetInstance()
#ifndef NDEBUG
#define _lui_inter_extra << LongUI::Interfmt("<%4dL@%s>: ", int(__LINE__), __FUNCTION__)
#define longui_debug_hr(hr, msg) if (!hr) LUIDebug(Error) << msg << hr << LongUI::endl
#else
#define _lui_inter_extra
#define longui_debug_hr(hr, msg) (void)0
#endif
// debug marco
#define LUIDebug(lv) _lui_inter_debug << LongUI::DebugStringLevel::DLevel_##lv _lui_inter_extra


// ui namespace
namespace LongUI {
#ifndef NDEBUG
    // mouse event
    enum class MouseEvent : uint32_t;
    // color
    struct ColorF;
    // struct
    struct StyleStateTypeChange;
#endif
    // debug string level
    enum DebugStringLevel : uint32_t {
        // None level
        DLevel_None = 0,
        // level Log
        DLevel_Log,
        // level Hint
        DLevel_Hint,
        // level Warning
        DLevel_Warning,
        // level Error
        DLevel_Error,
        // level Fatal
        DLevel_Fatal,
        // level's size
        DLEVEL_SIZE
    };
    // endl for longUI
    struct EndL { }; extern EndL const endl;
    // debug
    class CUIDebug {
    public:
        // set unhandled exp handler
        static void InitUnExpHandler() noexcept;
        // output debug string
        void OutputString(
            DebugStringLevel level,
            const wchar_t* str,
            bool flush
        ) noexcept;
#ifdef NDEBUG
        // get debug instance
        static auto GetInstance() noexcept->CUIDebug& {
            return *static_cast<CUIDebug*>(nullptr);
    }
        // overload << operator 重载 << 运算符
        template<typename T>
        inline const CUIDebug& operator<< (const T&) const noexcept { return *this; }
        // output with wide char
        inline void Output(DebugStringLevel, const char16_t*) const noexcept { }
        // output with utf-8
        inline void Output(DebugStringLevel, const char*) const noexcept { }

#else
        // get debug instance
        static auto GetInstance() noexcept->CUIDebug&;
    protected:
        // last DebugStringLevel
        DebugStringLevel        m_lastLevel = DebugStringLevel::DLevel_Log;
        // time tick count
        uint32_t                m_timeTick = 0;
        // log file
        CUIFile                 m_logFile;
    public:
        // overload << operator for DebugStringLevel
        CUIDebug& operator<< (const DebugStringLevel l)  noexcept { m_lastLevel = l; return *this; }
        // overload << operator for float
        CUIDebug& operator<< (const float f) noexcept;
        // overload << operator for float.2
        CUIDebug& operator<< (const DDFFloat2 f) noexcept;
        // overload << operator for float.3
        CUIDebug& operator<< (const DDFFloat3 f) noexcept;
        // overload << operator for float.4
        CUIDebug& operator<< (const DDFFloat4 f) noexcept;
        // overload << operator for uint32_t
        CUIDebug& operator<< (const uint32_t o) noexcept;
        // overload << operator for int32_t
        CUIDebug& operator<< (const int32_t o) noexcept;
        // overload << operator for bool
        CUIDebug& operator<< (const bool b) noexcept;
        // overload << operator for void*
        CUIDebug& operator<< (const void*) noexcept;
        // overload << operator for control
        CUIDebug& operator<< (const UIControl*) noexcept;
        // overload << operator for control
        CUIDebug& operator<< (const UIControl& c) noexcept { return *this << &c; }
        // overload << operator for endl
        CUIDebug& operator<< (const LongUI::EndL&) noexcept;
        // overload << operator for Matrix
        CUIDebug& operator<< (const Matrix3X2F& m) noexcept;
        // overload << operator for DXGI_ADAPTER_DESC*
        CUIDebug& operator<< (const GraphicsAdapterDesc& d) noexcept;
        // overload << operator for ColorF
        CUIDebug& operator<< (const ColorF& r) noexcept;
        // overload << operator for RectF
        CUIDebug& operator<< (const RectF& r) noexcept;
        // overload << operator for RectF
        CUIDebug& operator<< (const RectWHU& r) noexcept;
        // overload << operator for Point2F
        CUIDebug& operator<< (const Point2F& p) noexcept;
        // overload << operator for Size2F
        CUIDebug& operator<< (const Size2U& p) noexcept;
        // overload << operator for Size2F
        CUIDebug& operator<< (const Size2F& p) noexcept;
        // overload << operator for const char16_t*
        CUIDebug& operator<< (const wchar_t* s) noexcept;
        // overload << operator for const char16_t*
        CUIDebug& operator<< (const char16_t* s) noexcept { this->OutputNoFlush(m_lastLevel, s); return *this; }
        // overload << operator for const char*
        CUIDebug& operator<< (const char* s) noexcept { this->OutputNoFlush(m_lastLevel, s); return *this; }
        // overload << operator for const char*
        CUIDebug& operator<< (ULID id) noexcept { return *this << id.id; }
        // overload << operator for char32_t
        CUIDebug& operator<< (const char32_t ch) noexcept;
        // overload << operator for char16_t
        CUIDebug& operator<< (const char16_t ch) noexcept { return (*this) << char32_t(ch); }
        // overload << operator for char
        CUIDebug& operator<< (const char ch) noexcept { return (*this) << char32_t(ch); }
        // overload << operator for char
        CUIDebug& operator<< (const wchar_t ch) noexcept { return (*this) << char32_t(ch); }
        // overload << operator for CUIString
        CUIDebug& operator<< (const CUIString& s) noexcept;
        // overload << operator for utf-8 string-view
        CUIDebug& operator<< (const U8View s) noexcept;
        // overload << operator for MouseEvent
        CUIDebug& operator<< (Result) noexcept;
        // overload << operator for MouseEvent
        CUIDebug& operator<< (MouseEvent e) noexcept;
        // overload << operator for MouseEvent
        CUIDebug& operator<< (StyleStateTypeChange e) noexcept;
        // output debug string with flush
        void Output(DebugStringLevel l, const char16_t* s) noexcept;
        // output debug string with flush
        void Output(DebugStringLevel l, const char* s) noexcept;
    private:
        // output debug (utf-8) string without flush
        void OutputNoFlush(DebugStringLevel l, const char* s) noexcept;
        // output debug string without flush
        void OutputNoFlush(DebugStringLevel l, const char16_t* s) noexcept;
    public:
#endif
    protected:
#ifdef NDEBUG
        // ctor
        CUIDebug() noexcept = default;
#else
        // ctor
        CUIDebug(const char16_t* file = u"") noexcept;
#endif
        // dtor
        ~CUIDebug() noexcept = default;
        // ctor
        CUIDebug(const CUIDebug&) noexcept = delete;
        // ctor
        CUIDebug(CUIDebug&&) noexcept = delete;
    };
    // formated buffer
#ifndef NDEBUG
    auto Formated(const char* format, ...) noexcept -> const char*;
    auto Interfmt(const char* format, ...) noexcept -> const char*;
#else
    static auto Formated(...) noexcept { return static_cast<const char*>(nullptr); }
#endif
}
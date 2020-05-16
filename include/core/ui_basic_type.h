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

#include <cstdint>

// longui namespace
namespace LongUI {
    /// <summary>
    /// Result code
    /// </summary>
    struct Result {
        // code
        std::int32_t    code;
        // Commonly used code list
        enum CommonResult : int32_t { 
            RS_OK           = (int32_t)0x00000000, // Operation successful
            RS_FALSE        = (int32_t)0x00000001, // Operation successful
            RE_NOTIMPL      = (int32_t)0x80004001, // Not implemented
            RE_NOINTERFACE  = (int32_t)0x80004002, // No such interface supported
            RE_POINTER      = (int32_t)0x80004003, // Pointer that is not valid
            RE_ABORT        = (int32_t)0x80004004, // Operation aborted
            RE_FAIL         = (int32_t)0x80004005, // Unspecified failure
            RE_FILENOTFOUND = (int32_t)0x80070002, // File not found
            RE_UNEXPECTED   = (int32_t)0x8000FFFF, // Unexpected failure
            RE_ACCESSDENIED = (int32_t)0x80070005, // General access denied error
            RE_HANDLE       = (int32_t)0x80070006, // Handle that is not valid
            RE_OUTOFMEMORY  = (int32_t)0x8007000E, // Failed to allocate necessary memory
            RE_INVALIDARG   = (int32_t)0x80070057, // One or more arguments are not valid
            RE_BUFFEROVER   = (int32_t)0x8007006f, // The file name is too long.
        };

        // operator bool
        operator bool() const noexcept { return code >= 0; }
        // operator !
        bool operator !() const noexcept { return code < 0; }
        // get system last error
        static auto GetSystemLastError() noexcept ->Result;
    };
    // basic dpi(100%)
    enum : uint32_t { BASIC_DPI = 96, };
    // Script define
    struct ScriptUI {
        // script data, maybe binary data maybe string
        const uint8_t*      script;
        // size of it
        size_t              size;
    };
    // point
    template<typename T> struct Point { T x; T y; };
    // rect
    template<typename T> struct Rect { T left; T top; T right; T bottom; };
    // size
    template<typename T> struct Size { T width; T height; };
    // Point2U
    using Point2U = Point<uint32_t>;
    // Point2F
    using Point2F = Point<float>;
    // Point2L
    using Point2L = Point<std::int32_t>;
    // Vector2F
    struct Vector2F { float x; float y; };
    // Vector3F
    struct Vector3F { float x; float y; float z; };
    // Vector4F
    struct Vector4F { float x; float y; float z; float w; };
    // rect w/h model
    template<typename T> struct RectWH { 
        T left; T top; T width; T height; 
        auto point() const noexcept { return Point<T>{ left, top }; }
        auto size() const noexcept { return Size<T>{ width, height }; }
        RectWH& operator=(Point<T> p) noexcept { left = p.x; top = p.y; return *this; }
        RectWH& operator=(Size<T> s) noexcept { width = s.width; height = s.height; return *this; }
    };
    // RectF
    using RectF = Rect<float>;
    // RectU
    using RectU = Rect<uint32_t>;
    // RectL
    using RectL = Rect<std::int32_t>;
    // RectWHF
    using RectWHF = RectWH<float>;
    // RectWHU
    using RectWHU = RectWH<uint32_t>;
    // RectWHL
    using RectWHL = RectWH<std::int32_t>;
    // Size2F
    using Size2F = Size<float>;
    // Size2U
    using Size2U = Size<uint32_t>;
    // Size2U
    using Size2L = Size<int32_t>;
    // Matrix3X2F
    struct Matrix3X2F { float _11, _12, _21, _22, _31, _32; };
    // Matrix4X4F
    struct Matrix4X4F { float m[4][4]; };
#ifndef NDEBUG
    // debug display format float: x.xx
    struct DDFFloat2 { float f; };
    // debug display format float: x.xxx
    struct DDFFloat3 { float f; };
    // debug display format float: x.xxxx
    struct DDFFloat4 { float f; };
    // get float1
    static inline auto Get2() noexcept ->DDFFloat2 { return{ 0.0f }; }
    // get float1
    static inline auto Get3() noexcept ->DDFFloat3 { return{ 0.0f }; }
    // get float1
    static inline auto Get4() noexcept ->DDFFloat4 { return{ 0.0f }; }
#endif
    // Ellipse
    struct Ellipse { Point2F point; float radius_x; float radius_y; };
    // transform point
    auto TransformPoint(const Matrix3X2F& matrix, Point2F point) noexcept->Point2F;
    // transform point - inverse
    auto TransformPointInverse(const Matrix3X2F& matrix, Point2F point) noexcept->Point2F;
    // is overlap?
    bool IsOverlap(const RectF& a, const RectF& b) noexcept;
    // is include?
    bool IsInclude(const RectF& a, const RectF& b) noexcept;
    // is include?
    bool IsInclude(const RectF& a, const Point2F& b) noexcept;
    // is same?
    bool IsSame(const RectF& a, const RectF& b) noexcept;
    // gui float same
    bool IsSameInGuiLevel(float a, float b) noexcept;
    // gui point same
    bool IsSameInGuiLevel(Point2F a, Point2F b) noexcept;
    // get area
    auto GetArea(const RectF& rect) noexcept -> float;
    // mix point
    auto Mix(Point2F from, Point2F to, float progress) noexcept ->Point2F;
    // mix rect
    auto Mix(const RectF& from, const RectF& to, float progress) noexcept;
    // round in gui level
    inline auto RoundInGuiLevel(float a) noexcept { return float(long(a + 0.5f)); }
    // round in gui level
    inline auto RoundInGuiLevel(double a) noexcept { return double(long(a + 0.5)); }
#if 0
    // a && b
    inline bool operator &&(const RectF& a, const RectF& b) noexcept {
        return LongUI::IsOverlap(a, b);
    }
    // a == b
    inline bool operator ==(const RectF& a, const RectF& b) noexcept {
        return LongUI::IsSame(a, b);
    }
    // a != b
    inline bool operator !=(const RectF& a, const RectF& b) noexcept {
        return LongUI::IsSame(a, b);
    }
#endif
    // gui size same
    inline bool IsSameInGuiLevel(const Size2F& a, const Size2F& b) noexcept {
        return IsSameInGuiLevel(
            reinterpret_cast<const Point2F&>(a),
            reinterpret_cast<const Point2F&>(b)
        );
    }
    // point + oparator
    template<typename T> auto operator +(Point<T> a, Point<T> b) noexcept {
        return Point<T>{ a.x + b.x, a.y + b.y };
    }
    // point - oparator
    template<typename T> auto operator -(Point<T> a, Point<T> b) noexcept {
        return Point<T>{ a.x - b.x, a.y - b.y };
    }
}
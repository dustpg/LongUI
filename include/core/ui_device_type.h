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

#include <cstdint>

// longui namespace
namespace UI {
    // result
    using Result = int32_t;
    // Script define
    struct ScriptUI {
        // script data, maybe binary data maybe string
        const unsigned char*  script;
        // size of it
        size_t                size;
    };
    // windows error code to HRESULT
    inline auto WinCode2HRESULT(uint32_t x) noexcept ->Result {
        constexpr Result T_FACILITY_WIN32 = 7;
        return ((Result)(x) <= 0 ? ((Result)(x)) : ((Result)(((x)& 0x0000FFFF) | (T_FACILITY_WIN32 << 16) | 0x80000000)));
    }
    // point
    template<typename T> struct Point { T x; T y; };
    // rect
    template<typename T> struct Rect { T left; T top; T right; T bottom; };
    // rect-ltwh
    template<typename T> struct RectWH { T left; T top; T width; T height; };
    // size
    template<typename T> struct Size { T width; T height; };
    // Point2U
    using Point2U = Point<uint32_t>;
    // Point2F
    using Point2F = Point<float>;
    // Point2L
    using Point2L = Point<int32_t>;
    // Vector2F
    struct Vector2F { float x; float y; };
    // Vector3F
    struct Vector3F { float x; float y; float z; };
    // Vector4F
    struct Vector4F { float x; float y; float z; float w;};
    // RectF
    using RectF = Rect<float>;
    // RectU
    using RectU = Rect<uint32_t>;
    // RectL
    using RectL = Rect<int32_t>;
    // RectWHL
    using RectWHL = RectWH<int32_t>;
    // RectWHF
    using RectWHF = RectWH<float>;
    // SizeF
    using SizeF = Size<float>;
    // SizeU
    using SizeU = Size<uint32_t>;
    // ColorF
    struct ColorF { float r; float g; float b; float a; };
    // Matrix3X2F
    struct Matrix3X2F { float _11, _12, _21, _22, _31, _32; };
    // Matrix4X4F
    struct Matrix4X4F { float m[4][4]; };
    // Ellipse
    struct Ellipse { Point2F point; float radius_x; float radius_y; };
}
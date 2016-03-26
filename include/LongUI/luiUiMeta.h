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

#include "../luibase.h"
#include <cstdint>
#include <d2d1_3.h>

#ifdef _MSC_VER
#pragma warning(disable: 4200)
#endif

// longui namespace
namespace LongUI {
    // Device Independent Meta
    struct DeviceIndependentMeta {
        // source rect
        D2D1_RECT_F         src_rect;
        // index for bitmap, 0 for custom
        uint32_t            bitmap_index;
        // render rule
        BitmapRenderRule    rule;
        //  interpolation mode
        uint16_t            interpolation;
    };
    // Meta(Bitmap Element)
    struct Meta : DeviceIndependentMeta {
        // bitmap
        ID2D1Bitmap1*       bitmap;
        // render this
        void Render(ID2D1DeviceContext*, const D2D1_RECT_F& des_rect, float opacity = 1.f) const noexcept;
    };
    // MetaEx: store a group of metas, like gif, must be stored as pointer(or ref)
    struct MetaEx {
        // unit
        struct Unit { Meta meta; float delta_time; };
        // the length of group
        size_t      length;
        // group of it("0" in C99, "1" for other)
        Unit        group[0];
    };
}
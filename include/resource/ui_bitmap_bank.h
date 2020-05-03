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

// ui
#include "../luiconf.h"
#include "../core/ui_object.h"
#include "../core/ui_basic_type.h"
#include "../container/pod_vector.h"
// image
#include "ui_image_res.h"
// c++
#include <cstdint>

// ui namespace
namespace LongUI {
    // bitmapbank window
    struct BitbankWindow {
        // bitmap used [NUL for not used]
        I::Bitmap*              bitmap;
        // last node
        BitmapFrame*            last;
        // line bottom
        uint32_t                top;
        // line bottom
        uint32_t                bottom;
        // area used
        uint32_t                area;
    };
    // bitmap bank
    class CUIBitmapBank final : public CUINoMo {
    public:
        // private impl
        struct Private;
        // ctor
        CUIBitmapBank() noexcept;
        // dtor
        ~CUIBitmapBank() noexcept;
        // ctor copy
        CUIBitmapBank(const CUIBitmapBank&) noexcept = delete;
        // ctor move
        CUIBitmapBank(CUIBitmapBank&&) noexcept = delete;
        // release all
        void ReleaseAll() noexcept;
        // recreate
        auto Recreate() noexcept->Result;
    public:
        // alloc rect
        auto Alloc(Size2U, BitmapFrame&) noexcept ->Result;
        // free rects
        void Free(BitmapFrame& f) noexcept;
    private:
        // window list, unit could be NUL
        POD::Vector<BitbankWindow>  m_window;
        // list real count
        uint32_t                    m_count = 0; 
    };
}
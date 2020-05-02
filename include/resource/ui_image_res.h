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
#include "ui_resource.h"
#include "../core/ui_basic_type.h"
#include "../graphics/ui_attribute.h"
#include "../graphics/ui_graphics_decl.h"


// c++
#include <cstdint>
#include <cassert>


// ui namespace
namespace LongUI {
    // detail img_ctor_dtor
    namespace detail { template<typename T> struct ctor_dtor; }
    // bitmapbank window
    struct BitbankWindow;
    // frame[prev/next may be null]
    struct BitmapFrame : Node<BitmapFrame> {
        // bitmap data
        I::Bitmap*              bitmap;
        // window data  [NUL for standalone]
        BitbankWindow*          window;
        // source data
        RectF                   source;
        // rect data
        RectWHU                 rect;
    };
    // Shared Image
    class CUIImage final : public CUISharedResource {
        // as friend
        friend struct detail::ctor_dtor<CUIImage>;
        // no move
        CUIImage(CUIImage&&) noexcept = delete;
        // mo copy
        CUIImage(const CUIImage&) noexcept = delete;
        // dtor
        ~CUIImage() noexcept { this->Release(); };
        // ctor
        CUIImage(uint32_t f, uint32_t d, Size2U) noexcept;
    public:
        // make error bitmap
        //static auto MakeError(I::Bitmap&) noexcept->Result;
        // create image object
        static auto Create(uint32_t f, uint32_t d, Size2U) noexcept ->CUIImage*;
        // release
        void Release() noexcept;
        // destroy object
        void Destroy() noexcept LUI_MULTRES_OVERRIDE;
    public:
        // ref frame
        auto&RefFrame(uint32_t id) const noexcept { return m_frames[id]; }
        // recreate bitmap
        //void RecreateBitmap(I::Bitmap&) noexcept;
        // render
        void Render(
            uint32_t frame_id,
            I::Renderer2D& renderer,
            const RectF* des_rect = nullptr,
            float opacity = 1.f,
            InterpolationMode mode = Mode_Linear
        ) const noexcept;
    public:
        // frame count [length of m_frames, must >= 1]
        uint32_t    const   frame_count;
        // frame delay
        uint32_t    const   delay;
        // size in float
        Size2F      const   size;
    private:
        // frame list
        BitmapFrame         m_frames[1];
    };
}
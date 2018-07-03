#pragma once

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
    // rgba
    union RGBA;
    // Shared Image
    class CUIImage : public CUISharedResource {
    protected:
        // private ctor
        CUIImage(I::Bitmap& bmp) noexcept : m_bitmap(&bmp) { assert(m_bitmap); }
        // private dtor
        inline ~CUIImage() noexcept;
    public:
        // destroy object
        void Destroy() noexcept override;
        // create image
        static auto CreateImage(
            I::Bitmap& bmp,
            CUISharedResource*& ptr
        ) noexcept->Result;
        // make error bitmap
        static auto MakeError(I::Bitmap&) noexcept->Result;
    public:
        // get size
        auto GetSize() const noexcept { return m_size; }
        // get bitmap
        auto&RefBitmap() const noexcept { return *m_bitmap; }
        // recreate bitmap
        void RecreateBitmap(I::Bitmap&) noexcept;
    public:
        // render
        void Render(
            I::Renderer2D& renderer,
            const RectF* des_rect = nullptr,
            const RectF* src_rect = nullptr,
            float opacity = 1.f,
            InterpolationMode mode = Mode_Linear
        ) const noexcept;
    private:
        // bitmap data
        I::Bitmap*          m_bitmap;
        // size of bitmap
        Size2U              m_size;
    };
}
#pragma once

// ui
#include "../core/ui_basic_type.h"
#include "../core/ui_object.h"
#include "ui_graphics_decl.h"

// ui namespace
namespace LongUI {
    // native geometry
    namespace I { struct GeometryEx; }
    // geometry object
    class CUIGeometry final : public CUINoMo {
    public:
        // ctor
        CUIGeometry() noexcept {}
        // dtor
        ~CUIGeometry() noexcept { this->release(); }
        // steal from native interface
        auto StealFrom(I::Geometry* p) noexcept->Result;
        // draw render
        void Draw(I::Renderer2D&, I::Brush&, const Matrix3X2F&, float width) noexcept;
        // fill render
        void Fill(I::Renderer2D&, I::Brush&, const Matrix3X2F&) noexcept;
        // recreate
        auto Recreate() noexcept ->Result;
        // create from points
        static auto CreateFromPoints(CUIGeometry& obj, 
                                     const Point2F[], 
                                     uint32_t count) noexcept ->Result;
    private:
        // release
        void release() noexcept;
    private:
        // native interface - raw ptr
        I::Geometry*        m_pDawPtr = nullptr;
        // native interface - realization
        I::GeometryEx*      m_pRealization = nullptr;
    };
}
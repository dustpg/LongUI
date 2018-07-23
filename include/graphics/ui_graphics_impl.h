#pragma once

#include "../core/ui_basic_type.h"

#include "../core/ui_color.h"
#include "ui_graphics_decl.h"
#include "ui_attribute.h"
#include "util/ui_unimacro.h"

// D2D
#include <d2d1_1.h>
// D3D
#include <d3d11.h>
// DXGI
#include <dxgi1_2.h>

// LongUI::i namespace
namespace LongUI { namespace I {
    // 2d renderer
    struct PCN_NOVTABLE Renderer2D : ID2D1DeviceContext { };
    // bitmap
    struct PCN_NOVTABLE Bitmap : ID2D1Bitmap1 {};
    // brush
    struct PCN_NOVTABLE Brush : ID2D1Brush {};
    // effect
    struct PCN_NOVTABLE Effect : ID2D1Effect {};
    // Swap
    struct PCN_NOVTABLE Swapchan : IDXGISwapChain1 {};
    // 3d device
    struct PCN_NOVTABLE Device3D : ID3D11Device {};
    // 3d renderer
    struct PCN_NOVTABLE Renderer3D : ID3D11DeviceContext {};
    // graphics
    struct PCN_NOVTABLE FactoryGraphics : IDXGIFactory2 {};
}}

// ui namespace
namespace LongUI {
    // safe release
    template<class T> inline void SafeRelease(T *& pi) {
        if (pi) { pi->Release(); pi = nullptr; }
    }
    // safe acquire
    template<class T> inline auto SafeAcquire(T * pi) {
        if (pi)  pi->AddRef(); return pi;
    }
}


// ui auto cast
namespace LongUI {
    // ui auto cast: size 2u
    inline auto auto_cast(Size2U f) noexcept {
        return D2D1_SIZE_U{ f.width, f.height };
    }

    // ui auto cast: const matrix&
    inline auto&auto_cast(const Matrix3X2F& f) noexcept {
        auto& t = reinterpret_cast<const D2D1_MATRIX_3X2_F&>(f);
        static_assert(sizeof(f) == sizeof(t), "must be same");
        return t;
    }

    // ui auto cast: matrix&
    inline auto&auto_cast(Matrix3X2F& f) noexcept {
        auto& t = reinterpret_cast<D2D1_MATRIX_3X2_F&>(f);
        static_assert(sizeof(f) == sizeof(t), "must be same");
        return t;
    }
    // ui auto cast: rect&
    inline auto&auto_cast(const RectF& f) noexcept {
        auto& t = reinterpret_cast<const D2D1_RECT_F&>(f);
        static_assert(sizeof(f) == sizeof(t), "must be same");
        return t;
    }
    // ui auto cast: rect&
    inline auto&auto_cast(const RectL& l) noexcept {
        auto& t = reinterpret_cast<const D2D1_RECT_L&>(l);
        static_assert(sizeof(l) == sizeof(t), "must be same");
        return t;
    }
    // ui auto cast: ellipse&
    inline auto&auto_cast(const Ellipse& f) noexcept {
        auto& t = reinterpret_cast<const D2D1_ELLIPSE&>(f);
        static_assert(sizeof(f) == sizeof(t), "must be same");
        return t;
    }
    // ui auto cast: color&
    inline auto&auto_cast(const ColorF& f) noexcept {
        auto& t = reinterpret_cast<const D2D1_COLOR_F&>(f);
        static_assert(sizeof(f) == sizeof(t), "must be same");
        return t;
    }
    // ui auto cast: color&
    inline auto&auto_cast(D2D1_RECT_F& r) noexcept {
        auto& t = reinterpret_cast<RectF&>(r);
        static_assert(sizeof(r) == sizeof(t), "must be same");
        return t;
    }
    // ui auto cast: point&
    inline auto&auto_cast(const Point2F& f) noexcept {
        auto& t = reinterpret_cast<const D2D1_POINT_2F&>(f);
        static_assert(sizeof(f) == sizeof(t), "must be same");
        return t;
    }

    // ui auto cast: matrix*
    inline auto auto_cast(const Matrix3X2F* f) noexcept {
        return &auto_cast(*f);
    }
    // ui auto cast: ellipse*
    inline auto auto_cast(const Ellipse *f) noexcept {
        return &auto_cast(*f);
    }
    // ui auto cast: RectF*
    inline auto auto_cast(const RectF* f) noexcept {
        return &auto_cast(*f);
    }
    // ui auto cast: color*
    inline auto auto_cast(const ColorF* f) noexcept {
        return &auto_cast(*f);
    }
    // ui auto cast: point*
    inline auto auto_cast(const Point2F* f) noexcept {
        return &auto_cast(*f);
    }
    
    // ui auto cast: inter mode
    inline auto auto_cast(InterpolationMode f) noexcept {
        return static_cast<D2D1_INTERPOLATION_MODE>(f);
    }
}

// auto cast
using LongUI::auto_cast;

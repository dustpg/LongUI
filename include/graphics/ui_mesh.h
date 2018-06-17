#pragma once

// ui
#include "../core/ui_basic_type.h"
#include "../core/ui_object.h"
#include "ui_graphics_decl.h"

// ui namespace
namespace LongUI {
    // native geometry
    namespace I { struct Mesh; }
    // 2d triangle
    struct TriangleF {
        // point 1-3
        Point2F     a, b, c;
    };
    // create mesh from triangles
    auto CreateMesh(I::Mesh*&, const TriangleF[], uint32_t len) noexcept->Result;
    // fill mesh
    void FillMesh(I::Mesh&, I::Renderer2D&, I::Brush&, const Matrix3X2F&) noexcept;
    // releash mesh
    void SafeReleaseMesh(I::Mesh*) noexcept;
}

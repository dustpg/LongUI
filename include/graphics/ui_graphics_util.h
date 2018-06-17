#pragma once

// ui
#include "ui_graphics_decl.h"
#include "../core/ui_basic_type.h"

namespace LongUI {
    // fill rect with std brush
    void FillRectStdBrush(
        I::Renderer2D& renderer,
        I::Brush& brush,
        const RectF& rect,
        float opactiy
    ) noexcept;
}
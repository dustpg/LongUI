#pragma once

// ui
#include "../luiconf.h"
#ifndef LUI_DISABLE_STYLE_SUPPORT
#include "ui_renderer_decl.h"
#include "../core/ui_color.h"
#include "../core/ui_object.h"
#include "../style/ui_attribute.h"

namespace LongUI {
    // border renderer
    class CUIRendererBorder : public CUISmallObject {
    public:
        // ctor
        CUIRendererBorder() noexcept;
        // dtor
        ~CUIRendererBorder() noexcept;
        // move ctor
        CUIRendererBorder(CUIRendererBorder&&) = delete;
        // copy ctor
        CUIRendererBorder(const CUIRendererBorder&) = delete;
        // render border
        void RenderBorder(const Box& box) const noexcept;
    public:
    public:
        // ------------- GPU-RES ------------
    public:
        // ------------- CPU-RES ------------
        // rect for image slice
        RectF               slice_rect = {};
        // resource for image
        uint32_t            image_id = 0;
        // fill for image slice
        bool                slice_fill = false;
    };
}
#endif
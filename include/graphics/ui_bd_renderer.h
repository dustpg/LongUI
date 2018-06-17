#pragma once

// ui
#include "ui_renderer_decl.h"
#include "../core/ui_color.h"
#include "../core/ui_object.h"
#include "../style/ui_attribute.h"

namespace LongUI {
    // box
    struct Box;
    // background renderer
    class CUIBorderRender : public CUISmallObject {
    public:
        // ctor
        CUIBorderRender() noexcept;
        // dtor
        ~CUIBorderRender() noexcept;
        // move ctor
        CUIBorderRender(CUIBorderRender&&) = delete;
        // copy ctor
        CUIBorderRender(const CUIBorderRender&) = delete;
        // render border
        void RenderBorder(const Box& box) const noexcept;
    public:
    public:
        // ------------- GPU-RES ------------
    private:
        // ------------- CPU-RES ------------
        ColorF              m_unused;
    };
}
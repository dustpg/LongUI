#pragma once

#include "../core/ui_color.h"
#include "../core/ui_object.h"
#include "../text/ui_ctl_decl.h"
#include "../core/ui_basic_type.h"
#include "../graphics/ui_graphics_decl.h"

// ui namespace
namespace LongUI {
    // text basic context
    struct TextBasicContext {
        // 2d render
        I::Renderer2D*  renderer;
        // text color
        ColorF          color;
    };
    // text outline context
    struct TextOutlineContext : TextBasicContext {
        // outline-color
        ColorF          outline_color;
        // outline-width
        float           outline_width;
    };
    // text outline renderer
    class CUITextOutline : public CUINoMo {
    public:
        // ctor
        CUITextOutline() noexcept;
        // dtor
        ~CUITextOutline() noexcept;
        // render text layout
        void Render(
            const TextOutlineContext& ctx,
            I::Text* text,
            Point2F point
        ) const noexcept;
    private:
        // ctl text renderer buffer
        void*           m_buffer;
    };
}
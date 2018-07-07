#pragma once

#include "../core/ui_object.h"
#include "../text/ui_ctl_decl.h"
#include "../core/ui_basic_type.h"
#include "../graphics/ui_graphics_decl.h"

// ui namespace
namespace LongUI {
    // color
    struct ColorF;
    // text arg
    struct TextArg;
    // static text layout
    class CUITextLayout : public CUINoMo {
        // set text
        auto set_text(TextArg& arg) noexcept->Result;
    public:
        // ctor
        CUITextLayout() noexcept {}
        // dtor
        ~CUITextLayout() noexcept;
        // renderer
        void Render(
            I::Renderer2D& renderer, 
            const ColorF& color,
            Point2F point
        ) const noexcept;
    public:
        // set font data
        auto SetFont(const FontArg& arg, const wchar_t* str, size_t len) noexcept->Result;
        // set text
        auto SetText(const wchar_t* str, size_t len) noexcept->Result;
        // resize
        void Resize(Size2F) noexcept;
        // test text size
        auto GetSize() const noexcept->Size2F;
        // set underline
        void SetUnderline(uint32_t pos, uint32_t len, bool) noexcept;
    public:
        // ok
        bool IsOK() const noexcept { return !!m_text; }
        // bool
        operator bool() const noexcept { return !!m_text; }
        // operator !
        bool operator!() const noexcept { return !m_text; }
    private:
        // ctl text data
        I::Text*            m_text = nullptr;
        // half line spacing
        //float               m_halfls = 0.f;
#ifndef NDEBUG
        // text setted
        bool                m_dbgTexted = false;
#endif
    };
}
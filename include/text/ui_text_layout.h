#pragma once

#include "../core/ui_object.h"
#include "../text/ui_ctl_decl.h"
#include "../core/ui_basic_type.h"
#include "../graphics/ui_graphics_decl.h"

// ui namespace
namespace LongUI {
    // color
    struct ColorF;
    // text font
    struct TextFont;
    // crop rule
    enum AttributeCrop : uint8_t;
    // static text layout
    class CUITextLayout : public CUINoMo {
        // set text
        auto set_text(TextArg& arg) noexcept->Result;
    public:
        // ctor
        CUITextLayout() noexcept = default;
        // dtor
        ~CUITextLayout() noexcept;
        // renderer with default text render
        void Render(
            I::Renderer2D& renderer, 
            const ColorF& color,
            Point2F point
        ) const noexcept;
    public:
        // set font data
        auto SetFont(const TextFont& arg, const char16_t* str, size_t len) noexcept->Result;
        // set text
        auto SetText(const char16_t* str, size_t len) noexcept->Result;
        // resize
        void Resize(Size2F) noexcept;
        // test text size
        auto GetSize() const noexcept->Size2F;
        // set underline
        void SetUnderline(uint32_t pos, uint32_t len, bool) noexcept;
        // set cropping rule
        void SetCropRule(AttributeCrop) noexcept;
    public:
        // ok
        bool IsOK() const noexcept { return !!m_text; }
        // bool
        operator bool() const noexcept { return !!m_text; }
        // operator !
        bool operator!() const noexcept { return !m_text; }
        // get ctl text
        auto GetCtlText() const noexcept { return m_text; }
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
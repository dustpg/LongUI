#pragma once

// decl
#include "ui_ctl_decl.h"
#include "ui_attribute.h"
#include <util/ui_unimacro.h>

// DWrite
#include <dwrite_1.h>

// LongUI::i namespace
namespace LongUI { namespace I {
    // CTL Factory
    struct PCN_NOVTABLE FactoryCTL : IDWriteFactory1 { };
    // CTL Font
    struct PCN_NOVTABLE Font : IDWriteTextFormat { };
    // CTL Text
    struct PCN_NOVTABLE Text : IDWriteTextLayout1 { };
    // CTL Text
    struct PCN_NOVTABLE TextRenderer : IDWriteTextRenderer { };
    // font from text
    inline auto FontFromText(Text* text) noexcept {
        const auto fmt = static_cast<IDWriteTextFormat*>(text);
        return static_cast<Font*>(fmt);
    }
    // font from text
    inline auto&FontFromText(Text& text) noexcept { 
        auto&fmt = static_cast<IDWriteTextFormat&>(text);
        return static_cast<Font&>(fmt);
    }
}}

// ui namespace
namespace LongUI {
    // auto cast font-weight
    inline auto auto_cast(AttributeFontWeight w) noexcept {
        return static_cast<DWRITE_FONT_WEIGHT>(w);
    }
    // auto cast font-weight
    inline auto auto_cast(AttributeFontStyle s) noexcept {
        return static_cast<DWRITE_FONT_STYLE>(s);
    }
    // auto cast font-stretch
    inline auto auto_cast(AttributeFontStretch s) noexcept {
        return static_cast<DWRITE_FONT_STRETCH>(s);
    }
    
}

// auto_cast to top namespace
using LongUI::auto_cast;

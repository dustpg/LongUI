#pragma once

// decl
#include "ui_ctl_decl.h"
#include "ui_attribute.h"

// ui namespace
namespace LongUI {
    // text arg
    struct TextArg {
        // font, null for default
        I::Font*        font;
        // string pointer, be carefual about dangling pointer
        const char16_t* string;
        // string length
        size_t          length;
        // max width
        float           mwidth;
        // max height
        float           mheight;
    };
    // font arg
    struct FontArg {
        // [layout]font family, maybe use CUIManager::GetUniqueText to create release-free text
        const char*             family;
        // [layout]font size
        float                   size;
        // [layout]line height * 
        float                   line_height_multi;
        // [layout]line height + 
        float                   line_height_plus;
        // [layout]weight
        AttributeFontWeight     weight;
        // [layout]style
        AttributeFontStyle      style;
        // [layout]stretch 
        AttributeFontStretch    stretch;
    };
    // get line height
    inline auto GetLineHeight(const FontArg& fa) noexcept {
        return fa.size * fa.line_height_multi + fa.line_height_plus;
    }
}

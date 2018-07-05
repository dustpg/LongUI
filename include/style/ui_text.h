#pragma once

// ui
#include "../core/ui_color.h"
#include "../text/ui_ctl_arg.h"

// ui namepsace
namespace LongUI {
    // style.text
    struct StyleText {
        // text color
        ColorF          color;
        // text crop

    };
    // style.text/font
    struct TextFont {
        // text data
        StyleText       text;
        // font data
        FontArg         font;
    };
    // default 
    void MakeDefault(TextFont&) noexcept;
}
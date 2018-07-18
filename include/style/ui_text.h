#pragma once

// ui
#include "../core/ui_color.h"
#include "../text/ui_ctl_arg.h"

// ui namepsace
namespace LongUI {
    // style.text
    struct StyleText {
        // [display]text color
        ColorF          color;
        // [display]text stroke color
        ColorF          stroke_color;
        // [display]text stroke width
        float           stroke_width;
        // [display]text unused float32
        float           text_unused;
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
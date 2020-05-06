#pragma once

#include "../luiconf.h"
#include "../core/ui_basic_type.h"

// longui::effect namespace
namespace LongUI { namespace Effect {
    // release common data
    void ReleaseBackImage() noexcept;
    // register back image
    auto RegisterBackImage(void* factory) noexcept->Result;
    // back image Zone Matrix
    struct BackImageMatrix {
        // source
        RectF               source;
        // output
        Size2F              rounds;
        // output
        Size2F              output;
        // output
        Size2F              space;
        // space flag
        uint32_t            flag[2];
    };
}}


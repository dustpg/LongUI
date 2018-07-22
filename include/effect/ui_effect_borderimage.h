#pragma once

#include "../luiconf.h"
#include "../core/ui_basic_type.h"

// longui::effect namespace
namespace LongUI { namespace Effect {
    // register border image
    auto RegisterBorderImage(void* factory) noexcept->Result;
    // border image cbuffer
    struct CBufferBorderImage {
        // border-ratio
        RectF       border_ratio; // { 0.25f, 0.25f, 0.75f, 0.75f };
        // slice-ratio
        RectF       slice_ratio;  // { 0.3333f, 0.3333f, 0.3333f, 0.3333f };
        // repeat data 
        RectF       repeat; // { 4.f, 1.5f, 4.f, 1.5f };
        // draw.size
        Size2F      size; // { 200.f, 200.f };
        // center alpha
        float       center_alpha; // 0.f 1.f
        // unused
        float       unused;
    };
}}


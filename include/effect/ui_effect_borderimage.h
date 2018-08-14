#pragma once

#include "../luiconf.h"
#include "../core/ui_basic_type.h"

// longui::effect namespace
namespace LongUI { namespace Effect {
    // value index
    enum IndexBorderImage : uint32_t {
        BImage_Matrix,
        BImage_Draw,

        VERTEX_FULLCOUNT = 9 * (2 * 3),
        VERTEX_NOFILLCOUNT = 8 * (2 * 3),
    };
    // release common data
    void ReleaseBorderImage() noexcept;
    // register border image
    auto RegisterBorderImage(void* factory) noexcept->Result;
    // border image Zone Matrix
    struct BorderImageMatrix {
        // zone matrix: zone0 part
        RectF           zone0; // { 81, 81, 0.f, 0.f };
        // zone matrix: zone1 part
        RectF           zone1; // { 1280 - 81 - 81, 720 - 81 - 81, 0.3333f, 0.3333f };
        // zone matrix: zone2 part
        RectF           zone2; // { 81, 81, 0.6666f, 0.6666f };
        // zone matrix: zone3 part
        RectF           zone3; // { 1280, 720, 1.f, 1.f };
        // repeat
        RectF           repeat; // { 4, 3.5f, 2.1f, 1.5f };
    };
}}


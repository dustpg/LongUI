#pragma once

#include <cstdint>

// ui namespace
namespace LongUI { 
    // Graphics Adapter Desc
    struct GraphicsAdapterDesc {
        // friend name
        const wchar_t*      friend_name;
        // shared memory
        size_t              shared_system;
        // dedicated video memory
        size_t              dedicated_video;
        // dedicated system memory
        size_t              dedicated_system;
    };
}
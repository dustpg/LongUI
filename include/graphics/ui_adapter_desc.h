#pragma once

#include <cstdint>

// ui namespace
namespace LongUI { 
    // Graphics Adapter Desc
    struct GraphicsAdapterDesc {
        // shared memory
        size_t              shared_system;
        // dedicated video memory
        size_t              dedicated_video;
        // dedicated system memory
        size_t              dedicated_system;
        // friend name
        char16_t            friend_name[128];
    };
}
#pragma once

#include <cstdint>

// ui namespace
namespace LongUI {
    // interpolation mode
    enum InterpolationMode : uint8_t {
        // Nearest Neighbor
        Mode_NearestNeighbor    = 0,
        // Linear
        Mode_Linear             = 1,
        // Cubic
        Mode_Cubic              = 2,
        // Multi Sample Linear
        Mode_MultiSampleLinear  = 3,
        // Anisotropic
        Mode_Anisotropic        = 4,
        // High Quality Cubic
        Mode_HighQualityCubic   = 5,
        // Fant
        Mode_Fant               = 6,
        // MipmapLinear
        Mode_MipmapLinear       = 7
    };
}

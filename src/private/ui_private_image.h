#pragma once

// ui
#include <resource/ui_image.h>

// ui namespace
namespace LongUI {
    // default image
    class CUIImage0 final : public CUIImage {
    public:
        // create image index0
        static auto CreateImage0(
            CUISharedResource*& ptr
        ) noexcept->Result;
        // destroy object
        void Destroy() noexcept override;
        // real destroy
        void RealDestroy() noexcept { delete this; }
    };
}

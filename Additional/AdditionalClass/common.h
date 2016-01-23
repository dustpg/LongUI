#pragma once
#include "../../include/LongUI.h"

// longui::additional namespace
namespace LongUI { namespace Additional {
    // config for this dll
    struct Config {
        // ctor
        Config() noexcept = default;
        // longui manager
        CUIManager* manager = nullptr;
        // alloc function
        void* (*alloc) (size_t) = nullptr;
        // free function
        void (*free) (void*) = nullptr;
    };
    // config
    extern struct Additional::Config g_config;
    // config alloc
    inline auto Alloc(size_t size) noexcept {
        assert(g_config.alloc && "set config first");
        return g_config.alloc(size);
    }
    // config free
    inline auto Free(void* address) noexcept {
        assert(g_config.free && "set config first");
        return g_config.free(address);
    }
}}

// init for this dll
void LongUIAddInitialize(const LongUI::Additional::Config&) noexcept;
// register longui additional control
void LongUIAddRegisterControl() noexcept;
// un-init for this dll
void LongUIAddUninitialize() noexcept;
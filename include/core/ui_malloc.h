#pragma once

#include <cstdlib>

// function Setting
namespace LongUI {
    // alloc for normal space
    void*NormalAlloc(size_t length) noexcept;
    // free for normal space
    void NormalFree(void* address) noexcept;
    // realloc for normal space
    void*NormalRealloc(void* address, size_t length) noexcept;
    // alloc for small space
    void*SmallAlloc(size_t length) noexcept;
    // free for small space
    void SmallFree(void* address) noexcept;
    // template helper
    template<typename T> inline auto NormalAllocT(size_t length) noexcept {
        return reinterpret_cast<T*>(NormalAlloc(length * sizeof(T)));
    }
    // template helper
    template<typename T> inline auto SmallAllocT(size_t length) noexcept {
        return reinterpret_cast<T*>(SmallAlloc(length * sizeof(T)));
    }
}


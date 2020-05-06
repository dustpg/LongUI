#include <core/ui_malloc.h>
#include <core/ui_manager.h>
#include <util/ui_unimacro.h>
#include <interface/ui_default_config.h>
#include <cassert>
#ifndef  NDEBUG
#include <atomic>
#endif

// function Setting
namespace LongUI {
    // IMM
    struct CUIManager::IMM {
        // get malloc inteface
        static inline auto Get() noexcept { return UIManager.config; }
    };
    // alloc for normal space
    PCN_NOINLINE void*NormalAlloc(size_t length) noexcept {
#ifndef  NDEBUG
        using atomic_t = std::atomic<size_t>;
        ++reinterpret_cast<atomic_t&>(UIManager.alloc_counter_n_dbg);
#endif
        using M = CUIManager::IMM;
        assert(M::Get() && "must call UIManager.Initialize() first");
        const auto ptr = M::Get()->NormalAlloc(length);
        assert((reinterpret_cast<uintptr_t>(ptr) & 7) == 0 && "at least alignas 8bytes");
        return ptr;
    }
    // free for normal space
    PCN_NOINLINE void NormalFree(void* address) noexcept {
        using M = CUIManager::IMM;
        assert(M::Get() && "must call UIManager.Initialize() first");
        return M::Get()->NormalFree(address);
    }
    // realloc for normal space
    PCN_NOINLINE void*NormalRealloc(void* address, size_t length) noexcept {
#ifndef  NDEBUG
        using atomic_t = std::atomic<size_t>;
        ++reinterpret_cast<atomic_t&>(UIManager.alloc_counter_n_dbg);
#endif
        using M = CUIManager::IMM;
        assert(M::Get() && "must call UIManager.Initialize() first");
        return M::Get()->NormalRealloc(address, length);
    }
    // alloc for small space
    PCN_NOINLINE void*SmallAlloc(size_t length) noexcept {
#ifndef  NDEBUG
        using atomic_t = std::atomic<size_t>;
        ++reinterpret_cast<atomic_t&>(UIManager.alloc_counter_s_dbg);
#endif
        using M = CUIManager::IMM;
        assert(M::Get() && "must call UIManager.Initialize() first");
        return M::Get()->SmallAlloc(length);
    }
    // free for small space
    PCN_NOINLINE void SmallFree(void* address) noexcept {
        using M = CUIManager::IMM;
        assert(M::Get() && "must call UIManager.Initialize() first");
        return M::Get()->SmallFree(address);
    }
}


// ----------------------------------------------------------------------------
//                         RichED Custom AllocFunc
// ----------------------------------------------------------------------------

#include "../../RichED/ed_config.h"

#ifdef RED_CUSTOM_ALLOCFUNC

/// <summary>
/// Allocs the specified sz.
/// </summary>
/// <param name="len">The length.</param>
/// <returns></returns>
void* RichED::Alloc(size_t len) noexcept {
    return LongUI::NormalAlloc(len);
}

/// <summary>
/// Frees the specified .
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
void RichED::Free(void * ptr) noexcept {
    return LongUI::NormalFree(ptr);
}

/// <summary>
/// Res the alloc.
/// </summary>
/// <param name="ptr">The PTR.</param>
/// <param name="len">The length.</param>
/// <returns></returns>
void* RichED::ReAlloc(void* ptr, size_t len) noexcept {
    return LongUI::NormalRealloc(ptr, len);
}
#endif
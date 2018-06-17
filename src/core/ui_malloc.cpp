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
        using atomic_t = std::atomic<std::size_t>;
        ++reinterpret_cast<atomic_t&>(UIManager.alloc_counter_n_dbg);
#endif
        using M = CUIManager::IMM;
        assert(M::Get() && "must call UIManager.Initialize() first");
        return M::Get()->NormalAlloc(length);
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
        using atomic_t = std::atomic<std::size_t>;
        ++reinterpret_cast<atomic_t&>(UIManager.alloc_counter_n_dbg);
#endif
        using M = CUIManager::IMM;
        assert(M::Get() && "must call UIManager.Initialize() first");
        return M::Get()->NormalRealloc(address, length);
    }
    // alloc for small space
    PCN_NOINLINE void*SmallAlloc(size_t length) noexcept {
#ifndef  NDEBUG
        using atomic_t = std::atomic<std::size_t>;
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



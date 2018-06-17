#pragma once
#ifdef interface
#undef interface
#endif
#pragma interface

#include <new>
#include <util/ui_unimacro.h>

namespace LongUI {
    /// <summary>
    /// ui object cannot new
    /// </summary>
    class CUINoMo {
    public:
        // no move
        //CUINoMo(CUINoMo&&) noexcept = delete;
        // mo copy
        //CUINoMo(const CUINoMo&) noexcept = delete;
        // ctor
        CUINoMo() noexcept = default;
        // no-exception new
        void*operator new(std::size_t) = delete;
        // no-exception new[]
        void*operator new[](std::size_t) = delete;
        // delete []
        void operator delete[](void*, size_t size) noexcept = delete;
        // delete object
        void operator delete(void* ptr) noexcept = delete;
        // nothrow new 
        void*operator new(size_t size, const std::nothrow_t&) noexcept = delete;
        // nothrow delete 
        void operator delete(void* ptr, const std::nothrow_t&) noexcept = delete;
    };
    /// <summary>
    /// ui object
    /// </summary>
    class CUIObject {
    public:
        // no-exception new
        void*operator new(std::size_t) = delete;
        // no-exception new[]
        void*operator new[](std::size_t) = delete;
        // delete []
        void operator delete[](void*, size_t size) noexcept = delete;
        // delete object
        void operator delete(void* ptr) noexcept;
        // nothrow new 
        void*operator new(size_t size, const std::nothrow_t&) noexcept;
        // nothrow delete 
        void operator delete(void* ptr, const std::nothrow_t&) noexcept;
    };
    /// <summary>
    /// ui object
    /// </summary>
    class CUISmallObject {
    public:
        // no-exception new
        void*operator new(std::size_t) = delete;
        // no-exception new[]
        void*operator new[](std::size_t) = delete;
        // delete []
        void operator delete[](void*, size_t size) noexcept = delete;
        // delete object
        void operator delete(void* ptr) noexcept;
        // nothrow new 
        void*operator new(size_t size, const std::nothrow_t&) noexcept;
        // nothrow delete 
        void operator delete(void* ptr, const std::nothrow_t&) noexcept;
    };
    // updatable object
    //struct PCN_NOVTABLE IUIUpdatable { virtual void Update() noexcept = 0; };
    // destroyable object
    struct PCN_NOVTABLE IUIDestroyable { virtual void Destroy() noexcept = 0; };
}
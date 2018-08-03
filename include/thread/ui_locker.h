#pragma once

#include "../util/ui_unimacro.h"
#include <type_traits>
#include <cstdint>


// ui namespace
namespace LongUI {
    // detail namespace
    namespace detail {
        // impl
        template<size_t> struct locker_impl_info {};
        // impl for 32 bit
        template<> struct locker_impl_info<4> { enum { size = 24, align = 4 }; };
        // impl for 64 bit
        template<> struct locker_impl_info<8> { enum { size = 40, align = 8 }; };
    }
    // locker, could be used in "Recursive Calls"
    class CUILocker {
        // buffer size
        enum { buf_size = detail::locker_impl_info<sizeof(void*)>::size };
        // buffer align
        enum { buf_align = detail::locker_impl_info<sizeof(void*)>::align };
    public:
        // ctor
        CUILocker() noexcept;
        // dtor
        ~CUILocker() noexcept;
        // lock
        void Lock() noexcept;
        // unlock
        void Unlock() noexcept;
        // get recursion count
        auto GetRecursionCount() const noexcept->uint32_t;
    public:
        // cannot be moved
        CUILocker(CUILocker&&) noexcept = delete;
        // cannot be copied
        CUILocker(const CUILocker&) noexcept = delete;
        // cannot be operator= moved
        auto operator=(CUILocker&&) noexcept->CUILocker& = delete;
        // cannot be operator= copied
        auto operator=(const CUILocker&) noexcept->CUILocker& = delete;
    protected:
        // buffer storage
        std::aligned_storage<buf_size, buf_align>::type m_impl;
    };
}
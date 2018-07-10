#pragma once
#include <util/ui_unimacro.h>


// ui namespace
namespace LongUI {
    // thread-waiter
    class CUIWaiter {
        // impl
        struct impl_info { void* buf[2];};
        // buffer size
        enum { buf_size = sizeof(impl_info) };
        // buffer align
        enum { buf_align = alignof(impl_info) };
    public:
        // ctor
        CUIWaiter() noexcept;
        // dtor
        ~CUIWaiter() noexcept;
        // wait
        void Wait() noexcept;
        // notify all
        void Broadcast() noexcept;
        // reset, could notify again?
        //void Reset() noexcept;
    public:
        // cannot be moved
        CUIWaiter(CUIWaiter&&) noexcept = delete;
        // cannot be copied
        CUIWaiter(const CUIWaiter&) noexcept = delete;
        // cannot be operator= moved
        auto operator=(CUIWaiter&&) noexcept->CUIWaiter& = delete;
        // cannot be operator= copied
        auto operator=(const CUIWaiter&) noexcept->CUIWaiter& = delete;
    protected:
        // buffer storage
        impl_info                   m_impl;
    };
}
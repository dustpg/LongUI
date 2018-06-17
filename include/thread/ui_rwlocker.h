#pragma once
#include <type_traits>
#include "../util/ui_unimacro.h"


// ui namespace
namespace LongUI {
    // read-write locker, couldn't be used in "Recursive Calls"
    class CUIRWLocker {
        // impl
        template<size_t> struct impl_info {};
        // impl for 32 bit
        template<> struct impl_info<4> { enum { size = 4, align = 4 }; };
        // impl for 64 bit
        template<> struct impl_info<8> { enum { size = 8, align = 8 }; };
        // buffer size
        enum { buf_size = impl_info<sizeof(void*)>::size };
        // buffer align
        enum { buf_align = impl_info<sizeof(void*)>::align };
    public:
        // ctor
        CUIRWLocker() noexcept;
        // dtor
        ~CUIRWLocker() noexcept;
        // reader-lock
        void ReaderLock() noexcept;
        // writer-lock
        void WriterLock() noexcept;
        // reader-unlock
        void ReaderUnlock() noexcept;
        // writer-unlock
        void WriterUnlock() noexcept;
    public:
        // cannot be moved
        CUIRWLocker(CUIRWLocker&&) noexcept = delete;
        // cannot be copied
        CUIRWLocker(const CUIRWLocker&) noexcept = delete;
        // cannot be operator= moved
        auto operator=(CUIRWLocker&&) noexcept->CUIRWLocker& = delete;
        // cannot be operator= copied
        auto operator=(const CUIRWLocker&) noexcept->CUIRWLocker& = delete;
    protected:
        // buffer storage
        std::aligned_storage<buf_size, buf_align>::type m_impl;
    };
    // auto writer locker
    class CUIAutoWriterLocker {
    public:
        // ctor
        CUIAutoWriterLocker(CUIRWLocker& l) noexcept : m_locker(l) { l.WriterLock(); }
        // dtor
        ~CUIAutoWriterLocker() noexcept { m_locker.WriterUnlock(); }
    protected:
        // locker
        CUIRWLocker&        m_locker;
    };
    // auto reader locker
    class CUIAutoReaderLocker {
    public:
        // ctor
        CUIAutoReaderLocker(CUIRWLocker& l) noexcept : m_locker(l) { l.ReaderLock(); }
        // dtor
        ~CUIAutoReaderLocker() noexcept { m_locker.ReaderUnlock(); }
    protected:
        // locker
        CUIRWLocker&        m_locker;
    };
}
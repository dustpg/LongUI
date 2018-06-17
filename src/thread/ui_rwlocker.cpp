#ifndef _WIN32_WINNT
#define _WIN32_WINNT  0x0600
#endif
#include <Windows.h>
#include <Synchapi.h>
#include <thread/ui_rwlocker.h>

// read/write locker
namespace LongUI { using ui_rwlocker_t = SRWLOCK; }


/// <summary>
/// Initializes a new instance of the <see cref="CUIRWLocker"/> class.
/// </summary>
LongUI::CUIRWLocker::CUIRWLocker() noexcept {
    static_assert(sizeof(ui_rwlocker_t) == buf_size, "must be same");
    static_assert(alignof(ui_rwlocker_t) == buf_align, "must be same");
    const auto locker = reinterpret_cast<ui_rwlocker_t*>(&m_impl);
    ::InitializeSRWLock(locker);
}

/// <summary>
/// Finalizes an instance of the <see cref="CUIRWLocker"/> class.
/// </summary>
/// <returns></returns>
LongUI::CUIRWLocker::~CUIRWLocker() noexcept {

}

/// <summary>
/// Readers the lock.
/// </summary>
/// <returns></returns>
void LongUI::CUIRWLocker::ReaderLock() noexcept {
    const auto locker = reinterpret_cast<ui_rwlocker_t*>(&m_impl);
#ifdef NDEBUG
    ::AcquireSRWLockShared(locker);
#else
    if (!::TryAcquireSRWLockShared(locker)) {
        ::OutputDebugStringA("r-locker been locked.\r\n");
        ::AcquireSRWLockShared(locker);
    }
#endif
}

/// <summary>
/// Writers the lock.
/// </summary>
/// <returns></returns>
void LongUI::CUIRWLocker::WriterLock() noexcept {
    const auto locker = reinterpret_cast<ui_rwlocker_t*>(&m_impl);
#ifdef NDEBUG
    ::AcquireSRWLockExclusive(locker);
#else
    if (!::TryAcquireSRWLockExclusive(locker)) {
        ::OutputDebugStringA("w-locker been locked.\r\n");
        ::AcquireSRWLockExclusive(locker);
    }
#endif
}

/// <summary>
/// Readers the unlock.
/// </summary>
/// <returns></returns>
void LongUI::CUIRWLocker::ReaderUnlock() noexcept {
    const auto locker = reinterpret_cast<ui_rwlocker_t*>(&m_impl);
    ::ReleaseSRWLockShared(locker);
}

/// <summary>
/// Writers the unlock.
/// </summary>
/// <returns></returns>
void LongUI::CUIRWLocker::WriterUnlock() noexcept {
    const auto locker = reinterpret_cast<ui_rwlocker_t*>(&m_impl);
    ::ReleaseSRWLockExclusive(locker);
}
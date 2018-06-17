#include <Windows.h>
#include <thread/ui_locker.h>

// locker type
namespace LongUI { using ui_locker_t = CRITICAL_SECTION; }

/// <summary>
/// Initializes a new instance of the <see cref="CUILocker"/> class.
/// </summary>
LongUI::CUILocker::CUILocker() noexcept {
    static_assert(sizeof(ui_locker_t) == buf_size, "must be same");
    static_assert(alignof(ui_locker_t) == buf_align, "must be same");
    const auto locker = reinterpret_cast<ui_locker_t*>(&m_impl);
    ::InitializeCriticalSection(locker);
}

/// <summary>
/// Finalizes an instance of the <see cref="CUILocker"/> class.
/// </summary>
/// <returns></returns>
LongUI::CUILocker::~CUILocker() noexcept {
    const auto locker = reinterpret_cast<ui_locker_t*>(&m_impl);
    ::DeleteCriticalSection(locker);
}

/// <summary>
/// Locks this instance.
/// </summary>
/// <returns></returns>
void LongUI::CUILocker::Lock() noexcept {
    const auto locker = reinterpret_cast<ui_locker_t*>(&m_impl);
#ifdef NDEBUG
    ::EnterCriticalSection(locker);
#else
    if (!::TryEnterCriticalSection(locker)) {
        ::OutputDebugStringA("locker been locked.\r\n");
        ::EnterCriticalSection(locker);
    }
#endif
}

/// <summary>
/// Unlocks this instance.
/// </summary>
/// <returns></returns>
void LongUI::CUILocker::Unlock() noexcept {
    const auto locker = reinterpret_cast<ui_locker_t*>(&m_impl);
    ::LeaveCriticalSection(locker);
}
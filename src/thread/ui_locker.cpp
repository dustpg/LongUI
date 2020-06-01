#include <Windows.h>
#include <thread/ui_locker.h>
#ifndef NDEBUG
#include <cstdio>
extern volatile UINT g_dbgLastEventId ;
#endif

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
#ifdef LUI_DEBUG_BLOCKED_MSGID
    if (!::TryEnterCriticalSection(locker)) {
        char buf[16];
        static int s_counter = 0; ++s_counter;
        ::OutputDebugStringA("locker been locked#");
        std::sprintf(buf, "%4d", s_counter)[buf] = 0;
        ::OutputDebugStringA(buf);

        ::OutputDebugStringA("@msg: ");

        std::sprintf(buf, "%4d", g_dbgLastEventId)[buf] = 0;
        ::OutputDebugStringA(buf);

        ::OutputDebugStringA("\r\n");
        ::EnterCriticalSection(locker);
}
#else
    ::EnterCriticalSection(locker);
#endif
}

/// <summary>
/// try to lock
/// </summary>
/// <returns></returns>
bool LongUI::CUILocker::TryLock() noexcept {
    const auto locker = reinterpret_cast<ui_locker_t*>(&m_impl);
    return !!::TryEnterCriticalSection(locker);
}

/// <summary>
/// Unlocks this instance.
/// </summary>
/// <returns></returns>
void LongUI::CUILocker::Unlock() noexcept {
    const auto locker = reinterpret_cast<ui_locker_t*>(&m_impl);
    ::LeaveCriticalSection(locker);
}

/// <summary>
/// Gets the recursion count.
/// </summary>
/// <returns></returns>
auto LongUI::CUILocker::GetRecursionCount() const noexcept -> uint32_t {
    const auto locker = reinterpret_cast<const ui_locker_t*>(&m_impl);
    return locker->RecursionCount;
}
#ifndef _WIN32_WINNT
#define _WIN32_WINNT  0x0600
#endif
// windows header
#include <Windows.h>
#include <Synchapi.h>
// c/c++ header
#include <thread>
#include <cassert>
// ui header
#include <thread/ui_waiter.h>

// waiter impl type
namespace LongUI { struct ui_waiter_impl_t {
    // cs
    SRWLOCK             rwl;
    // cv
    CONDITION_VARIABLE  cv;
    // get bool
    auto get_bool() const noexcept ->uintptr_t {
        return reinterpret_cast<uintptr_t>(cv.Ptr) & 1;
    }
    // set bool(true)
    void set_true() noexcept {
        reinterpret_cast<uintptr_t&>(cv.Ptr) |= 1;
    }
    // set bool(false)
    void set_false() noexcept {
        reinterpret_cast<uintptr_t&>(cv.Ptr) &= ~uintptr_t(1);
    }
    // get cv
    auto get_cv() const noexcept ->CONDITION_VARIABLE {
        CONDITION_VARIABLE auto_cv = this->cv;
        reinterpret_cast<uintptr_t&>(auto_cv.Ptr) &= ~uintptr_t(1);
        return auto_cv;
    }
    // set cv
    void set_cv(CONDITION_VARIABLE auto_cv) noexcept {
        const auto b = this->get_bool();
        this->cv = auto_cv;
        reinterpret_cast<uintptr_t&>(this->cv.Ptr) |= b;
    }
};}

/// <summary>
/// Initializes a new instance of the <see cref="CUIWaiter"/> class.
/// </summary>
LongUI::CUIWaiter::CUIWaiter() noexcept {
    static_assert(sizeof(ui_waiter_impl_t) == buf_size, "must be same");
    static_assert(alignof(ui_waiter_impl_t) == buf_align, "must be same");
    auto& waiter = reinterpret_cast<ui_waiter_impl_t&>(m_impl);
    ::InitializeSRWLock(&waiter.rwl);
    ::InitializeConditionVariable(&waiter.cv);
    assert(waiter.get_bool() == false && "must be false");
}


/// <summary>
/// Waits this instance.
/// </summary>
/// <returns></returns>
void LongUI::CUIWaiter::Wait() noexcept {
    auto& waiter = reinterpret_cast<ui_waiter_impl_t&>(m_impl);
    ::AcquireSRWLockShared(&waiter.rwl);
    while (!waiter.get_bool())
        ::SleepConditionVariableSRW(
            &waiter.cv,
            &waiter.rwl,
            INFINITE,
            CONDITION_VARIABLE_LOCKMODE_SHARED
        );
    waiter.set_false();
    ::ReleaseSRWLockShared(&waiter.rwl);
}

/// <summary>
/// Notifies the one.
/// </summary>
/// <returns></returns>
/*void LongUI::CUIWaiter::NotifyOne() noexcept {
    // 最低位储存
    auto& waiter = reinterpret_cast<ui_waiter_impl_t&>(m_impl);
    ::AcquireSRWLockExclusive(&waiter.rwl);
    CONDITION_VARIABLE cv = waiter.cv;
    ::WakeConditionVariable(&waiter.cv);
    ::ReleaseSRWLockExclusive(&waiter.rwl);
}*/

/// <summary>
/// Notifies all.
/// </summary>
/// <returns></returns>
void LongUI::CUIWaiter::Broadcast() noexcept {
    auto& waiter = reinterpret_cast<ui_waiter_impl_t&>(m_impl);
    ::AcquireSRWLockExclusive(&waiter.rwl);
    auto cv = waiter.get_cv();
    ::WakeConditionVariable(&cv);
    waiter.set_cv(cv);
    waiter.set_true();
    ::ReleaseSRWLockExclusive(&waiter.rwl);
}

/// <summary>
/// Resets this instance.
/// </summary>
/// <returns></returns>
/*void LongUI::CUIWaiter::Reset() noexcept {
    auto& waiter = reinterpret_cast<ui_waiter_impl_t&>(m_impl);
    ::AcquireSRWLockExclusive(&waiter.rwl);
    ::InitializeConditionVariable(&waiter.cv);
    assert(waiter.get_bool() == false && "must be false");
    ::ReleaseSRWLockExclusive(&waiter.rwl);
}*/

/// <summary>
/// Finalizes an instance of the <see cref="CUIWaiter"/> class.
/// </summary>
/// <returns></returns>
LongUI::CUIWaiter::~CUIWaiter() noexcept {
    auto& waiter = reinterpret_cast<ui_waiter_impl_t&>(m_impl);
}
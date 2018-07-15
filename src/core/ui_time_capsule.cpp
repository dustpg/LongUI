#include <core/ui_time_capsule.h>
#include <control/ui_control.h>
#include <cassert>
// PRIVATE
#include "../private/ui_private_control.h"

/// <summary>
/// Initializes a new instance of the <see cref="CUITimeCapsule"/> class.
/// </summary>
/// <param name="total">The total.</param>
LongUI::CUITimeCapsule::CUITimeCapsule(float total) noexcept
    : m_fTotalTime(total) {
    assert(total >= 0.f && "bad time");
}


/// <summary>
/// Releases unmanaged and - optionally - managed resources.
/// </summary>
/// <returns></returns>
void LongUI::CUITimeCapsule::dispose() noexcept {
    assert(this && "this pointer cannot be null");
    assert(this->prev && "prev pointer cannot be null");
    assert(this->next && "next pointer cannot be null");
    this->prev->next = this->next;
    this->next->prev = this->prev;
    delete this;
}

/// <summary>
/// Normals the check hoster last end.
/// </summary>
/// <returns></returns>
void LongUI::CUITimeCapsule::normal_check_hoster_last_end() noexcept {
    // 记录针对控件的释放
    if (const auto ptr = m_pHoster) {
        auto& last_end = UIControlPrivate::RefLastEnd(*ptr);
        assert(last_end && "last end cannot be null if tc exist");
        if (last_end == this) last_end = nullptr;
    }
}

// longui::impl
namespace LongUI { namespace impl {
    // search for last end
    void search_capsule_for_last_end(UIControl&) noexcept;
}}

/// <summary>
/// Res the start.
/// </summary>
/// <returns></returns>
void LongUI::CUITimeCapsule::Restart() noexcept {
    m_fDoneTime = 0.f;
    // 存在拥有者?
    if (const auto hoster = m_pHoster) {
        auto& last_end = UIControlPrivate::RefLastEnd(*hoster);
        assert(last_end && "cannot be null here");
        // 自己比last_end更长寿则替换掉
        if (this->IsMoreMOThan(*last_end)) 
            last_end = this;
    }
}

/// <summary>
/// Terminates this instance.
/// </summary>
/// <returns></returns>
void LongUI::CUITimeCapsule::Terminate() noexcept {
    const auto hoster = m_pHoster;
    // 释放了再说
    this->dispose();
    // 存在拥有者?
    if (hoster) {
        auto& last_end = UIControlPrivate::RefLastEnd(*hoster);
        if (last_end == this) {
            last_end = nullptr;
            impl::search_capsule_for_last_end(*hoster);
        }
    }
}

/// <summary>
/// Calls the specified delta.
/// </summary>
/// <param name="delta">The delta.</param>
/// <returns></returns>
bool LongUI::CUITimeCapsule::Call(float delta) noexcept {
    m_fDoneTime += delta;
    auto p = m_fDoneTime / m_fTotalTime;
    bool rv = false;
    if (m_fDoneTime >= m_fTotalTime) { p = 1.f; rv = true; }
    this->call(p);
    return rv;
}

/// <summary>
/// Determines whether [is more mo than] [the specified x].
/// </summary>
/// <param name="x">The x.</param>
/// <returns></returns>
bool LongUI::CUITimeCapsule::IsMoreMOThan(const CUITimeCapsule& x) const noexcept {
    // 比较两个时间胶囊谁更长寿
    const auto a = m_fTotalTime - m_fDoneTime;
    const auto b = x.m_fTotalTime - x.m_fDoneTime;
    return a > b ;
}
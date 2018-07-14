#include <core/ui_time_capsule.h>
#include <control/ui_control.h>
#include <cassert>

/// <summary>
/// Initializes a new instance of the <see cref="CUITimeCapsule"/> class.
/// </summary>
/// <param name="total">The total.</param>
LongUI::CUITimeCapsule::CUITimeCapsule(float total) noexcept
    : m_fTotalTime(total) {
    assert(total >= 0.f && "bad time");
}


/// <summary>
/// Tries the clear last time capsule.
/// </summary>
/// <param name="tc">The tc.</param>
/// <returns></returns>
void LongUI::UIControl::TryClearLastTimeCapsule(CUITimeCapsule& tc) noexcept {
    assert(m_pLastEnd && "last end cannot be null if tc exist");
    if (m_pLastEnd == &tc) m_pLastEnd = nullptr;
}

/// <summary>
/// Releases unmanaged and - optionally - managed resources.
/// </summary>
/// <returns></returns>
void LongUI::CUITimeCapsule::Dispose() noexcept {
    assert(this && "this pointer cannot be null");
    assert(this->prev && "prev pointer cannot be null");
    assert(this->next && "next pointer cannot be null");
    // 记录针对控件的释放
    if (const auto ptr = m_pHoster) {
        ptr->TryClearLastTimeCapsule(*this);
    }
    this->prev->next = this->next;
    this->next->prev = this->prev;
    delete this;
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
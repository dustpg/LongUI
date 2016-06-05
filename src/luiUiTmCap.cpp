#include <LongUI/luiUiTmCap.h>
#include <algorithm>

/// <summary>
/// Initializes a new instance of the <see cref="CUITimeCapsule"/> class.
/// </summary>
/// <param name="call">The call.</param>
/// <param name="time">The time.</param>
LongUI::CUITimeCapsule::CUITimeCapsule(TimeCallBack && call, size_t id, float time) 
    : m_call(std::move(call)), m_id(id), m_fTimeTotal(time) {
}


/// <summary>
/// Updates the specified delta_time.
/// </summary>
/// <param name="delta_time">The delta_time.</param>
/// <returns></returns>
bool LongUI::CUITimeCapsule::Update(float delta_time) noexcept {
    m_fTimeDone += delta_time;
    float i = m_fTimeDone / m_fTimeTotal;
    i = std::min(1.f, i);
    return m_call(i) || (i == 1.f);
}
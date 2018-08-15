#include <util/ui_double_click.h>

PCN_NOINLINE
/// <summary>
/// Clicks this instance.
/// </summary>
/// <returns></returns>
bool LongUI::CUIDbClick::Click() noexcept {
    auto now = LongUI::GetTimeTick();
    bool result = ((now - m_dwDbClickPt) <= m_dwDbClickDur) ;
    /*&& x == this->ptx && y == this->pty*/
    m_dwDbClickPt = result ? 0 : now;
    return result;
}

PCN_NOINLINE
/// <summary>
/// Clicks the specified x.
/// </summary>
/// <param name="x">The x.</param>
/// <param name="y">The y.</param>
/// <returns></returns>
bool LongUI::CUIDbClickEx::Click(int32_t x, int32_t y) noexcept {
    auto now = LongUI::GetTimeTick();
    bool result = ((now - m_dwDbClickPt) <= m_dwDbClickDur)
        && x == m_x && y == m_y;
    m_dwDbClickPt = result ? 0 : now;
    m_x = x; m_y = y;
    return result;
}
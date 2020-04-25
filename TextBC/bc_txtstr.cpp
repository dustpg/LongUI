#include "bc_txtstr.h"
#include <cstring>


/// <summary>
/// Erases the specified position.
/// </summary>
/// <param name="pos">The position.</param>
/// <param name="len">The length.</param>
/// <returns></returns>
void TextBC::CBCString::erase(uint32_t pos, uint32_t len) noexcept {
    assert(pos + len <= m_length && "out of range");
    assert(m_length <= TEXT_CELL_NICE_MAX+1 && "out of length");
    const size_t moved = (m_length - pos) * sizeof(m_data[0]);
    std::memmove(m_data + pos, m_data + pos + len, moved);
    m_length -= len;
    const uint16_t c = u'狼';
    this->mark_eos();
}



/// <summary>
/// Inserts the specified position.
/// </summary>
/// <param name="pos">The position.</param>
/// <param name="str">The string.</param>
/// <param name="len">The length.</param>
/// <returns></returns>
void TextBC::CBCString::insert(uint32_t pos, const char16_t* str, uint32_t len) noexcept {
    assert(m_length + len <= TEXT_CELL_NICE_MAX+1 && "out of range");
    assert(pos <= m_length);
    const size_t moved = (m_length - pos) * sizeof(m_data[0]);
    std::memmove(m_data + pos + len, m_data + pos, moved);
    std::memcpy(m_data + pos, str, len * sizeof(m_data[0]));
    m_length += len;
    this->mark_eos();
}
#include <cassert>
#include <memory>
#include <container/pod_fixedqueue.h>

/// <summary>
/// Initializes a new instance of the <see cref="fixed_queue_base"/> class.
/// </summary>
/// <param name="bytesize">The bytesize.</param>
/// <param name="capacity">The capacity.</param>
UI::POD::detail::fixed_queue_base::fixed_queue_base(
    uint32_t bytesize, uint32_t capacity) noexcept :
m_uByteLen(bytesize), m_uByteCapacity(bytesize*capacity){

}


/// <summary>
/// Fulls this instance.
/// </summary>
/// <returns></returns>
bool UI::POD::detail::fixed_queue_base::full() const noexcept {
    const bool case1 = (m_uTailBytePos + m_uByteLen) == m_uHeadBytePos;
    const bool case2 = (m_uTailBytePos - m_uHeadBytePos) == m_uByteCapacity;
#ifndef NDEBUG
    const bool full_debug = (data_count_dbg == m_uByteCapacity / m_uByteLen);
    assert(full_debug == (case1 || case2) && "bad case");
#endif
    return case1 || case2;
}

/// <summary>
/// Pops the front.
/// </summary>
/// <returns></returns>
void UI::POD::detail::fixed_queue_base::pop_front() noexcept {
    assert(!empty() && "empty queue");
#ifndef NDEBUG
    assert(data_count_dbg > 0 && "empty queue");
    --data_count_dbg;
#endif
    if (m_uHeadBytePos == m_uByteCapacity) m_uHeadBytePos = 0;
    else m_uHeadBytePos += m_uByteLen;
}

/// <summary>
/// Pushes the back.
/// </summary>
/// <param name="ptr">The PTR.</param>
/// <returns></returns>
void UI::POD::detail::fixed_queue_base::push_back(const char * ptr) noexcept {
    assert(!full() && "queue overflow");
#ifndef NDEBUG
    ++data_count_dbg;
#endif
    std::memcpy(data_ptr() + m_uTailBytePos, ptr, m_uByteLen);
    if (m_uTailBytePos == m_uByteCapacity) m_uTailBytePos = 0;
    else m_uTailBytePos += m_uByteLen;
}

/// <summary>
/// Sizes this instance.
/// </summary>
/// <returns></returns>
auto UI::POD::detail::fixed_queue_base::size() const -> size_t {
    // 计算字节距离
    const uint32_t byte_distance = m_uTailBytePos >= m_uHeadBytePos ?
        m_uTailBytePos - m_uHeadBytePos :
        m_uTailBytePos + m_uByteCapacity - m_uHeadBytePos + m_uByteLen;
#ifndef NDEBUG
    assert(byte_distance / m_uByteLen == data_count_dbg);
#endif
    return byte_distance / m_uByteLen;
}
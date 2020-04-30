#include "bc_txtbuf.h"
#include <cstdlib>
#include <cstring>


/// <summary>
/// Initializes a new instance of the <see cref="buffer_base"/> class.
/// </summary>
TextBC::detail::buffer_base::buffer_base() noexcept {

}

/// <summary>
/// Finalizes an instance of the <see cref="buffer_base"/> class.
/// </summary>
/// <returns></returns>
TextBC::detail::buffer_base::~buffer_base() noexcept {
    if (this->is_ok()) {
        assert(m_data && "cannot be nullptr");
        std::free(m_data);
    }
}

/// <summary>
/// Resizes the buffer.
/// </summary>
/// <param name="len">The length.</param>
/// <param name="size_of">The size of.</param>
/// <returns></returns>
bool TextBC::detail::buffer_base::resize_buffer(uint32_t len, size_t size_of) noexcept {
    // 不够的情况
    if (len > m_capacity) {
        const auto old_ptr = m_data;
        const auto new_ptr = std::realloc(old_ptr, size_of * len);
        // 内存不足
        if (!new_ptr) {
            m_length = 0;
            m_capacity = 0;
            std::free(old_ptr);
            m_data = nullptr;
            return false;
        }
        m_data = new_ptr;
        m_capacity = len;
    }
#ifndef NDEBUG
    if (len > m_length) {
        const auto ptr = reinterpret_cast<char*>(m_data) + size_of * m_length;
        std::memset(ptr, -1, size_of * (len - m_length));
    }
#endif
    m_length = len;
    return true;
}

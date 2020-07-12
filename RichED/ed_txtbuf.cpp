#include "ed_config.h"
#include "ed_txtbuf.h"
#include "ed_txtplat.h"
#include <cstdlib>
#include <cstring>


/// <summary>
/// Initializes a new instance of the <see cref="buffer_base"/> class.
/// </summary>
RichED::impl::buffer_base::buffer_base() noexcept {

}

/// <summary>
/// Finalizes an instance of the <see cref="buffer_base"/> class.
/// </summary>
/// <returns></returns>
RichED::impl::buffer_base::~buffer_base() noexcept {
    if (this->is_ok()) {
        assert(m_data && "cannot be nullptr");
        RichED::Free(m_data);
    }
}

PCN_NOINLINE
/// <summary>
/// Resizes the buffer.
/// </summary>
/// <param name="len">The length.</param>
/// <param name="size_of">The size of.</param>
/// <param name="plat">The plat.</param>
/// <returns></returns>
bool RichED::impl::buffer_base::resize_buffer(
    uint32_t len, size_t size_of, IEDTextPlatform& plat) noexcept {
    // 不够的情况
    if (len > m_capacity) {
        const auto old_ptr = m_data;
        const auto myrealloc = [&](void* ptr, size_t len) noexcept ->void* {
            for (size_t i = 0; ; ++i) {
                if (const auto p = RichED::ReAlloc(ptr, len)) return p;
                if (plat.OnOOM(i, len) == OOM_Ignore) break;
            }
            return nullptr;
        };
        const auto new_ptr = myrealloc(old_ptr, size_of * len);
        // 内存不足
        if (!new_ptr) {
            m_length = 0;
            m_capacity = 0;
            RichED::Free(old_ptr);
            m_data = nullptr;
            return false;
        }
        m_data = new_ptr;
        m_capacity = len;
    }
#ifndef NDEBUG
    if (len > m_length) {
        const auto ptr = reinterpret_cast<char*>(m_data) + size_of * m_length;
        std::memset(ptr, 0xfc, size_of * (len - m_length));
    }
#endif
    m_length = len;
    return true;
}

#ifndef RED_CUSTOM_ALLOCFUNC

/// <summary>
/// Allocs the specified sz.
/// </summary>
/// <param name="len">The length.</param>
/// <returns></returns>
void* RichED::Alloc(size_t len) noexcept {
    return std::malloc(len);
}

/// <summary>
/// Frees the specified .
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
void RichED::Free(void * ptr) noexcept {
    return std::free(ptr);
}

/// <summary>
/// Res the alloc.
/// </summary>
/// <param name="ptr">The PTR.</param>
/// <param name="len">The length.</param>
/// <returns></returns>
void* RichED::ReAlloc(void* ptr, size_t len) noexcept {
    return std::realloc(ptr, len);
}
#endif
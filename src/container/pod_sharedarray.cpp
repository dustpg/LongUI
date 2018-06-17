#include <cassert>
#include <cstdlib>
#include <cstring>
#include <container/pod_sharedarray.h>
#include <util/ui_unimacro.h>
#include <core/ui_malloc.h>

PCN_NOINLINE
/// <summary>
/// Releases this instance.
/// </summary>
/// <returns></returns>
auto LongUI::POD::detail::basic_shared_array::release() noexcept ->uint32_t {
    assert(m_cRefCount > 0 && "bad ref-count");
    auto count = --m_cRefCount;
    if (!count) LongUI::NormalFree(this);
    return count;
}


/// <summary>
/// Creates the specified data.
/// </summary>
/// <param name="data">The data.</param>
/// <param name="len">The length.</param>
/// <returns></returns>
auto LongUI::POD::detail::basic_shared_array::create(
    const void* data, uint32_t len, uint32_t byte) noexcept -> self* {
    const auto al = sizeof(self) + len * byte;
    if (const auto ptr = LongUI::NormalAlloc(al)) {
        const auto obj = reinterpret_cast<self*>(ptr);
        obj->m_cRefCount = 1;
        obj->m_cLength = len;
        if (data) std::memcpy(obj->m_data, data, len * byte);
        return obj;
    }
    return nullptr;
}

#ifndef NDEBUG
void LongUI::POD::detail::basic_shared_array::assert_range(uint32_t i) noexcept {
    assert(this && "bad array or out of memory");
    assert(i < m_cLength && "out of range");
}
#endif


PCN_NOINLINE
/// <summary>
/// Copies from.
/// </summary>
/// <param name="a">a.</param>
/// <param name="x">The x.</param>
/// <returns></returns>
void LongUI::POD::detail::basic_shared_array::copy_from(self*& a, self* x) noexcept {
    self::safe_release(a);
    a = self::safe_ref(x);
}

PCN_NOINLINE
/// <summary>
/// Moves from.
/// </summary>
/// <param name="a">a.</param>
/// <param name="x">The x.</param>
/// <returns></returns>
void LongUI::POD::detail::basic_shared_array::move_from(self*& a, self*& x) noexcept {
    self::safe_release(a);
    a = x;
    x = nullptr;
}
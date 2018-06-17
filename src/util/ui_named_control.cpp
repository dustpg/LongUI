// ui
#include <luiconf.h>
#include <core/ui_malloc.h>
#include <core/ui_window.h>
#include <util/ui_named_control.h>
// c++
#include <cstring>


// LongUI::NAMED_CONTROL_EXALLOC
namespace LongUI { enum  : size_t {
#ifdef NDEBUG
    NAMED_CONTROL_EXALLOC = 0
#else
    NAMED_CONTROL_EXALLOC = 1
#endif
};}

/// <summary>
/// Sets the name.
/// </summary>
/// <param name="begin">The begin.</param>
/// <param name="end">The end.</param>
void LongUI::NamedControl::SetName(
    const char* begin, const char* end) noexcept {
    assert(name == nullptr && "bad pointer");
    assert(end > begin && "bad name");
    const size_t str_len = end - begin;
    const size_t all_len = str_len + 1 + NAMED_CONTROL_EXALLOC;
    assert(all_len > SHORT_MEMORY_LENGTH && "name too long");
    // 申请内存
    if (const auto ptr = LongUI::SmallAllocT<char>(all_len)) {
        const auto str = ptr + NAMED_CONTROL_EXALLOC;
        std::memcpy(str, begin, str_len);
        str[str_len] = 0;
        this->name = str;
    }
}

/// <summary>
/// Finds the control.
/// </summary>
/// <param name="window">The window.</param>
void LongUI::NamedControl::FindControl(CUIWindow& window) noexcept {
    // 名称有效
    if (name) {
#ifndef NDEBUG
        assert(value != 1 && "bad pointer");
        assert((value & 1) == 0 && "bad pointer");
#endif
        this->ctrl = window.FindControl(this->name);
        const auto raw_ptr = name - NAMED_CONTROL_EXALLOC;
        LongUI::SmallFree(const_cast<char*>(raw_ptr));
    }
}
// ui
#include <luiconf.h>
#include <core/ui_malloc.h>
#include <core/ui_window.h>
#include <util/ui_named_control.h>
// c++
#include <cstring>


// LongUI::NAMED_CONTROL_EXALLOC
namespace LongUI { enum  : size_t {
    NAMED_CONTROL_EXALLOC = 1
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
        assert((reinterpret_cast<uintptr_t>(ptr) & 1) == 0);
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
void LongUI::NamedControl::FindControl(CUIWindow* window) noexcept {
    const auto raw_name = this->name;
    const auto raw_value = this->value;
    // 名称有效
    if (raw_value & 1) {
        assert(value != 1 && "bad pointer");
        this->ctrl = window ? window->FindControl(raw_name) : nullptr;
        const auto raw_ptr = raw_name - NAMED_CONTROL_EXALLOC;
        LongUI::SmallFree(const_cast<char*>(raw_ptr));
    }
}
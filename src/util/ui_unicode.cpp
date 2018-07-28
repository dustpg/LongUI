// c/c++ header
#include <algorithm>
// ui header
#include <util/ui_unicode_cast.h>


extern "C" {
    // utf-16 -> utf-8
    uint32_t ui_utf16_to_utf8_get_buflen(const char16_t* src, const char16_t* end);
    // utf-8 -> utf-16
    uint32_t ui_utf8_to_utf16_get_buflen(const char* src, const char* end);
    // utf-16 -> utf-8
    uint32_t ui_utf16_to_utf8(
        char* __restrict buf, 
        uint32_t buflen,
        const char16_t* __restrict src , 
        const char16_t* end
    );
    // utf-8 -> utf-16
    uint32_t ui_utf8_to_utf16(
        char16_t* __restrict buf,
        uint32_t buflen,
        const char* __restrict src,
        const char* end
    );
}

/// <summary>
/// Gets the length of the buffer.
/// </summary>
/// <param name="str">The string.</param>
/// <returns></returns>
template<> auto LongUI::Unicode::
GetBufferLength<LongUI::Unicode::UTF8>(const char16_t str[]) noexcept->uint32_t {
    const auto length = detail::strlen(str);
    return ui_utf16_to_utf8_get_buflen(str, str + length) + 1;
}

/// <summary>
/// Gets the length of the buffer.
/// </summary>
/// <param name="view">The view.</param>
/// <returns></returns>
template<> auto LongUI::Unicode::
GetBufferLength<LongUI::Unicode::UTF8>(PodStringView<char16_t> view) noexcept->uint32_t {
    return ui_utf16_to_utf8_get_buflen(view.first, view.second);
}

/// <summary>
/// Gets the length of the buffer.
/// </summary>
/// <param name="str">The string.</param>
/// <returns></returns>
template<> auto LongUI::Unicode::GetBufferLength
<LongUI::Unicode::UTF16>(const char str[]) noexcept->uint32_t {
    const auto length = detail::strlen(str);
    return ui_utf8_to_utf16_get_buflen(str, str + length) + 1;
}

/// <summary>
/// Gets the length of the buffer.
/// </summary>
/// <param name="a">a.</param>
/// <param name="b">The b.</param>
/// <returns></returns>
template<> auto LongUI::Unicode::GetBufferLength
<LongUI::Unicode::UTF16>(PodStringView<char> view) noexcept->uint32_t {
    return ui_utf8_to_utf16_get_buflen(view.first, view.second);
}

/// <summary>
/// To the specified buf.
/// </summary>
/// <param name="buf">The buf.</param>
/// <param name="buflen">The buflen.</param>
/// <param name="str">The string.</param>
/// <returns></returns>
template<>
PCN_NOINLINE
auto LongUI::Unicode::To<LongUI::Unicode::UTF8>(char buf[], uint32_t buflen, const char16_t str[]) noexcept->uint32_t {
    const auto length = ui_utf16_to_utf8(buf, buflen - 1, str, str + detail::strlen(str));
    buf[length] = 0;
    return length;
}

/// <summary>
/// To the specified buf.
/// </summary>
/// <param name="buf">The buf.</param>
/// <param name="buflen">The buflen.</param>
/// <param name="view">The view.</param>
/// <returns></returns>
template<>
auto LongUI::Unicode::To<LongUI::Unicode::UTF8>(char buf[], uint32_t buflen, PodStringView<char16_t> view) noexcept->uint32_t {
    return ui_utf16_to_utf8(buf, buflen, view.first, view.second);
}

/// <summary>
/// To the specified buf.
/// </summary>
/// <param name="buf">The buf.</param>
/// <param name="buflen">The buflen.</param>
/// <param name="str">The string.</param>
/// <returns></returns>
template<>
PCN_NOINLINE
auto LongUI::Unicode::To<LongUI::Unicode::UTF16>(char16_t buf[], uint32_t buflen, const char str[]) noexcept->uint32_t {
    const auto length = ui_utf8_to_utf16(buf, buflen - 1, str, str + detail::strlen(str));
    buf[length] = 0;
    return length;
}

/// <summary>
/// To the specified buf.
/// </summary>
/// <param name="buf">The buf.</param>
/// <param name="buflen">The buflen.</param>
/// <param name="view">The view.</param>
/// <returns></returns>
template<>
auto LongUI::Unicode::To<LongUI::Unicode::UTF16>(char16_t buf[], uint32_t buflen, PodStringView<char> view) noexcept->uint32_t {
    return ui_utf8_to_utf16(buf, buflen, view.first, view.second);
}

PCN_NOINLINE
/// <summary>
/// Swaps the endian.
/// </summary>
/// <param name="begin">The begin.</param>
/// <param name="end">The end.</param>
/// <returns></returns>
void LongUI::Unicode::SwapEndian(char16_t* begin, char16_t* end) noexcept {
    for (auto itr = begin; itr != end; ++itr) {
        const auto ptr = reinterpret_cast<char*>(itr);
        std::swap(ptr[0], ptr[1]);
    }
}


PCN_NOINLINE
/// <summary>
/// Swaps the endian.
/// </summary>
/// <param name="begin">The begin.</param>
/// <param name="end">The end.</param>
/// <returns></returns>
void LongUI::Unicode::SwapEndian(char32_t* begin, char32_t* end) noexcept {
    for (auto itr = begin; itr != end; ++itr) {
        const auto ptr = reinterpret_cast<char*>(itr);
        std::swap(ptr[0], ptr[3]);
        std::swap(ptr[1], ptr[2]);
    }
}

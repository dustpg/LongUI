#pragma once
#include <cstdint>
#include <core/ui_string_view.h>

namespace LongUI {
    /// <summary>
    /// unicode namespace
    /// </summary>
    namespace Unicode {
        // SizeType
        using SizeType = uint32_t;
        // utf-8
        struct UTF8 { using type = char; };
        // utf-16
        struct UTF16 { using type = char16_t; };
        // utf-16
        struct UTF32 { using type = char32_t; };
        // wchar helper
        template<SizeType> struct wchar_helper;
        // 2?char16_t
        template<> struct wchar_helper<sizeof(char16_t)> { using type = UTF16; };
        // 4?char32_t
        template<> struct wchar_helper<sizeof(char32_t)> { using type = UTF32; };
        // wide char
        struct WChar {
            using type = wchar_t;
            using same = typename wchar_helper<sizeof(wchar_t)>::type;
            using same_t = typename same::type;
        };
        // is_surrogate
        inline auto IsSurrogate(uint16_t ch) noexcept { return ((ch) & 0xF800) == 0xD800; }
        // is_high_surrogate
        inline auto IsHighSurrogate(uint16_t ch) noexcept { return ((ch) & 0xFC00) == 0xD800; }
        // is_low_surrogate
        inline auto IsLowSurrogate(uint16_t ch) noexcept { return ((ch) & 0xFC00) == 0xDC00; }
    }
    // wchar_t
    using wcharxx_t = typename Unicode::WChar::same_t;
}

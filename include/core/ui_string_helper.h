#pragma once
#include "../container/pod_vector.h"
#include "../util/ui_unicode.h"
#include "ui_string_view.h"
#include "ui_core_type.h"


namespace LongUI {
    // detail namespace
    namespace detail {
        // string helper
        struct string_helper {
            // base string
            using base_str = POD::detail::vector_base;
            // string set null char
            static void string_set_null_char(void* pos, size_t bytelen) noexcept;
            // change length
            static void relength(base_str& str, uint32_t len) noexcept { str.m_uVecLen = len; }
            // string assign helper
            static void string_assign(base_str&, const char*, const char*) noexcept;
            // string format helper
            static bool string_format(base_str&, const char* fmt, ...) noexcept;
            // string insert helper
            static void string_insert(base_str&, uintptr_t, const char*, const char*) noexcept;
            // string erase
            static void string_erase(base_str& str, size_t pos, size_t len) noexcept;
            // copy as latin1
            static void copy_from_latin1(wchar_t* des, const char* src_begin, const char* src_end) noexcept;
            // copy as latin1
            static void copy_from_latin1(char* des, const char* begin, const char* end) noexcept { std::memcpy(des, begin, end - begin); }
            // string utf-16 to utf32
            static void string_u16_u32(base_str& str, const char16_t*, const char16_t*) noexcept;
            // string utf-16 to utf8
            static void string_u16_u8(base_str& str, const char16_t*, const char16_t*) noexcept;
            // string utf8 to utf-xx
            static void string_u16(char32_t, base_str& str, const char16_t* a, const char16_t* b) noexcept { string_u16_u32(str, a, b); }
            // string utf8 to utf-xx
            static void string_u16(char, base_str& str, const char16_t* a, const char16_t* b) noexcept { string_u16_u8(str, a, b); }
            // string utf8 to utf-16
            static void string_u8_u16(base_str& str, const char*, const char*) noexcept;
            // string utf8 to utf-32
            static void string_u8_u32(base_str& str, const char*, const char*) noexcept;
            // string utf8 to utf-16
            // string utf8 to utf-32

            // string utf8 to utf-xx
            static void string_u8(char32_t, base_str& str, const char* a, const char* b) noexcept { string_u8_u32(str, a, b); }
            // string utf8 to utf-xx
            static void string_u8(char16_t, base_str& str, const char* a, const char* b) noexcept { string_u8_u16(str, a, b); }
            // string utf8 to utf-xx
            static void string_u8(wchar_t, base_str& str, const char* a, const char* b) noexcept { string_u8(wcharxx_t{}, str, a, b); }
#ifdef OPTIMIZATION_STRING_SPLIT_WITHIN_SINGLE_CHAR
            // string split with single char
            static auto string_split(base_str& str, POD::Vector<PodStringView<char>>&, char ch) noexcept->views<char>;
            // string split with single char
            static auto string_split(base_str& str, POD::Vector<PodStringView<wchar_t>>&, wchar_t ch) noexcept->views<wchar_t>;
        private:
            // string split with single char, implement
            template<typename T>
            static void string_split(T ch, base_str& str, POD::Vector<PodStringView<T>>&) noexcept;
#endif
        public:
            // string split with string
            static void string_split(
                const base_str& str,
                POD::Vector<PodStringView<char>>&,
                const char* b,
                const char* end
            ) noexcept;
            // string split with string
            static inline void string_split(
                const base_str& str,
                POD::Vector<PodStringView<wchar_t>>& list,
                const char* b,
                const char* end
            ) noexcept {
                string_split(str, reinterpret_cast<POD::Vector<PodStringView<char>>&>(list), b, end);
            }
        };
    }
}

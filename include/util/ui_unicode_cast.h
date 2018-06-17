#pragma once
#include <cstdint>
#include <core/ui_string_view.h>
#include "ui_unicode.h"


namespace LongUI {
    /// <summary>
    /// unicode namespace
    /// </summary>
    namespace Unicode {
        // swap big/little endian
        inline void SwapEndian(char*, char*) noexcept {}
        // swap big/little endian
        void SwapEndian(char16_t* begin, char16_t* end) noexcept;
        // swap big/little endian
        void SwapEndian(char32_t* begin, char32_t* end) noexcept;
        // get buffer length within null char
        template<typename DST, typename SRC>
        auto GetBufferLength(const SRC str[]) noexcept->SizeType;
        // get buffer length without null char
        template<typename DST, typename SRC>
        auto GetBufferLength(PodStringView<SRC>) noexcept->SizeType;
        // convert to within null char
        template<typename DST, typename SRC>
        auto To(typename DST::type buf[], uint32_t buflen, const SRC str[]) noexcept->SizeType;
        // convert to without null char
        template<typename DST, typename SRC>
        auto To(typename DST::type buf[], uint32_t buflen, PodStringView<SRC>) noexcept->SizeType;
        // ---------------------------------------
        // utf16 -> utf-8
        template<> auto GetBufferLength<UTF8>(const char16_t str[]) noexcept->SizeType;
        // utf16 -> utf-8
        template<> auto GetBufferLength<UTF8>(PodStringView<char16_t>) noexcept->SizeType;
        // utf-8 -> utf16
        template<> auto GetBufferLength<UTF16>(const char str[]) noexcept->SizeType;
        // utf-8 -> utf16
        template<> auto GetBufferLength<UTF16>(PodStringView<char>) noexcept->SizeType;
        // utf16 -> utf-8
        template<> auto To<UTF8>(char buf[], uint32_t buflen, const char16_t str[]) noexcept->SizeType;
        // utf16 -> utf-8
        template<> auto To<UTF8>(char buf[], uint32_t buflen, PodStringView<char16_t>) noexcept->SizeType;
        //  utf-8 -> utf16
        template<> auto To<UTF16>(char16_t buf[], uint32_t buflen, const char str[]) noexcept->SizeType;
        //  utf-8 -> utf16
        template<> auto To<UTF16>(char16_t buf[], uint32_t buflen, PodStringView<char>) noexcept->SizeType;
    }
}

// UI  wchar sp
namespace LongUI {
    namespace Unicode {
        // wchar -> utf-8
        template<> inline auto GetBufferLength<UTF8>(const wchar_t str[]) noexcept->SizeType {
            using type = typename WChar::same::type;
            return GetBufferLength<UTF8>(reinterpret_cast<const type*>(str));
        }
        // wchar -> utf-8
        template<> inline auto GetBufferLength<UTF8>(PodStringView<wchar_t> v) noexcept->SizeType {
            using type = typename WChar::same::type;
            return GetBufferLength<UTF8>(PodStringView<type>{
                reinterpret_cast<const type*>(v.first),
                    reinterpret_cast<const type*>(v.second)
            });
        }
        // utf-8 -> wchar
        template<> inline auto GetBufferLength<WChar>(const char str[]) noexcept->SizeType {
            using type = typename WChar::same;
            return GetBufferLength<type>(str);
        }
        // utf-8 -> wchar
        template<> inline auto GetBufferLength<WChar>(PodStringView<char> v) noexcept->SizeType {
            using type = typename WChar::same;
            return GetBufferLength<type>(v);
        }

        // utf16 -> utf-8
        template<> inline auto To<UTF8>(char buf[], uint32_t buflen, const wchar_t str[]) noexcept->SizeType {
            using type = typename WChar::same::type;
            return To<UTF8>(buf, buflen, reinterpret_cast<const type*>(str));
        }
        // utf16 -> utf-8
        template<> inline auto To<UTF8>(char buf[], uint32_t buflen, PodStringView<wchar_t> v) noexcept->SizeType {
            using type = typename WChar::same::type;
            return To<UTF8>(buf, buflen, PodStringView<type>{
                reinterpret_cast<const type*>(v.first),
                    reinterpret_cast<const type*>(v.second)
            });
        }
        //  utf-8 -> utf16
        template<> inline auto To<WChar>(wchar_t buf[], uint32_t buflen, const char str[]) noexcept->SizeType {
            using type = typename WChar::same;
            return To<type>(reinterpret_cast<type::type*>(buf), buflen, str);
        }
        //  utf-8 -> utf16
        template<> inline auto To<WChar>(wchar_t buf[], uint32_t buflen, PodStringView<char> v) noexcept->SizeType {
            using type = typename WChar::same;
            return To<type>(reinterpret_cast<type::type*>(buf), buflen, v);
        }
    }
}

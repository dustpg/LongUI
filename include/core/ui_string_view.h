#pragma once

// Util
#include "../luiconf.h"
#include "../util/ui_endian.h"
#include "../util/ui_unicode.h"
#include "../util/ui_unimacro.h"
// C++
#include <cstdint>
#include <cstring>
#include <cwchar>


namespace LongUI {
    // detail namespace
    namespace detail {
        // string length
        inline auto strlen(const char* ptr) noexcept { return std::strlen(ptr); }
        // string length
        inline auto strlen(const wcharxx_t* ptr) noexcept { return std::wcslen(reinterpret_cast<const wchar_t*>(ptr)); }
        // string length
        auto strlen(const unwchar_t* ptr) noexcept->size_t;
        // name to rgb32
        auto name_rgb32(const char* a, const char* b, char c) noexcept->uint32_t;
        // name to rgba32
        auto color_rgba32(const char* a, const char* b, char c) noexcept->uint32_t;
    }
    // string view
    template<typename T> struct PodStringView {
        // from c-tyle string
        static auto FromCStyle(const T str[]) noexcept -> PodStringView { return{ str, str + detail::strlen(str) }; };
        // size
        auto size() const noexcept { return static_cast<uint32_t>(end() - begin()); }
        // begin
        auto begin() const noexcept { return first; };
        // end
        auto end() const noexcept { return second; };
        // to RGBA32 in byte order
        auto ColorRGBA32() const noexcept->uint32_t {
            return detail::color_rgba32(
                reinterpret_cast<const char*>(first) + helper::ascii_offset<sizeof(T)>::value,
                reinterpret_cast<const char*>(second) + helper::ascii_offset<sizeof(T)>::value,
                sizeof(T)
            );}
        // to RGBA32 in byte order
        auto NamedRGB32() const noexcept->uint32_t { 
            return detail::name_rgb32(
                reinterpret_cast<const char*>(first) + helper::ascii_offset<sizeof(T)>::value,
                reinterpret_cast<const char*>(second) + helper::ascii_offset<sizeof(T)>::value,
                sizeof(T)
            );}
        // get char then move
        //auto Char32() noexcept -> char32_t;
        // to rgba32
        //auto NamedRGBA32() const noexcept -> uint32_t { (NamedRGB32() << 8) | 0xff; }
        // split then move
        auto Split(T ch) noexcept->PodStringView;
        // to float
        operator float() const noexcept;
        // to double
        operator double() const noexcept;
        // to int32_t
        operator int32_t() const noexcept;
        // operator==
        bool operator==(const PodStringView x) const noexcept {
            return size() == x.size() && !std::memcmp(begin(), x.begin(), size() * sizeof(T));
        }
        // operator!=
        bool operator!=(const PodStringView x) const noexcept {
            return !((*this) == x);
        }
        // to bool
        bool ToBool() noexcept { return *begin() == 't'; }
        // to float
        auto ToFloat() const noexcept { return static_cast<float>(*this); }
        // to int32_t
        auto ToInt32() const noexcept { return static_cast<int32_t>(*this); }
        // to double
        auto ToDouble() const noexcept { return static_cast<double>(*this); }
        // 1st
        const T*        first;
        // 2nd
        const T*        second;
    };
    // split unit for U8View
    auto SplitUnit(PodStringView<char>&) noexcept ->PodStringView<char>;
    // split string with substring
    auto SplitStr(PodStringView<char>&, PodStringView<char>) noexcept->PodStringView<char>;
    // find last dir
    auto FindLastDir(PodStringView<char>) noexcept ->PodStringView<char>;
    // _sv
    inline PodStringView<char> operator ""_sv(const char* str, size_t len) noexcept {
        return{ str , str + len };
    }
    // _sv
    inline PodStringView<wchar_t> operator ""_sv(const wchar_t* str, size_t len) noexcept {
        return{ str , str + len };
    }
    // _sv
    inline PodStringView<char16_t> operator ""_sv(const char16_t* str, size_t len) noexcept {
        return{ str , str + len };
    }
    // _sv
    inline PodStringView<char32_t> operator ""_sv(const char32_t* str, size_t len) noexcept {
        return{ str , str + len };
    }
}
// HELPER MACRO
#define LUI_DECLARE_METHOD_FOR_CHAR_TYPE(T) \
    template<> PodStringView<T>::operator float() const noexcept;\
    template<> PodStringView<T>::operator double() const noexcept;\
    template<> PodStringView<T>::operator int32_t() const noexcept;\
    template<> PodStringView<T> PodStringView<T>::Split(T ch) noexcept;

namespace LongUI {
    // char
    LUI_DECLARE_METHOD_FOR_CHAR_TYPE(char);
    // char16_t
    LUI_DECLARE_METHOD_FOR_CHAR_TYPE(char16_t);
    // char32_t
    LUI_DECLARE_METHOD_FOR_CHAR_TYPE(char32_t);
}
#undef LUI_DECLARE_METHOD_FOR_CHAR_TYPE

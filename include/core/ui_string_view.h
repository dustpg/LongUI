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
    namespace impl {
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
        // range assert
        void range_assert(uint32_t, uint32_t) noexcept;
    }
    // string view
    template<typename T> struct PodStringView {
        // from c-tyle string
        static auto FromCStyle(const T str[]) noexcept -> PodStringView { return{ str, str + impl::strlen(str) }; };
        // size
        auto size() const noexcept { return static_cast<uint32_t>(end() - begin()); }
        // begin
        auto begin() const noexcept { return first; };
        // end
        auto end() const noexcept { return second; };
        // to RGBA32 in byte order
        auto ColorRGBA32() const noexcept->uint32_t {
            return impl::color_rgba32(
                reinterpret_cast<const char*>(first) + helper::ascii_offset<sizeof(T)>::value,
                reinterpret_cast<const char*>(second) + helper::ascii_offset<sizeof(T)>::value,
                sizeof(T)
            );}
        // to RGBA32 in byte order
        auto NamedRGB32() const noexcept->uint32_t { 
            return impl::name_rgb32(
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
        // operator==
        bool operator==(const PodStringView x) const noexcept {
            return size() == x.size() && !std::memcmp(begin(), x.begin(), size() * sizeof(T));
        }
        // operator!=
        bool operator!=(const PodStringView x) const noexcept { return !((*this) == x); }
#ifdef NDEBUG
        // operator[]
        auto operator[](uint32_t i) const noexcept { return first[i]; }
#else
        // operator[]
        auto operator[](uint32_t i) const noexcept { impl::range_assert(i, size()); return first[i]; }
#endif
        // to bool
        bool ToBool() noexcept { return *begin() == 't'; }
        // to int32_t
        auto ToInt32() const noexcept ->int32_t;
        // to double
        auto ToDouble(char decimal = '.') const noexcept ->double;
        // to float
        auto ToFloat(char decimal = '.') const noexcept { return static_cast<float>(ToDouble()); }
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
    auto FindLastDir(PodStringView<char>) noexcept->PodStringView<char>;
    // _sv
    inline PodStringView<char> operator ""_sv(const char* str, size_t len) noexcept { return{ str , str + len }; }
    // _sv
    inline PodStringView<wchar_t> operator ""_sv(const wchar_t* str, size_t len) noexcept { return{ str , str + len }; }
    // _sv
    inline PodStringView<char16_t> operator ""_sv(const char16_t* str, size_t len) noexcept { return{ str , str + len }; }
    // _sv
    inline PodStringView<char32_t> operator ""_sv(const char32_t* str, size_t len) noexcept { return{ str , str + len }; }
}

#pragma once

// c++
#include <cstdint>
#include <climits>
// ui
#include "../util/ui_unimacro.h"
#include "../util/ui_endian.h"

// ui
namespace LongUI {
    // RGBA(ABGR in little-endian system, like x86)
    union RGBA {
        // primitive type
        using type = uint32_t;
        // primitive data
        type                primitive;
        // right byte order
        struct  { uint8_t   r, g, b, a; } u8;
    };
    // ColorRGBA
    enum ColorRGBA : RGBA::type;
    // ColorF basic interface
    //struct ColorFBI {  };
    // colorf
    namespace helper {
        // rgba
        constexpr inline RGBA::type rgba(int r, int g, int b, int a) noexcept {
            return 
                RGBA::type(RGBA::type(r) << (CHAR_BIT * color_order::r)) |
                RGBA::type(RGBA::type(g) << (CHAR_BIT * color_order::g)) |
                RGBA::type(RGBA::type(b) << (CHAR_BIT * color_order::b)) |
                RGBA::type(RGBA::type(a) << (CHAR_BIT * color_order::a)) ;
        }
        // rgba
        constexpr inline RGBA::type rgba(uint32_t value) noexcept {
            using c = helper::color_order;
            return
                uint32_t((value >> (CHAR_BIT * 0)) & 0xfful) << (CHAR_BIT * c::r) |
                uint32_t((value >> (CHAR_BIT * 1)) & 0xfful) << (CHAR_BIT * c::g) |
                uint32_t((value >> (CHAR_BIT * 2)) & 0xfful) << (CHAR_BIT * c::b) |
                uint32_t((value >> (CHAR_BIT * 3)) & 0xfful) << (CHAR_BIT * c::a);
        }
    }
    // ColorF
    struct ColorF {
        // rgba
        float r, g, b, a;
        // to rgba
        auto ToRGBA() const noexcept->RGBA;
        // from rgba in run-time
        static void FromRGBA_RT(ColorF& c, RGBA) noexcept;
        // from rgba in compile-time
        template<RGBA::type C>
        static inline ColorF FromRGBA_CT() noexcept {
            ColorF color;
            constexpr float r = GetFloat<0, C>();
            constexpr float g = GetFloat<1, C>();
            constexpr float b = GetFloat<2, C>();
            constexpr float a = GetFloat<3, C>();
            color.r = r; color.g = g; color.b = b; color.a = a;
            return color;
        }
        // get float
        template<RGBA::type SHIFT, RGBA::type COLOR>
        static inline constexpr auto GetFloat() noexcept -> float {
            return static_cast<float>((COLOR & (0xfful << (8 * SHIFT))) >> (8 * SHIFT)) / 255.f;
        }
    };
    // mix color
    auto Mix(const ColorF& from, const ColorF& to, float x) noexcept->ColorF;
}
#include <util/ui_little_math.h>
#include <core/ui_color.h>
#include <emmintrin.h>
#include <algorithm>

PCN_NOINLINE
/// <summary>
/// To the RGBA.
/// </summary>
/// <returns></returns>
auto LongUI::ColorF::ToRGBA() const noexcept -> RGBA {
    // 写入
    RGBA argb;
    argb.u8.r = uint8_t(uint32_t(this->r * 255.f) & 0xFF);
    argb.u8.g = uint8_t(uint32_t(this->g * 255.f) & 0xFF);
    argb.u8.b = uint8_t(uint32_t(this->b * 255.f) & 0xFF);
    argb.u8.a = uint8_t(uint32_t(this->a * 255.f) & 0xFF);
    return argb;
}

PCN_NOINLINE
/// <summary>
/// Froms the RGBA in run time
/// </summary>
/// <param name="color32">The color32.</param>
/// <returns></returns>
void LongUI::ColorF::FromRGBA_RT(ColorF& color, RGBA color32) noexcept {
#ifdef UI_NO_SSE2
    // 位移量
    constexpr RGBA RED_SHIFT = CHAR_BIT * helper::color_order::r;
    constexpr RGBA GREEN_SHIFT = CHAR_BIT * helper::color_order::g;
    constexpr RGBA BLUE_SHIFT = CHAR_BIT * helper::color_order::b;
    constexpr RGBA ALPHA_SHIFT = CHAR_BIT * helper::color_order::a;
    // 掩码
    constexpr RGBA RED_MASK = 0xFFU << RED_SHIFT;
    constexpr RGBA GREEN_MASK = 0xFFU << GREEN_SHIFT;
    constexpr RGBA BLUE_MASK = 0xFFU << BLUE_SHIFT;
    constexpr RGBA ALPHA_MASK = 0xFFU << ALPHA_SHIFT;
    // 计算
    color.r = static_cast<float>((color32.pri & RED_MASK) >> RED_SHIFT) / 255.f;
    color.g = static_cast<float>((color32.pri & GREEN_MASK) >> GREEN_SHIFT) / 255.f;
    color.b = static_cast<float>((color32.pri & BLUE_MASK) >> BLUE_SHIFT) / 255.f;
    color.a = static_cast<float>((color32.pri & ALPHA_MASK) >> ALPHA_SHIFT) / 255.f;
#else
    constexpr float f256_255 = 256.0f / 255.0f;
    constexpr int32_t magic32 = 0x47004700;
    const __m128i magic = _mm_set_epi32(magic32, magic32, magic32, magic32);
    const __m128 i16max = _mm_set_ps(32768.0f, 32768.0f, 32768.0f, 32768.0f);
    const __m128 u8ratio = _mm_set_ps(f256_255, f256_255, f256_255, f256_255);
    // 单个转换其实比上面的慢了 10%, 但是生成的代码少了一半
    // p.s. 同时转换4个就会比上面的快 一倍
    __m128 in1, out1;
    __m128i in, tmplo; reinterpret_cast<uint32_t&>(in) = color32.primitive;
    tmplo = _mm_unpacklo_epi8(in, _mm_set_epi32(0, 0, 0, 0));
    in1 = _mm_castsi128_ps(_mm_unpacklo_epi16(tmplo, magic));
    out1 = _mm_mul_ps(_mm_sub_ps(in1, i16max), u8ratio);
    _mm_storeu_ps(reinterpret_cast<float*>(&color), out1);
#endif
}

PCN_NOINLINE
/// <summary>
/// Mixes the color.
/// </summary>
/// <param name="from">From.</param>
/// <param name="to">To.</param>
/// <param name="x">The x.</param>
/// <returns></returns>
auto LongUI::Mix(const ColorF& from, const ColorF& to, float x) noexcept -> ColorF {
    const auto x0_0 = detail::clamp(x, 0.f, 1.f);
    const auto x0_1 = 1.f - x0_0;
    ColorF rv;
    rv.r = from.r * x0_1 + to.r * x0_0;
    rv.g = from.g * x0_1 + to.g * x0_0;
    rv.b = from.b * x0_1 + to.b * x0_0;
    rv.a = from.a * x0_1 + to.a * x0_0;
    return rv;
}
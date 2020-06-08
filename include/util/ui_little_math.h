#pragma once

// longui::detail namespace
namespace LongUI { namespace detail {
    // clamp for float/double
    template<typename T> inline auto clamp(T v, T lo, T hi) noexcept {
        return (v < lo) ? lo : (hi < v) ? hi : v;
    }
    // is percent value?
    inline auto is_percent_value(float f) noexcept { return f > -1.f && f < 1.f; }
    // get percent value
    inline auto get_percent_value(float f) noexcept { return f * 1e3f; }
    // make percent value from 1.00
    inline auto mark_percent_from1(float f) noexcept { return f * 1e-3f; }
    // make percent value from 100%
    inline auto mark_percent_from100(float f) noexcept { return f * 1e-5f; }
    // make percent value from int 10000
    inline auto mark_percent_from10000(float f) noexcept { return f * 1e-7f; }
}}
#pragma once

// longui::detail namespace
namespace LongUI { namespace detail {
    // clamp for float
    inline auto clamp(float d, float min, float max) noexcept {
        const float t = d < min ? min : d;
        return t > max ? max : t;
    }
    // get percent value
    inline auto get_percent_value(float f) noexcept { return f * 1e3f; }
    // is percent value?
    inline auto is_percent_value(float f) noexcept { return f > -1.f && f < 1.f; }
    // make percent value from 1.00
    inline auto mark_percent_from1(float f) noexcept { return f * 1e-3f; }
    // make percent value from 100%
    inline auto mark_percent_from100(float f) noexcept { return f * 1e-5f; }
    // make percent value from int 10000
    inline auto mark_percent_from10000(float f) noexcept { return f * 1e-7f; }
}}
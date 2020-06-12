#pragma once

// ui::detail namespace
namespace LongUI { namespace detail {
    template<typename T>
    constexpr T log2(T n) noexcept { return (n > 1) ? 1 + log2(n >> 1) : 0; }
}}

#pragma once

// endian
#include "ui_endian.h"
// c++
#include <cstdint>
#include <algorithm>

// ui namespace
namespace LongUI { 
    // [begin, end-1) already be sorted
    template<typename T, typename U>
    void LastSort(T begin, T end, U comp) noexcept {
        if (end - begin < 2) return;
        for (auto itr = end - 1; itr != begin; --itr) {
            if (comp(itr[-1], itr[0])) break;
            std::swap(itr[-1], itr[0]);
        }
    }
    // remove begin to end
    template<typename T>
    void Remove(T begin, T end) noexcept {
        for (auto itr = begin; itr < end - 1; ++itr) {
            std::swap(itr[0], itr[1]);
        }
    }
    // helper namespace
    namespace helper {
        // u8_mask
        template<unsigned SIZE> struct u8u32_mask;
        // uint8_t mask for offset 0
        template<> struct u8u32_mask<0> { enum : uint32_t { value = 0xfful << (color_order::a * 8) }; };
        // uint8_t mask for offset 1
        template<> struct u8u32_mask<1> { enum : uint32_t { value = 0xfful << (color_order::b * 8) }; };
        // uint8_t mask for offset 2
        template<> struct u8u32_mask<2> { enum : uint32_t { value = 0xfful << (color_order::g * 8) }; };
        // uint8_t mask for offset 2
        template<> struct u8u32_mask<3> { enum : uint32_t { value = 0xfful << (color_order::r * 8) }; };
    }
    // sort pointers
    void SortPointers(const void**ptr, const void**end, uint32_t offset, uint32_t mask) noexcept;
}


#pragma once

// c++
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
}


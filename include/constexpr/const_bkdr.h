﻿#pragma once
/**
* Copyright (c) 2014-2020 dustpg   mailto:dustpg@gmail.com
*
* Permission is hereby granted, free of charge, to any person
* obtaining a copy of this software and associated documentation
* files (the "Software"), to deal in the Software without
* restriction, including without limitation the rights to use,
* copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following
* conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
* OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
* HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/

#include <cstdint>
//#include "../util/ui_unimacro.h"

// ui namespace
namespace LongUI {
    // detail namespace
    namespace detail {
        template<uint32_t SEED>
        constexpr uint32_t const_bkdr(uint32_t hash, const char* str) noexcept {
            return *str ? const_bkdr<SEED>(
                static_cast<uint32_t>(static_cast<uint64_t>(hash) * SEED + (*str))
                , str + 1) : hash;
        }
    }
    // Typical BKDR hash function
    constexpr uint32_t TypicalBKDR(const char* str) noexcept {
        return detail::const_bkdr<131>(0, str);
    }
    // BKDR
    constexpr uint32_t operator ""_bkdr(const char* str, size_t) noexcept {
        return TypicalBKDR(str);
    }
}

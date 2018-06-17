#pragma once
/**
* Copyright (c) 2014-2018 dustpg   mailto:dustpg@gmail.com
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

// TextBC namespace
namespace TextBC {
    // text cell
    class CBCTextCell;
    // text line data
    struct TextLineData {
        // y offset
        double          offset;
        // first cell
        CBCTextCell*    first;
        // last cell
        CBCTextCell*    last;
        // char count to here
        uint32_t        char_count;
        // string len to here
        uint32_t        string_len;
        // max height above baseline
        float           max_height1;
        // max height under baseline
        float           max_height2;
        // TODO: max_height1-> max_baseline  max_height2 -> max_height

        // clear line data
        void Clear(const TextLineData& lastline) noexcept;
        // add cell
        void operator +=(CBCTextCell&) noexcept;
        // calculate line width ex
        static auto CalculateWidth(const CBCTextCell*, const CBCTextCell*) noexcept ->float;
        // calculate line height ex
        static auto CalculateHeight(const CBCTextCell*, const CBCTextCell*) noexcept ->float;
        // calculate line width
        auto CalculateWidth() const noexcept ->float {
            return this->CalculateWidth(this->first, this->last);
        }
    };
    
}
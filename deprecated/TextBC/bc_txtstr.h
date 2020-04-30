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

#include <cassert>
#include <cstdint>

// TextBC namespace
namespace TextBC {
    // fixed length
#ifdef NDEBUG
    enum { TEXT_CELL_NICE_LENGTH = 40, TEXT_CELL_NICE_MAX = 40 };
#else
    enum { TEXT_CELL_NICE_LENGTH = 10, TEXT_CELL_NICE_MAX = 10 };
#endif
    // fixed-length string
    class CBCString {
    public:
        // ctor
        CBCString() noexcept { m_data[0] = 0; }
        // dtor
        ~CBCString() noexcept {}
    public: // STL like interface
        // get length
        auto size() const noexcept { return m_length; }
        // get c-style string
        auto c_str() const noexcept { return m_data; }
        // resize
        void resize(uint32_t s) noexcept { m_length = s; this->mark_eos(); }
        // oprator []
        auto&operator[](uint32_t i) noexcept { assert(i<TEXT_CELL_NICE_MAX); return m_data[i]; }
        // oprator []
        auto&operator[](uint32_t i) const noexcept { assert(i<TEXT_CELL_NICE_MAX); return m_data[i]; }
        // erase string
        void erase(uint32_t pos, uint32_t len) noexcept;
        // insert string
        void insert(uint32_t pos, const char16_t* str, uint32_t len) noexcept;
    private:
        // mark end-of-string
        void mark_eos() noexcept { assert(m_length < TEXT_CELL_NICE_MAX+2); m_data[m_length] = 0; }
    private:
        // string length
        uint32_t            m_length = 0;
        // string data(1 for null 1 for surrogate )
        char16_t            m_data[TEXT_CELL_NICE_MAX + 2];
    };
}
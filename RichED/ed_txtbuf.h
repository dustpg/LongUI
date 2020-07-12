#pragma once
/**
* Copyright (c) 2018-2019 dustpg   mailto:dustpg@gmail.com
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
#include <type_traits>

// RichED namespace
namespace RichED {
    // platform
    struct IEDTextPlatform;
    // detail namespace
    namespace impl {
        // buffer base class
        class buffer_base {
        protected:
            // ctor
            buffer_base() noexcept;
            // dtor
            ~buffer_base() noexcept;
        protected:
            // resize buffer
            bool resize_buffer(uint32_t, size_t size_of, IEDTextPlatform&) noexcept;
            // is ok?
            bool is_ok() const noexcept { return !!m_data; }
            // is failed?
            bool is_failed() const noexcept { return !m_data; }
            // assert index
            void assert_index(uint32_t i) const noexcept { assert(i < m_length); }
            // assert size
            void assert_size(uint32_t size) const noexcept { assert(size <= m_length); }
            // clear
            void clear() noexcept { m_length = 0; }
        protected:
            // data pointer
            void*               m_data = nullptr;
            // length of data
            uint32_t            m_length = 0;
            // capacity of data
            uint32_t            m_capacity = 0;
        };
    }
    // buffer class
    template<typename T> class CEDBuffer: public impl::buffer_base {
        // tptr / const tptr
        using tptr = T * ; using cptr = const T*;
        // must be pod
        static_assert(std::is_pod<T>::value, "must be pod");
        // must be pod
        static_assert(alignof(T) <= alignof(double), "'alignof' must be less than double");
        // tr pointer
        static tptr tr(void* ptr) noexcept { return reinterpret_cast<T*>(ptr); }
        // tr pointer
        static cptr ct(void* ptr) noexcept { return reinterpret_cast<const T*>(ptr); }
    public:
        // ctor
        CEDBuffer() noexcept {}
        // dtor
        ~CEDBuffer() noexcept {}
        // resize
        bool Resize(uint32_t l, IEDTextPlatform& p) noexcept { return this->resize_buffer(l, sizeof(T), p); }
        // reduce size
        void ReduceSize(uint32_t l) noexcept { this->assert_size(l); m_length = l; }
        // full
        auto IsFull() const noexcept { return m_length == m_capacity; }
        // get size
        auto GetSize() const noexcept { return m_length; }
        // get data
        auto GetData() noexcept { return tr(m_data); }
        // is ok
        bool IsOK() const noexcept { return this->is_ok(); }
        // is failed
        bool IsFailed() const noexcept { return this->is_failed(); }
        // clear
        void Clear() noexcept { this->clear(); }
    public:
        // operaotr[]
        auto&operator[](uint32_t index) noexcept { return this->at(index); }
        // operaotr[]
        auto&operator[](uint32_t index) const noexcept { return this->cat(index); }
        // begin pointer
        auto begin() noexcept { return tr(m_data); }
        // end pointer
        auto end() noexcept { return tr(m_data) + m_length; }
        // begin pointer
        auto begin() const noexcept { return ct(m_data); }
        // end pointer
        auto end() const noexcept { return ct(m_data) + m_length; }
    private:
        // get at
        auto&at(uint32_t i) noexcept { assert_index(i); return tr(m_data)[i]; }
        // get const at
        const auto&cat(uint32_t i) const noexcept { assert_index(i); return tr(m_data)[i]; }
    };
}
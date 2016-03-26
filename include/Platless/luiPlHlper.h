#pragma once
/**
* Copyright (c) 2014-2016 dustpg   mailto:dustpg@gmail.com
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

// longui::helper namespace
namespace LongUI { namespace Helper {
    // to match ruby-style, use low-case char
    // type helper with c++ tuple
    template <typename T> struct type_helper : public type_helper<decltype(&T::operator())> {};
    // type helper
    template <typename ClassType, typename ReturnType, typename... Args>
    struct type_helper<ReturnType(ClassType::*)(Args...) const> {
        // number of arguments
        enum : size_t { arity = sizeof...(Args) };
        // return type
        using result_type = ReturnType;
        // arg type
        template <size_t i> struct arg { using type = typename std::tuple_element<i, std::tuple<Args...>>::type; };
    };
    // make floats from string
    bool MakeFloats(const char* str, float fary[], uint32_t count) noexcept;
    // make ints from string
    bool MakeInts(const char* str, int fary[], uint32_t count) noexcept;
    // Bit Array 计算机中每一字节都很宝贵
    template<typename T> class BitArray {
    public:
        // bit wide
        static constexpr size_t LENGTH = sizeof(T) * CHAR_BIT;
        // ctor
        BitArray() noexcept {};
        // ctor
        BitArray(T v) noexcept : m_data(v) {};
        // dtor
        ~BitArray() noexcept {};
        // is true or fasle
        auto Test(uint32_t index) const noexcept { assert(index<LENGTH); return !!(m_data & (1 << index)); }
        // set to true
        auto SetTrue(uint32_t index) noexcept { assert(index<LENGTH); m_data |= (1 << index); };
        // set to false
        auto SetFalse(uint32_t index) noexcept { assert(index<LENGTH); m_data &= ~(1 << index); };
        // set to NOT
        auto SetNot(uint32_t index) noexcept { assert(index<LENGTH); m_data ^= (1 << index); };
        // set to???
        template<typename V>
        auto SetTo(uint32_t index, V value) noexcept { assert(index<LENGTH); value ? this->SetTrue(index) : this->SetFalse(index); }
    private:
        // data for bit-array
        T           m_data = T(0);
    };
    // 特例化
    using BitArray_8 = BitArray<uint8_t>;
    using BitArray16 = BitArray<uint16_t>;
    using BitArray32 = BitArray<uint32_t>;
    using BitArray64 = BitArray<uint64_t>;
    // data 放肆!450交了么!
    constexpr size_t  INFOPDATA12_ZONE = (size_t(3));
    constexpr size_t  INFOPOINTER_ZONE = ~INFOPDATA12_ZONE;
    constexpr size_t  INFOPTDATA1_ZONE = ~(size_t(2));
    constexpr size_t  INFOPTDATA2_ZONE = ~(size_t(1));
    // Infomation-ed pointer  计算机中每一字节都很宝贵
    template<typename T>
    class InfomationPointer {
    public:
        // constructor
        InfomationPointer(T* pointer) :data(reinterpret_cast<size_t>(pointer)) { assert(!(data&INFOPDATA12_ZONE)); }
        // copy constructor
        InfomationPointer(const InfomationPointer&) = delete;
        // move constructor
        InfomationPointer(InfomationPointer&&) = delete;
        // operator =
        T* operator=(T* pt) { assert(!(data&INFOPDATA12_ZONE)); data = reinterpret_cast<size_t>(pt) | (data&INFOPDATA12_ZONE); return pt; }
        // operator ->
        T* operator->() noexcept { return reinterpret_cast<T*>(data & INFOPOINTER_ZONE); }
        // operator *
        T& operator*() noexcept { return *reinterpret_cast<T*>(data & INFOPOINTER_ZONE); }
        // operator T*
        operator T*() const noexcept { return reinterpret_cast<T*>(data & INFOPOINTER_ZONE); }
        // operator []
        T& operator [](const int index) noexcept { return (reinterpret_cast<T*>(data & INFOPOINTER_ZONE))[index]; }
        // operator [] const ver.
        const T& operator [](const int index) const noexcept { return (reinterpret_cast<T*>(data & INFOPOINTER_ZONE))[index]; }
        // pointer
        T* Ptr() const noexcept { return reinterpret_cast<T*>(data & INFOPOINTER_ZONE); }
        // bool1
        bool Bool1() const noexcept { return (data & (1 << 0)) > 0; }
        // bool2
        bool Bool2() const noexcept { return (data & (1 << 1)) > 0; }
        // set bool1
        void SetBool1(bool b) noexcept { data = (data & INFOPTDATA2_ZONE) | size_t(b); }
        // bool2
        void SetBool2(bool b) noexcept { data = (data & INFOPTDATA1_ZONE) | (size_t(b) << 1); }
        // SafeRelease if keep a Relase() interface(COM like)
        void SafeRelease() noexcept { T* t = reinterpret_cast<T*>(data & INFOPOINTER_ZONE); if (t) { t->Release(); data &= INFOPDATA12_ZONE; } }
    private:
        // pointer & boolx2 data
        size_t          data;
    };
}}


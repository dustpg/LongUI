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

#include "../luiconf.h"
#include <cstdint>
#include <cassert>
#include <new>

// longui namespace
namespace LongUI {
    // white space
    template<typename T> inline auto white_space(T c) noexcept { return ((c) == ' ' || (c) == '\t'); }
    // valid digit
    template<typename T> inline auto valid_digit(T c) noexcept { return ((c) >= '0' && (c) <= '9'); }
    // 0xD800 <= ch <= 0xDFFF
    inline bool IsSurrogate(char16_t ch) noexcept { return (ch & 0xF800) == 0xD800; }
    // 0xD800 <= ch <= 0xDBFF
    inline bool IsHighSurrogate(char16_t ch) noexcept { return (ch & 0xFC00) == 0xD800; }
    // 0xDC00 <= ch <= 0xDFFF
    inline bool IsLowSurrogate(char16_t ch) noexcept { return (ch & 0xFC00) == 0xDC00; }
    // hex -> int
    unsigned int Hex2Int(char c) noexcept;
    // lengthof
    template<typename R, typename T> constexpr auto lengthof(T& t) { return static_cast<R>(sizeof(t) / sizeof(*t)); }
    // lengthof
    template<typename T> constexpr auto lengthof(T& t) { return sizeof(t) / sizeof(*t); }
    // byte distance
    template<typename T, typename Y> auto bdistance(T* a, T* b) noexcept { reinterpret_cast<const char*>(b) - reinterpret_cast<const char*>(a); };
    // BKDR Hash
    auto BKDRHash(const char* str) noexcept ->uint32_t;
    // BKDR Hash
    auto BKDRHash(const wchar_t* str) noexcept ->uint32_t;
    // BKDR Hash
    inline auto BKDRHash(const char* str, uint32_t size) noexcept { return BKDRHash(str) % size; }
    // BKDR Hash
    inline auto BKDRHash(const wchar_t* str, uint32_t size) noexcept { return BKDRHash(str) % size; }
    // is 2 power?
    inline constexpr auto Is2Power(const size_t x) noexcept { return (x & (x - 1)) == 0; }
    // round
    inline auto RoundToInt(float x) noexcept { return static_cast<int>(x + .5f); }
    // safe buffer
    template<typename T, size_t BUFFER ,typename Lambda>
    void SafeBuffer(size_t buflen, Lambda lam) noexcept(noexcept(lam.operator()))  {
        T fixedbuf[BUFFER]; T* buf = fixedbuf;
        if (buflen > BUFFER) buf = LongUI::NormalAllocT<T>(buflen);
        if (!buf) return; lam(buf);
        if (buf != fixedbuf) LongUI::NormalFree(buf);
    }
    // safe buffer
    template<typename T, typename Lambda>
    void SafeBuffer(size_t buflen, Lambda lam) noexcept(noexcept(lam.operator()))  {
        LongUI::SafeBuffer<T, LongUIStringBufferLength>(buflen, lam);
    }
    // std::atoi diy version
    auto AtoI(const char* __restrict) noexcept -> int;
    // std::atoi diy version overload for wchar_t
    auto AtoI(const wchar_t* __restrict) noexcept -> int;
    // std::atof diy version(float ver)
    auto AtoF(const char* __restrict) noexcept -> float;
    // std::atof diy version(float ver) overload for wchar_t
    auto AtoF(const wchar_t* __restrict) noexcept -> float;
    // UTF-32 to UTF-16 char
    auto Char32toChar16(char32_t ch, char16_t* str) -> char16_t*;
    // UTF-32 to UTF-32 char
    auto UTF8ChartoChar32(const char* ) -> char32_t;
    // UTF-16 to UTF-8: Return end of utf8  string
    auto UTF16toUTF8(const char16_t* __restrict src, char* __restrict des, uint32_t buflen) noexcept -> char*;
    // UTF-8 to UTF-16: Return end of utf16 string
    auto UTF8toUTF16(const char* __restrict src, char16_t* __restrict des, uint32_t buflen) noexcept -> char16_t*;
    // get buffer length for UTF-16 to UTF-8(include NULL-END char)
    auto UTF16toUTF8GetBufLen(const char16_t* src) noexcept -> uint32_t;
    // get buffer length for UTF-8 to UTF-16(include NULL-END char)
    auto UTF8toUTF16GetBufLen(const char* src) noexcept -> uint32_t;
    // get buffer length for wchar to UTF-8(not include NULL-END char)
    inline auto WideChartoUTF8GetBufLen(const wchar_t* src) noexcept {
        static_assert(sizeof(wchar_t) == sizeof(char16_t), "change UTF-16 to UTF-32");
        return LongUI::UTF16toUTF8GetBufLen(reinterpret_cast<const char16_t*>(src));
    }
    // get buffer length for UTF-8 to wchar(not include NULL-END char)
    inline auto UTF8toWideCharGetBufLen(const char* src) noexcept {
        static_assert(sizeof(wchar_t) == sizeof(char16_t), "change UTF-16 to UTF-32");
        return LongUI::UTF8toUTF16GetBufLen(src);
    }
    // safe wchar to UTF-8
    template<size_t BUFFER, typename Lambda> 
    inline void SafeWideChartoUTF8(const wchar_t* src, Lambda lam) noexcept {
        auto len = LongUI::WideChartoUTF8GetBufLen(src);
        LongUI::SafeBuffer<char, BUFFER>(len, [len, src, lam](char* buf) noexcept {
            static_assert(sizeof(wchar_t) == sizeof(char16_t), "change UTF-16 to UTF-32");
            auto end = LongUI::UTF16toUTF8(reinterpret_cast<const char16_t*>(src), buf, len);
            0[end] = 0;
            lam(buf, end);
        });
    }
    // safe wchar to UTF-8
    template<typename Lambda> 
    inline void SafeWideChartoUTF8(const wchar_t* src, Lambda lam) noexcept {
        return LongUI::SafeWideChartoUTF8<LongUIStringBufferLength>(src, lam);
    }
    // safe  UTF-8 to wchar
    template<size_t BUFFER, typename Lambda> 
    inline void SafeUTF8toWideChar(const char* src, Lambda lam) noexcept {
        auto len = LongUI::UTF8toWideCharGetBufLen(src);
        LongUI::SafeBuffer<wchar_t, BUFFER>(len, [len, src, lam](wchar_t* buf) noexcept {
            static_assert(sizeof(wchar_t) == sizeof(char16_t), "change UTF-16 to UTF-32");
            auto end = LongUI::UTF8toUTF16(src, reinterpret_cast<char16_t*>(buf), len);
            0[end] = 0;
            lam(buf, reinterpret_cast<wchar_t*>(end));
        });
    }
    // safe  UTF-8 to wchar
    template<typename Lambda> 
    inline void SafeUTF8toWideChar(const char* src, Lambda lam) noexcept {
        return LongUI::SafeUTF8toWideChar<LongUIStringBufferLength>(src, lam);
    }
    // wchar to UTF-8, unsafe way
    inline auto WideChartoUTF8(const wchar_t* __restrict src, char* __restrict des, uint32_t buflen) noexcept {
        // UTF-8 UTF-16 UTF-32(UCS-4)
        static_assert(sizeof(wchar_t) == sizeof(char16_t), "change UTF-16 to UTF-32");
        return UTF16toUTF8(reinterpret_cast<const char16_t*>(src), des, buflen);
    }
    // UTF-8 to wchar, unsafe way
    inline auto UTF8toWideChar(const char* __restrict src, wchar_t* __restrict des, uint32_t buflen) noexcept {
        // UTF-8 UTF-16 UTF-32(UCS-4)
        static_assert(sizeof(wchar_t) == sizeof(char16_t), "change UTF-16 to UTF-32");
        return UTF8toUTF16(src, reinterpret_cast<char16_t*>(des), buflen);
    }
    // bubble sort for vector or list, you should include <algorithm> first
    template<typename Itr, typename Lamda>
    void BubbleSort(Itr begin, Itr end, Lamda lam) noexcept {
        --end;
        bool flag = true;
        while (flag) {
            flag = false;
            for (auto i = begin; i != end; ++i) {
                auto j = i; ++j;
                if (lam(*j, *i)) {
                    std::swap(*j, *i);
                    flag = true;
                }
            }
            --end;
        }
    }
    // small single object
    struct CUISingleObject {
        // throw new []
        auto operator new(size_t size) ->void* = delete;
        // throw new []
        auto operator new[](size_t size) ->void* = delete;
        // delete []
        void operator delete[](void*, size_t size) noexcept = delete;
    };
    // small single object
    struct CUISingleNormalObject : CUISingleObject {
        // nothrow new 
        auto operator new(size_t size, const std::nothrow_t&) noexcept ->void*{ return LongUI::NormalAlloc(size); };
        // nothrow delete 
        auto operator delete(void* address, const std::nothrow_t&) ->void { return LongUI::NormalFree(address); }
        // delete
        auto operator delete(void* address) noexcept ->void { return LongUI::NormalFree(address); }
    };
    // small single object
    struct CUISingleSmallObject : CUISingleObject {
        // nothrow new 
        auto operator new(size_t size, const std::nothrow_t&) noexcept ->void*{ return LongUI::SmallAlloc(size); };
        // delete
        auto operator delete(void* address) noexcept ->void { return LongUI::SmallFree(address); }
    };
    // BaseFunc
    template<typename Result, typename ...Args>
    class XUIBaseFunc : public CUISingleSmallObject {
    public:
        // call
        virtual auto Call(Args... args) noexcept ->Result = 0;
        // dtor
        virtual ~XUIBaseFunc() noexcept { if (this->chain) delete this->chain; this->chain = nullptr; };
        // call chain
        XUIBaseFunc*        chain = nullptr;
    };
    // RealFunc
    template<typename Func, typename Result, typename ...Args>
    class CUIRealFunc final : public XUIBaseFunc<Result, Args...> {
        // func data
        Func                m_func;
    public:
        // ctor
        CUIRealFunc(const Func &x) noexcept : m_func(x) {}
        // call
        auto Call(Args... args) noexcept ->Result override { 
            if (this->chain) this->chain->Call(args...);
            return m_func(args...);
        }
        // dtor
        virtual ~CUIRealFunc() noexcept = default;
    };
    // type helper
    template<typename Func> struct type_helper { 
        using type = Func; 
    };
    // type helper
    template<typename Result, typename ...Args> struct type_helper<Result(Args...)> { 
        using type = Result (*)(Args...);
    };
    // UI Function, lightweight and chain-call-able version std::function
    template<typename Result, typename ...Args>
    class CUIFunction<Result(Args...)> {
        // this type
        using MyType = CUIFunction<Result(Args...)>;
        // RealFunc pointer
        XUIBaseFunc<Result, Args...>*   m_pFunction = nullptr;
        // release
        void release() noexcept { if (m_pFunction) delete m_pFunction; m_pFunction = nullptr; }
    public:
        // Ok
        auto IsOK() const noexcept { return !!m_pFunction; }
        // dtor
        ~CUIFunction() noexcept { this->release(); }
        // ctor
        CUIFunction() noexcept = default;
        // move ctor
        CUIFunction(MyType&& obj) noexcept : m_pFunction(obj.m_pFunction) { assert(&obj != this && "bad move"); obj.m_pFunction = nullptr; };
        // no copy ctor
        CUIFunction(const MyType&) = delete;
        // and call chain
        auto AddCallChain(MyType&& chain) { 
            if (chain.IsOK()) {
                chain.m_pFunction->chain = m_pFunction;
                m_pFunction = chain.m_pFunction;
                chain.m_pFunction = nullptr;
            }
            else {
                assert(!"error");
            }
        }
        // and call chain
        auto& operator += (MyType&& chain) { this->AddCallChain(std::move(chain)); return *this; }
        // and call chain
        template<typename Func> 
        auto& operator += (const Func &x) { this->AddCallChain(std::move(CUIFunction(x))); return *this; }
        // opeator =
        template<typename Func> auto& operator=(const Func &x) noexcept {
            this->release();
            m_pFunction = new(std::nothrow) CUIRealFunc<type_helper<Func>::type, Result, Args...>(x);
            return *this;
        }
        // opeator =
        MyType& operator=(const MyType &x) noexcept = delete;
        // ctor with func
        template<typename Func> CUIFunction(const Func& f) noexcept : 
        m_pFunction(new(std::nothrow) CUIRealFunc<type_helper<Func>::type, Result, Args...>(f))  {}
        // () operator
        auto operator()(Args... args) const noexcept { assert(m_pFunction && "bad call or oom"); return m_pFunction ? m_pFunction->Call(args...) : Result(); }
    };
    // easing func f(x)
    auto EasingFunction(AnimationType type, float x) noexcept ->float;
}


// the _longui32 后缀
constexpr uint32_t operator"" _longui32(const char* src, size_t len) {
    return len == 2 ?
        static_cast<uint32_t>(src[0]) << (8 * 0) |
        static_cast<uint32_t>(src[1]) << (8 * 1) :
        static_cast<uint32_t>(src[0]) << (8 * 0) |
        static_cast<uint32_t>(src[1]) << (8 * 1) |
        static_cast<uint32_t>(src[2]) << (8 * 2) |
        static_cast<uint32_t>(src[3]) << (8 * 3);
}


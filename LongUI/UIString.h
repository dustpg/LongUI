#pragma once
/**
* Copyright (c) 2014-2015 dustpg   mailto:dustpg@gmail.com
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

// LongUI namespace
namespace LongUI{
    // UI String -- compatible with std library string interface(part of) but static
    class CUIString {
    public:
        // 默认构造函数
        CUIString() noexcept { *m_aDataStatic = 0; }
        // 字符串构造函数
        CUIString(const wchar_t* str, uint32_t l = 0) noexcept { this->Set(str, l); }
        // 析构函数
        ~CUIString() noexcept;
        // 复制构造函数
        CUIString(const CUIString&) noexcept;
        // 移动构造函数
        CUIString(CUIString&&) noexcept;
    public:
        // set/assign
        void Set(const wchar_t* str, uint32_t = 0) noexcept;
        // append
        void Append(const wchar_t* str, uint32_t = 0) noexcept;
        // format
        void Format(const wchar_t* format, ...) noexcept;
        // On Out of Memory
        void OnOOM() noexcept;
    public: // std::string compatibled interface/method
        // get data overload for const
        auto* data() noexcept { return m_pString; }
        // get data length
        auto length() const noexcept { return m_cLength; }
        // get size
        auto size() const noexcept { return m_cLength; }
        // get data
        const auto* data() const noexcept { return m_pString; }
        // c style string
        const auto* c_str() const noexcept { return m_pString; }
    public: // std::string compatibled interface/method
        // assign: overload for [const wchar_t*]
        auto assign(const wchar_t* str, uint32_t count = 0) noexcept { return this->Set(str, count); }
        // assign: overload for [const wchar_t]
        auto assign(const wchar_t ch) noexcept { wchar_t buf[2] = { ch, 0 }; return this->Set(buf, 1); }
        // assign: overload for [const char]
        auto assign(const char ch) noexcept { wchar_t buf[2] = { wchar_t(ch), 0 }; return this->Set(buf, 1); }
        // = 操作: overload for [const wchar_t*]
        auto&operator= (const wchar_t* s) noexcept { this->assign(s); return *this; }
        // = 操作: overload for [const wchar_t]
        auto&operator= (const wchar_t ch) noexcept { this->assign(ch); return *this; }
        // = 操作: overload for [const char]
        auto&operator= (const char ch) noexcept { this->assign(ch); return *this; }
    public: // std::string compatibled interface/method
        // append overload for [const wchar_t*]
        auto append(const wchar_t* str, uint32_t count = 0) noexcept { return this->Append(str, count); }
        // append overload for [const wchar_t]
        auto append(const wchar_t ch) noexcept { wchar_t buf[2] = { ch, 0 }; return this->Append(buf, 1); }
        // append overload for [const char]
        auto append(const char ch) noexcept { wchar_t buf[2] = { wchar_t(ch), 0 }; return this->Append(buf, 1); }
        // += 操作 overload for [const wchar_t*]
        auto&operator+=(const wchar_t* s) noexcept { this->append(s); return *this; }
        // += 操作 overload for [const wchar_t]
        auto&operator+=(const wchar_t c) noexcept { this->append(c); return *this; }
        // += 操作 overload for [const char]
        auto&operator+=(const char c) noexcept { this->append(c); return *this; }
        // += 操作 overload for [longui string]
        auto&operator+=(const CUIString& str) noexcept { this->append(str.c_str(), str.length()); return *this; }
    public:
        // == 操作
        bool operator==(const wchar_t* str) const noexcept { return (std::wcscmp(m_pString, str) == 0); };
        // != 操作
        bool operator!=(const wchar_t* str) const noexcept { return (std::wcscmp(m_pString, str) != 0); };
        // <= 操作
        bool operator<=(const wchar_t* str) const noexcept { return (std::wcscmp(m_pString, str) <= 0); };
        // < 操作
        bool operator< (const wchar_t* str) const noexcept { return (std::wcscmp(m_pString, str) < 0); };
        // >= 操作
        bool operator>=(const wchar_t* str) const noexcept { return (std::wcscmp(m_pString, str) >= 0); };
        // > 操作
        bool operator> (const wchar_t* str) const noexcept { return (std::wcscmp(m_pString, str) > 0); };
        // 转换为 wchar_t*
        operator const wchar_t*()const noexcept { return m_pString; }
    private:
        // 字符串
        wchar_t*            m_pString = m_aDataStatic;
        // 字符串长度
        uint32_t            m_cLength = 0;
        // 缓冲区长度
        uint32_t            m_cBufferLength = LongUIStringLength;
        // 静态数据
        wchar_t             m_aDataStatic[LongUIStringLength];
    private:
        // free buffer in safe way
        auto safe_free_bufer() {
            if (m_pString && m_pString != m_aDataStatic) {
                LongUI::SmallFree(m_pString);
            }
            m_pString = nullptr;
        }
        // alloc buffer in safe way
        static inline auto alloc_bufer(uint32_t buffer_length) {
            register size_t length = sizeof(wchar_t) * size_t(buffer_length);
            return reinterpret_cast<wchar_t*>(LongUI::SmallAlloc(length));
        }
        // copy string
        static auto copy_string(wchar_t* des, const wchar_t* src, uint32_t length) {
            ::memcpy(des, src, sizeof(wchar_t) * (length + 1));
        }
        // choose a nice length for buffer
        static auto nice_buffer_length(uint32_t target) {
            auto sizex2 = sizeof(void*) * 2;
            auto temp = target + (target + LongUIStringLength) / 2 + sizex2;
            return temp - temp % sizex2;
        }
    };
}
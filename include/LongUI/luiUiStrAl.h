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

#include "../luibase.h"
#include "../luiconf.h"
#include <cassert>

#ifdef _MSC_VER
#pragma warning(disable: 4200)
#endif
// longui namespace
namespace LongUI {
    // short allocator, memory created with allocator, destroyed with allocator
    class CUIShortStringAllocator {
        // CHAIN
        struct CHAIN { CHAIN* next; size_t used; char buffer[0]; };
        // buffer length
        enum : size_t { CHAIN_SIZE = 2048, BUFFER_SIZE = CHAIN_SIZE - sizeof(void*) * 2 };
        // memory chain
        CHAIN*              m_pHeader = nullptr;
    public:
        // ctor
        CUIShortStringAllocator() noexcept = default;
        // cpoy ctor
        CUIShortStringAllocator(const CUIShortStringAllocator&) noexcept = default;
        // dtor
        ~CUIShortStringAllocator() noexcept {
            auto node = m_pHeader;
            while (node) {
                auto tmp = node;
                node = node->next;
                LongUI::NormalFree(tmp);
            }
            m_pHeader = nullptr;
        }
    public:
        // free
        auto Free(void* address) noexcept { }
        // alloc for normal buffer
        auto Alloc(size_t len) noexcept { return this->alloc(len); }
        // alloc string with length
        template<typename T>
        const T* Alloc(const T* str, size_t len) noexcept {
            assert(str);
            // align operation
            auto buffer = reinterpret_cast<T*>(this->alloc(sizeof(T)*(len + 1)));
            if (!buffer) return nullptr;
            std::memcpy(buffer, str, sizeof(T) * len);
            buffer[len] = 0;
            return buffer;
        }
        // alloc string
        auto CopyString(const char* str) noexcept { return this->Alloc(str, std::strlen(str)); }
        // alloc w-string
        auto CopyString(const wchar_t* str) noexcept { return this->Alloc(str, std::wcslen(str)); }
    private:
        // reserve
        auto reserve(size_t len) noexcept ->CHAIN* {
            assert(len < BUFFER_SIZE && "out of range");
            // check
            if (!m_pHeader || (m_pHeader->used + len) > BUFFER_SIZE) {
                auto new_header = reinterpret_cast<CHAIN*>(LongUI::NormalAlloc(CHAIN_SIZE));
                if (!new_header) return nullptr;
                new_header->next = m_pHeader;
                new_header->used = 0;
                m_pHeader = new_header;
            }
            return m_pHeader;
        }
        // alloc buffer
        LongUINoinline auto alloc(size_t len) noexcept ->void* {
            assert(len < BUFFER_SIZE && "bad action");
            if (len >= BUFFER_SIZE) {
                return nullptr;
            }
            void* address = nullptr;
            auto chian = this->reserve(len);
            if (chian) {
                address = chian->buffer + chian->used;
                chian->used += len;
            }
            return address;
        }
    };
}

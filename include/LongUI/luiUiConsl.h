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
#include <Windows.h>
#include <cstdint>

// longui namespace
namespace LongUI {
    // XXX: Console for App, 
    class CUIConsole {
    public:
        // config
        struct Config {
            // ctor
            Config() noexcept { position_xy = uint32_t(-1); };
            // buffer X
            int32_t         buffer_size_x = -1;
            // buffer Y
            int32_t         buffer_size_y = -1;
            // position XY
            union {
                // U32
                uint32_t    position_xy;
                // I16 * 2
                struct {
                    // X pos
                    int16_t x;
                    // Y pos
                    int16_t y;
                };
            };
            // atribute
            uint32_t        atribute = 0;
            // logger name
            const wchar_t*  logger_name = nullptr;
            // logger name
            const wchar_t*  helper_executable = nullptr;
        };
        // ctor
        CUIConsole() noexcept;
        // ctor
        ~CUIConsole() noexcept;
    public:
        // bool
        operator bool() const noexcept { return m_hConsole != INVALID_HANDLE_VALUE || !m_hConsole; }
        // create a new console foe this app
        long Create(const wchar_t*, Config& config) noexcept;
        // close this
        long Close() noexcept;
        // output the string
        long Output(const wchar_t* str, bool flush, size_t len) noexcept;
        // output the string
        long Output(const wchar_t* str, bool flush) noexcept;// { return this->Output(str, flush, std::wcslen(str)); }
    protected:
        // handle for console
        HANDLE              m_hConsole = INVALID_HANDLE_VALUE;
        // current length
        size_t              m_length = 0;
        // lock
        //CRITICAL_SECTION    m_cs;
        // name for this
        wchar_t             m_name[64];
        // buffer
        wchar_t             m_buffer[1024];
    };
}


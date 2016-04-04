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

// longui namespace
namespace LongUI {
    // file
    class CUIFile final {
        // file
        struct File;
    public:
        // open flag
        enum OpenFlag : uint32_t {
            // read
            Flag_Read = 1 << 0,
            // write
            Flag_Write = 1 << 1,
            // create always
            Flag_CreateAlways = 1 << 2,
            // create new
            Flag_CreateNew = 1 << 3,
        };
        // seek type
        enum TypeOfSeek : uint32_t {
            // begin of file
            Seek_Begin      = 0,
            // current of file
            Seek_Current    = 1,
            // end of file
            Seek_End        = 2
        };
        // handle to file
        using FileHandle = File*;
        // ctor
        CUIFile(const wchar_t* namefile, OpenFlag) noexcept;
        // dtor
        ~CUIFile() noexcept;
        // ok?
        bool IsOk() const noexcept;
        // get file size
        auto GetFilezize() const noexcept -> uint32_t;
        // tell, tell position of file
        auto Tell() const noexcept -> uint32_t;
        // seek, set file pointer
        auto Seek(int32_t offset, TypeOfSeek seek) const noexcept -> uint32_t;
        // read file, return byte read
        auto Read(void* buffer, uint32_t bufread) const noexcept ->uint32_t;
        // write file, return byte writed
        auto Write(void* buffer, uint32_t bufwrite) noexcept ->uint32_t;
    private:
        // file handle
        FileHandle          m_hFile = nullptr;
    };
    // define flag
    LONGUI_DEFINE_ENUM_FLAG_OPERATORS(CUIFile::OpenFlag, uint32_t);

}


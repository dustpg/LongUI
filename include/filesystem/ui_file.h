#pragma once
#include <cstdint>
#include "../util/ui_unimacro.h"

// utf-16 based filename file

namespace LongUI {
    /// <summary>
    /// file
    /// </summary>
    class CUIFile {
    public:
        // error handle
        enum : intptr_t { ERROR_HANDLE = -1 };
        // open flag
        enum OpenFlag : uint32_t {
            // read
            Flag_Read = 1 << 0,
            // write
            Flag_Write = 1 << 1,
            // create always
            Flag_CreateAlways = 1 << 2,
            // open always, create if not exsit
            Flag_OpenAlways = 1 << 3,
        };
        // seek type
        enum TypeOfSeek : uint32_t {
            // begin of file
            Seek_Begin = 0,
            // current of file
            Seek_Current = 1,
            // end of file
            Seek_End = 2
        };
    private:
        // utf-8 file name
        enum : uint32_t { Flag_UTF8FileName = 1ul << 31 };
    public:
        // ctor
        CUIFile(const char16_t* namefile, OpenFlag) noexcept;
        // ctor
        CUIFile(const char* namefile, OpenFlag) noexcept;
        // dtor
        ~CUIFile() noexcept;
        // ok?
        bool IsOk() const noexcept { return m_hFile != ERROR_HANDLE; }
        // ok?
        bool IsOK() const noexcept { return m_hFile != ERROR_HANDLE; }
        // ok?
        operator bool() const noexcept { return this->IsOk(); }
        // tell, tell position of file
        auto Tell() const noexcept->uint32_t;
        // get file size less than 4GB
        auto GetFilezize() const noexcept->uint32_t;
        // seek, set file pointer less than 2GB
        auto Seek(int32_t offset, TypeOfSeek seek) const noexcept->uint32_t;
        // read file, return byte read
        auto Read(void* buffer, uint32_t bufread) const noexcept->uint32_t;
        // write file, return byte writed
        auto Write(const void* buffer, uint32_t bufwrite) noexcept->uint32_t;
        // get handle
        void*GetHandle() const noexcept { return reinterpret_cast<void*>(m_hFile); }
    private:
        // ctor
        CUIFile(OpenFlag, const char16_t* namefile) noexcept;
        // file handle
        intptr_t        m_hFile = ERROR_HANDLE;
    };
    // operator | for OF
    constexpr CUIFile::OpenFlag operator|(CUIFile::OpenFlag a, CUIFile::OpenFlag b) noexcept {
        return static_cast<CUIFile::OpenFlag>(static_cast<uint32_t>(a) | b);
    }
    // ctor with wchar file name
    inline CUIFile::CUIFile(const char16_t* n, OpenFlag f) noexcept :
        CUIFile(static_cast<OpenFlag>(f), n){}
    // ctor with char file name
    inline CUIFile::CUIFile(const char* n, OpenFlag f) noexcept :
        CUIFile(static_cast<OpenFlag>(f | Flag_UTF8FileName), reinterpret_cast<const char16_t*>(n)) {}
}
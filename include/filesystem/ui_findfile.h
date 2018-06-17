#pragma once
#include <cstdint>
#include <type_traits>
#include "../util/ui_unimacro.h"


namespace LongUI {
    // file finding
    class CUIFindFile {
        // error handle
        enum : intptr_t { ERROR_HANDLE = -1 };
        // buffer size
        enum { buf_size = 592 };
        // buffer align
        enum { buf_align = 4 };
        // string type
        using string_t = const wchar_t*;
    public:
        // ctor: first file
        CUIFindFile(string_t first) noexcept;
        // dtor: first file
        ~CUIFindFile() noexcept;
        // ok? first file found?
        bool IsOk() const noexcept { return m_file != ERROR_HANDLE; }
        // find next
        bool FindNext() noexcept;
    public:
        // is directory?
        bool IsDirectory() const noexcept;
        // get file size, 32bit
        auto GetFileSize() const noexcept ->uint32_t;
        // get file name
        auto GetFileName() const noexcept ->string_t;
        // get file size, 64bit
        auto GetFileSize64() const noexcept ->uint64_t;
    protected:
        // file handle
        intptr_t                                        m_file;
        // buffer storage
        std::aligned_storage<buf_size, buf_align>::type m_impl;
    };
}
#pragma once
#include <cstdint>
#include "../util/ui_unimacro.h"
#include "../core/ui_core_type.h"

// ui namespace
namespace LongUI {
    // path op namespace
    namespace PathOP {
        // canonical the path
        auto Canonical(char* buf) noexcept ->uint32_t;
        // base path length
        enum { FILEOP_MAX_PATH = 512 };
        // uri path
        struct UriPath { char path[FILEOP_MAX_PATH]; };
        // make uri path
        auto MakeUriPath(UriPath& buf, U8View prefix, U8View path) noexcept ->U8View;
        // base path
        struct BasePath { wchar_t path[FILEOP_MAX_PATH]; };
        // get temp path, return string length
        auto TempDirectoryPath(BasePath&) noexcept -> uint32_t;
        // get temp path, return string length
        auto TempDirectoryPath(CUIString&) noexcept->uint32_t;
        // get temp path, return tmp-id, <path>\<pre><uuuu>.tmp
        auto TempFileName(
            const wchar_t* path, 
            const wchar_t* prefix, 
            BasePath& filename
        ) noexcept->uint32_t;
    }
}
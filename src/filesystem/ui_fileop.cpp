#include <Windows.h>
#include <filesystem/ui_pathop.h>
#include <core/ui_string.h>

/// <summary>
/// Gets the temporary path.
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
auto LongUI::PathOP::TempDirectoryPath(BasePath& bp) noexcept->uint32_t {
    return ::GetTempPathW(FILEOP_MAX_PATH, bp.path);
}


/// <summary>
/// Temporaries the directory path.
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
auto LongUI::PathOP::TempDirectoryPath(CUIString& str) noexcept -> uint32_t {
    const auto buflen = ::GetTempPathW(0, nullptr);
    str.as_buffer_nul(buflen -1, [buflen](wchar_t* buf) noexcept {
        ::GetTempPathW(buflen, buf);
    });
    return static_cast<uint32_t>(str.length());
}

/// <summary>
/// Temporaries the name of the file.
/// </summary>
/// <param name="path">The path.</param>
/// <param name="prefix">The prefix.</param>
/// <param name="filename">The filename.</param>
/// <returns></returns>
auto LongUI::PathOP::TempFileName(
    const wchar_t* path, 
    const wchar_t* prefix, 
    BasePath & filename) noexcept -> uint32_t {
    return ::GetTempFileNameW(path, prefix, 0, filename.path);
}

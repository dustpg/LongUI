#include <Windows.h>
#include <cassert>
#include <filesystem/ui_findfile.h>

// impl for findfile
namespace LongUI { struct findfile_impl_t {
    // file handle
    WIN32_FIND_DATAW     data;
};}


/// <summary>
/// Initializes a new instance of the <see cref="CUIFindFile" /> class.
/// </summary>
/// <param name="path">The path.</param>
LongUI::CUIFindFile::CUIFindFile(string_t path) noexcept {
    static_assert(buf_size == sizeof(findfile_impl_t), "must be same");
    static_assert(buf_align == alignof(findfile_impl_t), "must be same");
    auto& ff = reinterpret_cast<findfile_impl_t&>(m_impl);
    const auto handle = ::FindFirstFileW(path, &ff.data);
    m_file = reinterpret_cast<intptr_t>(handle);
}

/// <summary>
/// Finalizes an instance of the <see cref="CUIFindFile"/> class.
/// </summary>
/// <returns></returns>
LongUI::CUIFindFile::~CUIFindFile() noexcept {
    auto& ff = reinterpret_cast<findfile_impl_t&>(m_impl);
    if (this->IsOk()) ::FindClose(reinterpret_cast<HANDLE>(m_file));
}


/// <summary>
/// Determines whether this instance is directory.
/// </summary>
/// <returns></returns>
bool LongUI::CUIFindFile::IsDirectory() const noexcept {
    auto& ff = reinterpret_cast<const findfile_impl_t&>(m_impl);
    return !!(ff.data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
}

/// <summary>
/// Gets the name of the file.
/// </summary>
/// <returns></returns>
auto LongUI::CUIFindFile::GetFileName() const noexcept -> string_t {
    auto& ff = reinterpret_cast<const findfile_impl_t&>(m_impl);
    return ff.data.cFileName;
}

/// <summary>
/// Gets the size of the file.
/// </summary>
/// <returns></returns>
auto LongUI::CUIFindFile::GetFileSize() const noexcept -> uint32_t {
    auto& ff = reinterpret_cast<const findfile_impl_t&>(m_impl);
    return ff.data.nFileSizeLow;
}

/// <summary>
/// Gets the file size64.
/// </summary>
/// <returns></returns>
auto LongUI::CUIFindFile::GetFileSize64() const noexcept -> uint64_t {
    auto& ff = reinterpret_cast<const findfile_impl_t&>(m_impl);
    const uint64_t size = static_cast<uint64_t>(ff.data.nFileSizeHigh) << 32;
    return size | static_cast<uint64_t>(ff.data.nFileSizeLow);
}

/// <summary>
/// Finds the next.
/// </summary>
/// <returns></returns>
bool LongUI::CUIFindFile::FindNext() noexcept {
    auto& ff = reinterpret_cast<findfile_impl_t&>(m_impl);
    return !!::FindNextFileW(reinterpret_cast<HANDLE>(m_file), &ff.data);
}

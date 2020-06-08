// windows
#include <Windows.h>

// c++
#include <cassert>

// ui
#include <core/ui_string.h>
#include <util/ui_unicode.h>
#include <filesystem/ui_file.h>

// TODO: 异步IO

/// <summary>
/// Initializes a new instance of the <see cref="CUIFile"/> class.
/// </summary>
/// <param name="flag">The flag.</param>
/// <param name="filename">The file name.</param>
LongUI::CUIFile::CUIFile(OpenFlag flag, const char16_t* filename) noexcept {
    assert(filename && "bad filename");
    // FAILED TO GCC
    //static_assert(ERROR_HANDLE == intptr_t(INVALID_HANDLE_VALUE), "must be same");
    DWORD access = 0;
    // 读
    if (flag & Flag_Read) access |= GENERIC_READ;
    // 写
    if (flag & Flag_Write) access |= GENERIC_WRITE;
    // 打开已存
    DWORD creation = 0;
    // 只读
    if ((flag & (Flag_Read | Flag_Write)) == Flag_Read)
        creation |= OPEN_EXISTING;
    // 总是创建
    if (flag & Flag_CreateAlways) creation |= CREATE_ALWAYS;
    // 总是打开, 没有就创建
    if (flag & Flag_OpenAlways) creation |= OPEN_ALWAYS;
    // UTF-8转换
    CUIString path;
    if (flag & Flag_UTF8FileName) {
        path = CUIString::FromUtf8(reinterpret_cast<const char*>(filename));
        filename = path.c_str();
    }
    // 创建文件句柄
    const auto file = ::CreateFileW(
        detail::sys(filename),
        access,
        FILE_SHARE_READ,
        nullptr,
        creation,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    // 写入
    m_hFile = reinterpret_cast<intptr_t>(file);
}


/// <summary>
/// Reads the specified buffer.
/// </summary>
/// <param name="buffer">The buffer.</param>
/// <param name="bufread">The bufread.</param>
/// <returns></returns>
auto LongUI::CUIFile::Read(void * buffer, uint32_t bufread) const noexcept -> uint32_t {
    assert(this->IsOk() && "bad file");
    DWORD read = 0;
    ::ReadFile(reinterpret_cast<HANDLE>(m_hFile), buffer, bufread, &read, nullptr);
    return read;
}

/// <summary>
/// Writes the specified buffer.
/// </summary>
/// <param name="buffer">The buffer.</param>
/// <param name="bufwrite">The bufwrite.</param>
/// <returns></returns>
auto LongUI::CUIFile::Write(const void * buffer, uint32_t bufwrite) noexcept -> uint32_t {
    assert(this->IsOk() && "bad file");
    DWORD writed = 0;
    ::WriteFile(reinterpret_cast<HANDLE>(m_hFile), buffer, bufwrite, &writed, nullptr);
    return writed;
}

/// <summary>
/// Tells this instance.
/// </summary>
/// <returns></returns>
auto LongUI::CUIFile::Tell() const noexcept -> uint32_t {
    assert(this->IsOk());
    return ::SetFilePointer(reinterpret_cast<HANDLE>(m_hFile), 0, nullptr, FILE_CURRENT);
}

/// <summary>
/// Seeks the specified offset.
/// </summary>
/// <param name="offset">The offset.</param>
/// <param name="seek">The seek.</param>
/// <returns></returns>
auto LongUI::CUIFile::Seek(int32_t offset, TypeOfSeek seek) const noexcept -> uint32_t {
    assert(this->IsOk() && "bad file");
    static_assert(Seek_End == FILE_END, "must be same");
    static_assert(Seek_Begin == FILE_BEGIN, "must be same");
    static_assert(Seek_Current == FILE_CURRENT, "must be same");
    const auto handle = reinterpret_cast<HANDLE>(m_hFile);
    const auto method = static_cast<DWORD>(seek);
    return ::SetFilePointer(handle, offset, nullptr, method);
}

/// <summary>
/// Gets the filezize.
/// </summary>
/// <returns></returns>
auto LongUI::CUIFile::GetFilezize() const noexcept -> uint32_t {
    assert(this->IsOk() && "bad file");
    return ::GetFileSize(reinterpret_cast<HANDLE>(m_hFile), nullptr);
}

/// <summary>
/// Finalizes an instance of the <see cref="CUIFile"/> class.
/// </summary>
/// <returns></returns>
LongUI::CUIFile::~CUIFile() noexcept {
    if (this->IsOk()) {
        ::CloseHandle(reinterpret_cast<HANDLE>(m_hFile));
        m_hFile = ERROR_HANDLE;
    }
}
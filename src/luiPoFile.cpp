#include <Platonly/luiPoFile.h>
#include <Windows.h>

// longui 命名空间
namespace LongUI {
    // windows helper
    inline auto windows(CUIFile::FileHandle menu) { return reinterpret_cast<HANDLE>(menu); }
    // longui helper
    inline auto longui(HANDLE menu) { return reinterpret_cast<CUIFile::FileHandle>(menu); }
}


// CUIFile 构造函数
LongUI::CUIFile::CUIFile(const wchar_t* namefile, OpenFlag flag) noexcept {
    assert(namefile);
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
    // 只是创建新的
    if (flag & Flag_CreateNew) creation |= CREATE_NEW;
    // 创建文件句柄
    auto file = ::CreateFileW(
        namefile,
        access,
        FILE_SHARE_READ,
        nullptr,
        creation,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    // 写入
    m_hFile = longui(file);
}

// 打开成功
bool LongUI::CUIFile::IsOk() const noexcept {
    return windows(m_hFile) != INVALID_HANDLE_VALUE;
}


// 读取文件
auto LongUI::CUIFile::Read(void * buffer, uint32_t bufread) const noexcept -> uint32_t {
    assert(this->IsOk());
    DWORD read = 0;
    ::ReadFile(windows(m_hFile), buffer, bufread, &read, nullptr);
    return read;
}

// 写入文件
auto LongUI::CUIFile::Write(void * buffer, uint32_t bufwrite) noexcept -> uint32_t {
    assert(this->IsOk());
    DWORD writed = 0;
    ::WriteFile(windows(m_hFile), buffer, bufwrite, &writed, nullptr);
    return writed;
}

// 获取文件指针位置
auto LongUI::CUIFile::Tell() const noexcept -> uint32_t {
    assert(this->IsOk());
    return ::SetFilePointer(windows(m_hFile), 0, nullptr, FILE_CURRENT);
}

// 设置文件指针位置
auto LongUI::CUIFile::Seek(int32_t offset, TypeOfSeek seek) const noexcept -> uint32_t {
    assert(this->IsOk());
    return ::SetFilePointer(windows(m_hFile), offset, nullptr, DWORD(seek));
}

// 获取文件大小
auto LongUI::CUIFile::GetFilezize() const noexcept -> uint32_t {
    assert(this->IsOk());
    return ::GetFileSize(windows(m_hFile), nullptr);
}

// CUIFile 析构函数
LongUI::CUIFile::~CUIFile() noexcept {
    if (this->IsOk()) {
        ::CloseHandle(windows(m_hFile));
        m_hFile = longui(INVALID_HANDLE_VALUE);
    }
}
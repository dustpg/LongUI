#include <Windows.h>
#include <Mmsystem.h>
#include <util/ui_time.h>
#include <core/ui_string.h>
#include <util/ui_clipboard.h>
#include <util/ui_time_meter.h>
#pragma comment(lib, "Winmm")

/// <summary>
/// To the system time.
/// </summary>
/// <param name="st">The system time.</param>
/// <returns></returns>
bool LongUI::FileTime::ToSystemTime(SystemTime& st) const noexcept {
    static_assert(sizeof(SystemTime) == sizeof(SYSTEMTIME), "must be same");
    FILETIME ff;
    ff.dwLowDateTime = static_cast<uint32_t>(this->time);
    ff.dwHighDateTime = static_cast<uint32_t>(this->time >> 32);
    return !!::FileTimeToSystemTime(&ff, reinterpret_cast<SYSTEMTIME*>(&st));
}

/// <summary>
/// To the file time.
/// </summary>
/// <param name="time">The file time.</param>
/// <returns></returns>
bool LongUI::SystemTime::ToFileTime(FileTime& time) const noexcept {
    const auto st = reinterpret_cast<const SYSTEMTIME*>(this);
    FILETIME ff; const auto code = ::SystemTimeToFileTime(st, &ff);
    time.time = static_cast<uint64_t>(ff.dwLowDateTime) |
        (static_cast<uint64_t>(ff.dwHighDateTime) << 32);
    return !!code;
}

/// <summary>
/// Froms the system time.
/// </summary>
/// <param name="st">The st.</param>
/// <returns></returns>
void LongUI::SystemTime::FromSystemTime(SystemTime& st) noexcept {
    ::GetSystemTime(reinterpret_cast<SYSTEMTIME*>(&st));
}



/// <summary>
/// Queries the performance frequency.
/// </summary>
/// <param name="ll">The ll.</param>
/// <returns></returns>
void LongUI::CUITimeMeterH::QueryPerformanceFrequency(uint64_t& ll) noexcept {
    ::QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&ll));
}

/// <summary>
/// Queries the performance counter.
/// </summary>
/// <param name="ll">The ll.</param>
/// <returns></returns>
void LongUI::CUITimeMeterH::QueryPerformanceCounter(uint64_t& ll) noexcept {
    /*auto old = ::SetThreadAffinityMask(::GetCurrentThread(), 0);
    auto r = ::QueryPerformanceCounter(ll);
    ::SetThreadAffinityMask(::GetCurrentThread(), old);
    return r;*/
    ::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&ll));
}


/// <summary>
/// Gets the time tick.
/// </summary>
/// <returns></returns>
auto LongUI::GetTimeTick() noexcept -> uint32_t {
    return ::timeGetTime();
}

/// <summary>
/// Gets the double click time.
/// </summary>
/// <returns></returns>
auto LongUI::GetDoubleClickTime() noexcept -> uint32_t {
    return ::GetDoubleClickTime();
}


/// <summary>
/// Copies the text to clipboard.
/// </summary>
/// <param name="view">The view.</param>
/// <returns></returns>
bool LongUI::CopyTextToClipboard(U16View view) noexcept {
    static_assert(sizeof(char16_t) == sizeof(wchar_t), "WINDOWS");
    // 申请全局内存
    constexpr size_t sz = sizeof(char16_t);
    const size_t len = sz * ((view.end() - view.begin()) + 1);
    const auto handle = ::GlobalAlloc(GMEM_MOVEABLE, len);
    if (handle) {
        // 准备解锁写入
        if (const auto str = reinterpret_cast<char16_t*>(::GlobalLock(handle))) {
            std::memcpy(str, view.begin(), len - sz);
            str[view.end() - view.begin()] = 0;
            ::GlobalUnlock(handle);
        }
    }
    else return false;
    // 待返回值
    bool rv = false;
    // 打开任务剪切板
    if (::OpenClipboard(nullptr)) {
        ::EmptyClipboard();
        if (::SetClipboardData(CF_UNICODETEXT, handle)) rv = true;
        else ::GlobalFree(handle);
        ::CloseClipboard();
    }
    return rv;
}


/// <summary>
/// Pastes the text to clipboard.
/// </summary>
/// <param name="str">The string.</param>
/// <returns></returns>
bool LongUI::PasteTextToClipboard(CUIString& str) noexcept {
    str.clear();
    bool rv = false;
    // 打开任务剪切板
    if (::OpenClipboard(nullptr)) {
        const auto handle = ::GetClipboardData(CF_UNICODETEXT);
        // 解锁写入
        if (const auto ptr = ::GlobalLock(handle)) {
            const auto len = ::GlobalSize(handle) / sizeof(char16_t) - 1;
            const auto text = reinterpret_cast<const char16_t*>(ptr);
            const auto real = len;
            str.assign(text, text + real);
            rv = true;
            ::GlobalUnlock(handle);
        }
        ::CloseClipboard();
    }
    return rv;
}
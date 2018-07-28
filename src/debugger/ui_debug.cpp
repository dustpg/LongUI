#define NOMINMAX
#include <Windows.h>
#pragma warning(push)
#pragma warning(disable: 4091)
#include <Dbghelp.h>
#pragma warning(pop)

#include <cstring>

#include <core/ui_color.h>
#include <debugger/ui_debug.h>
#include <filesystem/ui_file.h>
#include <style/ui_style_state.h>

#pragma comment(lib, "dbghelp")

/// <summary>
/// LongUI::endl instance
/// </summary>
namespace LongUI { const EndL endl; }

// longui::detail namespace
namespace LongUI { namespace detail {
    // utf16 to system char type
    static inline auto sys(const char16_t* str) noexcept {
        using target_t = wchar_t;
        static_assert(sizeof(target_t) == sizeof(char16_t), "WINDOWS!");
        return reinterpret_cast<const wchar_t*>(str);
    }
}}

/// <summary>
/// UIs the unexp filter.
/// </summary>
/// <param name="p">The p.</param>
/// <returns></returns>
extern "C" LONG WINAPI ui_unexp_filter(EXCEPTION_POINTERS* p) noexcept {
    //::MessageBoxW(0, 0, 0, 0);
    using namespace LongUI;
    // 写入bug报告
    const auto flag = CUIFile::Flag_CreateAlways | CUIFile::Flag_Write;
#if 0
    constexpr uint32_t CAP_SIZE = 256;
    constexpr uint32_t BUF_SIZE = 512;
    void* backtrace[CAP_SIZE];
    char buffer[BUF_SIZE];
    union { SYMBOL_INFO syminfo; char syminfobuf[BUF_SIZE]; };
    std::memset(syminfobuf, 0, sizeof(syminfobuf));
    syminfo.MaxNameLen = BUF_SIZE - sizeof(SYMBOL_INFO);
    syminfo.SizeOfStruct = sizeof(SYMBOL_INFO);
    // 获取进程句柄
    const auto process = ::GetCurrentProcess();
    // 初始化符号表
    ::SymSetOptions(SYMOPT_LOAD_LINES);
    ::SymInitialize(process, nullptr, true);
    // 获取栈回溯信息
    const auto len = CUIDebug::Backtrace(backtrace, CAP_SIZE);
    // 二进制版
    /*if (CUIFile bug_bin{ "bug.bin", flag }) {
        struct header { uint32_t endianness, pointer; } h;
        h.endianness = 0x0a0b0c0d;
        h.pointer = sizeof(void*);
        bug_bin.Write(&h, sizeof(h));
        bug_bin.Write(p->ExceptionRecord, sizeof *p->ExceptionRecord);
        bug_bin.Write(p->ContextRecord, sizeof *p->ContextRecord);
        bug_bin.Write(backtrace, sizeof(void*) * len);
    }*/
    // 文本版
    if (CUIFile bug_log{ "bug.log", flag }) {
        if(p)
        bug_log.Write(
            buffer, sizeof(buffer[0]) * std::snprintf(
                buffer, BUF_SIZE,
                "CODE:\r\n  0x%08X\r\n", 
                p->ExceptionRecord->ExceptionCode
            )
        );
        bug_log.Write(
            buffer, sizeof(buffer[0]) * std::snprintf(
                buffer, BUF_SIZE,
                "GetModuleHandler:\r\n  0x%p\r\n",
                ::GetModuleHandleA(nullptr)
            )
        );
        const auto bt = "BACKTRACE:\r\n";
        bug_log.Write(bt, std::strlen(bt) * sizeof(buffer[0]));
        for (uint32_t i = 0; i != len; ++i) {
            const auto address = reinterpret_cast<uintptr_t>(backtrace[i]);
            auto code = ::SymFromAddr(process, address, 0, &syminfo);
            const auto length = std::snprintf(
                buffer, BUF_SIZE, 
                "  [%03d]0x%p{0x%016llX} %s \r\n", int(i+1), 
                backtrace[i],
                syminfo.Address,
                syminfo.Name
            );
            bug_log.Write(buffer, length * sizeof(buffer[0]));
        }
    }
#else
    // 文件路径
    constexpr size_t FILE_PATH_LENGTH = 1024;
    char16_t file_path[FILE_PATH_LENGTH]; *file_path = 0;
    // 获取DUMP路径
    std::memcpy(file_path, u"bug.dmp", 8 * sizeof(char16_t));
    //::MessageBoxW(nullptr, file_path, file_path, MB_OK | MB_ICONERROR);
    // 写入dump
    if (CUIFile bug_log{ file_path, flag }) {
        // 建立最小转储
        MINIDUMP_EXCEPTION_INFORMATION ExInfo;
        ExInfo.ThreadId = ::GetCurrentThreadId();
        ExInfo.ExceptionPointers = p;
        ExInfo.ClientPointers = false;
        ::MiniDumpWriteDump(
            ::GetCurrentProcess(),
            ::GetCurrentProcessId(),
            static_cast<HANDLE>(bug_log.GetHandle()),
            MiniDumpNormal,
            &ExInfo,
            nullptr,
            nullptr
        );
    }
    // 打开错误
    else {
        // TODO: 错误处理

        return EXCEPTION_EXECUTE_HANDLER;
    }
#endif
    // 通知BUG报告发送
    ::MessageBoxW(
        nullptr, 
        L"ERROR, MINIDUMP FILE DUMPED.", 
        LongUI::detail::sys(file_path),
        MB_ICONERROR
    );
    return EXCEPTION_EXECUTE_HANDLER;
}

/// <summary>
/// Initializes the un exp handler.
/// </summary>
/// <returns></returns>
void LongUI::CUIDebug::InitUnExpHandler() noexcept {
    ::SetUnhandledExceptionFilter(ui_unexp_filter);
}

#ifndef NDEBUG
#include <ctime>
#include <core/ui_manager.h>
#include <core/ui_string.h>
#include "../private/ui_console.h"


/// <summary>
/// Initializes a new instance of the <see cref="CUIDebug"/> class.
/// </summary>
/// <param name="fname">The fname.</param>
LongUI::CUIDebug::CUIDebug(const char16_t* fname) noexcept
    : m_logFile(fname, CUIFile::Flag_Write | CUIFile::Flag_OpenAlways)
{
}


/// <summary>
/// Outputs the debug string w.
/// </summary>
/// <param name="level">The level.</param>
/// <param name="str">The string.</param>
/// <param name="flush">if set to <c>true</c> [flush].</param>
/// <returns></returns>
void LongUI::CUIDebug::OutputString(
    DebugStringLevel level,
    const wchar_t* str,
    bool flush) noexcept {
    if (!(UIManager.flag & IUIConfigure::Flag_OutputDebugString)) return;
    static CUIConsole s_consoles[DebugStringLevel::DLEVEL_SIZE];
    auto& console = s_consoles[level];
    auto create_console = [=]() noexcept {
        CUIConsole::Config config;
        config.i16.x = -5;
        config.i16.y = int16_t(level) * 128;
        switch (level)
        {
        case LongUI::DLevel_None:
            break;
        case LongUI::DLevel_Log:
            break;
        case LongUI::DLevel_Hint:
            break;
        case LongUI::DLevel_Warning:
            config.atribute = FOREGROUND_RED | FOREGROUND_GREEN;
            break;
        case LongUI::DLevel_Error:
        case LongUI::DLevel_Fatal:
            config.atribute = FOREGROUND_RED;
            break;
        }
        assert(level < LongUI::DLEVEL_SIZE && "out of range");
        // 名称
        const wchar_t* strings[LongUI::DLEVEL_SIZE] = {
            L"None      Console",
            L"Log       Console",
            L"Hint      Console",
            L"Warning   Console",
            L"Error     Console",
            L"Fatal     Console"
        };
        s_consoles[level].Create(strings[level], config);
    };
    // 无效就创建
    if (!console) {
        create_console();
    }
    // 有效就输出
    if (console) {
        console.Output(str, flush);
    }
    // 输出到日志?
    if (m_logFile && level == DebugStringLevel::DLevel_Log) {
        // 五秒精度
        constexpr uint32_t UNIT = 5'000;
        auto now = ::GetTickCount();
        if ((now / UNIT) != (static_cast<uint32_t>(m_timeTick) / UNIT)) {
            m_timeTick = static_cast<size_t>(now);
            // 不一样则输出时间
            std::time_t time = std::time(nullptr);
            constexpr size_t bl = 1024;
            wchar_t buffer[1024];
            std::wcsftime(
                buffer, bl,
                L"[%c]\r\n",
                std::localtime(&time)
            );
            m_logFile.Write(buffer, static_cast<uint32_t>(sizeof(wchar_t) * std::wcslen(buffer)));
        }
        m_logFile.Write(str, static_cast<uint32_t>(sizeof(wchar_t) * std::wcslen(str)));
    }
}

/// <summary>
/// Gets the debug instance.
/// 获取调试实例
/// </summary>
/// <returns></returns>
auto LongUI::CUIDebug::GetInstance() noexcept->CUIDebug& {
    return LongUI::CUIManager::GetInstance();
}


/// <summary>
/// Outputs the debug string without flushing.
/// 调试输出(不刷新)
/// </summary>
/// <param name="lv">The debug lv.</param>
/// <param name="str">The debug string.</param>
/// <returns></returns>
void LongUI::CUIDebug::OutputNoFlush(
    DebugStringLevel lv, const char16_t * str) noexcept {
    this->OutputString(lv, detail::sys(str), false);
}

/// <summary>
/// Formateds the specified format.
/// 传递可视化东西
/// </summary>
/// <param name="format">The format.</param>
/// <param name="">The .</param>
/// <returns></returns>
auto LongUI::Formated(const char* format, ...) noexcept -> const char* {
    constexpr size_t buflen = 2048;
    static thread_local char buffer[buflen];
    va_list ap;
    va_start(ap, format);
    std::vsnprintf(buffer, buflen, format, ap);
    va_end(ap);
    return buffer;
}


/// <summary>
/// Interfmts the specified format.
/// </summary>
/// <param name="format">The format.</param>
/// <param name="">The .</param>
/// <returns></returns>
auto LongUI::Interfmt(const char* format, ...) noexcept -> const char* {
    constexpr size_t buflen = 2048;
    static thread_local char buffer[buflen];
    va_list ap;
    va_start(ap, format);
    std::vsnprintf(buffer, buflen, format, ap);
    va_end(ap);
    return buffer;
}

/// <summary>
/// Operators the specified .
/// 换行刷新重载
/// </summary>
/// <param name="">The .</param>
/// <returns></returns>
auto LongUI::CUIDebug::operator<<(const LongUI::EndL&) noexcept ->CUIDebug& {
    char16_t chs[3] = { L'\r',L'\n', 0 };
    this->Output(m_lastLevel, chs);
    return *this;
}

#include <graphics/ui_adapter_desc.h>

/// <summary>
/// Operators the specified desc.
/// </summary>
/// <param name="desc">The desc.</param>
/// <returns></returns>
auto LongUI::CUIDebug::operator<<(const GraphicsAdapterDesc& desc) noexcept ->CUIDebug& {
    CUIString str;
    str.format(
        u"Adapter:   { \r\n\t Friend Name: %ls\r\n"
        u"\t DedicatedVideoMemory: %.3lfMB\r\n"
        u"\t DedicatedSystemMemory: %.3lfMB\r\n"
        u"\t SharedSystemMemory: %.3lfMB\r\n"
        u"}",
        desc.friend_name,
        static_cast<double>(desc.dedicated_video) / (1024.*1024.),
        static_cast<double>(desc.dedicated_system) / (1024.*1024.),
        static_cast<double>(desc.shared_system) / (1024.*1024.)
    );
    this->OutputNoFlush(m_lastLevel, str.c_str());
    return *this;
}

/// <summary>
/// Operators the specified matrix.
/// </summary>
/// <param name="matrix">The matrix.</param>
/// <returns></returns>
auto LongUI::CUIDebug::operator<<(const LongUI::Matrix3X2F& matrix) noexcept ->CUIDebug& {
    CUIString str;
    str.format(
        u"Matrix3x2 (%7.2f, %7.2f, %7.2f, %7.2f, %7.2f, %7.2f)",
        matrix._11, matrix._12,
        matrix._21, matrix._22,
        matrix._31, matrix._32
    );
    this->OutputNoFlush(m_lastLevel, str.c_str());
    return *this;
}

/// <summary>
/// Operators the specified rect.
/// </summary>
/// <param name="rect">The rect.</param>
/// <returns></returns>
auto LongUI::CUIDebug::operator<<(const LongUI::RectF& rect) noexcept ->CUIDebug& {
    CUIString str;
    str.format(
        u"Rect(%7.2f, %7.2f, %7.2f, %7.2f)",
        rect.left, rect.top, rect.right, rect.bottom
    );
    this->OutputNoFlush(m_lastLevel, str.c_str());
    return *this;
}

/// <summary>
/// Operators the specified rect.
/// </summary>
/// <param name="rect">The rect.</param>
/// <returns></returns>
auto LongUI::CUIDebug::operator<<(const LongUI::ColorF& color) noexcept ->CUIDebug& {
    CUIString str;
    str.format(
        u"RGBA(%0.2f, %0.2f, %0.2f, %0.2f)",
        color.r, color.g, color.b, color.a
    );
    this->OutputNoFlush(m_lastLevel, str.c_str());
    return *this;
}

/// <summary>
/// Operators the specified pt.
/// </summary>
/// <param name="pt">The pt.</param>
/// <returns></returns>
auto LongUI::CUIDebug::operator<<(const LongUI::Point2F& pt) noexcept ->CUIDebug& {
    CUIString str;
    str.format(u"Point(%7.2f, %7.2f)", pt.x, pt.y);
    this->OutputNoFlush(m_lastLevel, str.c_str());
    return *this;
}

/// <summary>
/// Operators the specified pt.
/// </summary>
/// <param name="pt">The pt.</param>
/// <returns></returns>
auto LongUI::CUIDebug::operator<<(const LongUI::Size2F& sz) noexcept ->CUIDebug& {
    CUIString str;
    str.format(u"Size(%7.2f, %7.2f)", sz.width, sz.height);
    this->OutputNoFlush(m_lastLevel, str.c_str());
    return *this;
}

/// <summary>
/// Outputs the specified l.
/// 输出UTF-8字符串 并刷新
/// </summary>
/// <param name="l">The l.</param>
/// <param name="s">The s.</param>
/// <returns></returns>
void LongUI::CUIDebug::Output(DebugStringLevel l, const char* s) noexcept {
    const auto c = LongUI::CUIStringEx::FromUtf8(s);
    this->Output(l, c.c_str());
}

/// <summary>
/// Outputs the debug string with flushing.
/// </summary>
/// <param name="l">The l.</param>
/// <param name="s">The s.</param>
/// <returns></returns>
void LongUI::CUIDebug::Output(DebugStringLevel l, const char16_t* s) noexcept {
    this->OutputString(l, detail::sys(s), true);
}

/// <summary>
/// Outputs the no flush.
/// 输出UTF-8字符串
/// </summary>
/// <param name="l">The l.</param>
/// <param name="s">The s.</param>
/// <returns></returns>
void LongUI::CUIDebug::OutputNoFlush(DebugStringLevel l, const char* s) noexcept {
    const auto c = LongUI::CUIStringEx::FromUtf8(s);
    this->OutputNoFlush(l, c.c_str());
}

/// <summary>
/// Operators the specified f.
/// 浮点重载
/// </summary>
/// <param name="f">The f.</param>
/// <returns></returns>
auto LongUI::CUIDebug::operator<<(const float f) noexcept ->CUIDebug& {
    CUIString str;
    str.format(u"%f", f);
    this->OutputNoFlush(m_lastLevel, str.c_str());
    return *this;
}

/// <summary>
/// Operators the specified f.
/// 浮点重载
/// </summary>
/// <param name="f">The f.</param>
/// <returns></returns>
auto LongUI::CUIDebug::operator<<(const DDFFloat2 f) noexcept ->CUIDebug& {
    CUIString str; const float value = f.f;
    str.format(u"%.2f", value);
    this->OutputNoFlush(m_lastLevel, str.c_str());
    return *this;
}

/// <summary>
/// Operators the specified f.
/// 浮点重载
/// </summary>
/// <param name="f">The f.</param>
/// <returns></returns>
auto LongUI::CUIDebug::operator<<(const DDFFloat3 f) noexcept ->CUIDebug& {
    CUIString str;
    str.format(u"%.3f", f.f);
    this->OutputNoFlush(m_lastLevel, str.c_str());
    return *this;
}

/// <summary>
/// Operators the specified f.
/// 浮点重载
/// </summary>
/// <param name="f">The f.</param>
/// <returns></returns>
auto LongUI::CUIDebug::operator<<(const DDFFloat4 f) noexcept ->CUIDebug& {
    CUIString str;
    str.format(u"%.4f", f.f);
    this->OutputNoFlush(m_lastLevel, str.c_str());
    return *this;
}

/// <summary>
/// Operators the specified control.
/// 指针
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
auto LongUI::CUIDebug::operator<<(const void* ctrl) noexcept ->CUIDebug& {
    CUIString str;
    str.format(u"[0x%p] ", ctrl);
    this->OutputNoFlush(m_lastLevel, str.c_str());
    return *this;
}

#include <control/ui_control.h>
#include <core/ui_ctrlmeta.h>
#include <wchar.h>

/// <summary>
/// Operators the specified control.
/// 控件
/// </summary>
/// <param name="ctrl">The control.</param>
/// <returns></returns>
auto LongUI::CUIDebug::operator<<(const UIControl* ctrl) noexcept ->CUIDebug& {
    CUIStringU8 str{ "[null]" };
    if (false) {
        if (ctrl) str.format(
            "[%p]<%s>{%s}",
            ctrl,
            ctrl->GetID(),
            ctrl->name_dbg
        );
    }
    if (ctrl) str.format(
        "<%s~%s>", 
        ctrl->GetMetaInfo().element_name, 
        ctrl->name_dbg
    );
    this->OutputNoFlush(m_lastLevel, str.c_str());
    return *this;
}


/// <summary>
/// Operators the specified l.
/// 整型输出重载
/// </summary>
/// <param name="l">The l.</param>
/// <returns></returns>

#include <cinttypes>

/// <summary>
/// Operators the specified b.
/// </summary>
/// <param name="b">The b.</param>
/// <returns></returns>
auto LongUI::CUIDebug::operator<<(const uint32_t o) noexcept ->CUIDebug& {
    CUIString str;
    str.format(u"%" PRIu32, o);
    this->OutputNoFlush(m_lastLevel, str.c_str());
    return *this;
}

/// <summary>
/// Operators the specified b.
/// </summary>
/// <param name="b">The b.</param>
/// <returns></returns>
auto LongUI::CUIDebug::operator<<(const int32_t o) noexcept ->CUIDebug& {
    CUIString str;
    str.format(u"%" PRIi32, o);
    this->OutputNoFlush(m_lastLevel, str.c_str());
    return *this;
}

/// <summary>
/// Operators the specified b.
/// 布尔输出重载
/// </summary>
/// <param name="b">The b.</param>
/// <returns></returns>
auto LongUI::CUIDebug::operator<<(const bool b) noexcept ->CUIDebug& {
    this->OutputNoFlush(m_lastLevel, b ? "true" : "false");
    return *this;
}


/// <summary>
/// Operators the specified string.
/// </summary>
/// <param name="str">The string.</param>
/// <returns></returns>
auto LongUI::CUIDebug::operator<<(const CUIString& str) noexcept ->CUIDebug& {
    this->OutputNoFlush(m_lastLevel, str.c_str());
    return *this;
}



/// <summary>
/// Operators the specified s.
/// </summary>
/// <param name="view">The view.</param>
/// <returns></returns>
auto LongUI::CUIDebug::operator<<(const U8View view) noexcept ->CUIDebug& {
    this->OutputNoFlush(m_lastLevel, CUIString::FromUtf8(view).c_str());
    return *this;
}


#include <event/ui_mouse_event.h>
namespace {
    // mouse event string map
    const char* const MESTRMAP[] = {
        "MouseWheelV",
        "MouseWheelH",
        "MouseEnter",
        "MouseLeave",
        "MouseMove",
        "LButtonDown",
        "LButtonUp",
        "RButtonDown",
        "RButtonUp",
        "MButtonDown",
        "MButtonUp",
        "Unknown",
    };
}

/// <summary>
/// Operators the specified string.
/// </summary>
/// <param name="str">The string.</param>
/// <returns></returns>
auto LongUI::CUIDebug::operator<<(MouseEvent e) noexcept ->CUIDebug& {
    this->OutputNoFlush(m_lastLevel, MESTRMAP[int(e)]);
    return *this;
}

/// <summary>
/// Operators the specified e.
/// </summary>
/// <param name="e">The e.</param>
/// <returns></returns>
auto LongUI::CUIDebug::operator<<(StyleStateTypeChange e) noexcept->CUIDebug& {
    const char* type_name ;
    switch (e.type)
    {
    default: type_name = "[unkown]"; break;
    case LongUI::StyleStateType::Type_None:     type_name = "[none]"; break;
    //case LongUI::StyleStateType::Type_OddIndex: type_name = "[oddindex]"; break;
    case LongUI::StyleStateType::Type_Selected: type_name = "[selected]"; break;
    case LongUI::StyleStateType::Type_Default:  type_name = "[default]"; break;
    case LongUI::StyleStateType::Type_Disabled: type_name = "[disabled]"; break;
    case LongUI::StyleStateType::Type_Hover:    type_name = "[hover]"; break;
    case LongUI::StyleStateType::Type_Active:   type_name = "[active]"; break;
    case LongUI::StyleStateType::Type_Focus:    type_name = "[focus]"; break;
    case LongUI::StyleStateType::Type_Checked:  type_name = "[checked]"; break;
    case LongUI::StyleStateType::Type_Indeterminate:type_name = "[indeterminate]"; break;
    case LongUI::StyleStateType::Type_Closed    :type_name = "[closed]"; break;
    case LongUI::StyleStateType::Type_NA_TabAfterSelectedTab:type_name = "[tab after seltab]"; break;
    }
    return *this << type_name << " -> " << e.change;
}

/// <summary>
/// Operators the specified ch.
/// </summary>
/// <param name="ch">The ch.</param>
/// <returns></returns>
auto LongUI::CUIDebug::operator<<(const char32_t ch) noexcept  ->CUIDebug& {
    char16_t buffer[4] = { 0,0,0,0 };
    // TODO: utf32 to utf16
    buffer[0] = char16_t(ch);
    this->OutputNoFlush(m_lastLevel, buffer);
    return *this;
}

#endif
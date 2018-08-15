#pragma once
#ifndef NDEBUG
// c++
#include <cstdint>

// windows
#include <windows.h>

// ui namespace
namespace LongUI {
    // XXX: Console for App, 
    class CUIConsole {
    public:
        // config
        struct Config {
            // ctor
            Config() noexcept { position_xy = uint32_t(-1); };
            // buffer X
            int32_t         buffer_size_x = -1;
            // buffer Y
            int32_t         buffer_size_y = -1;
            // position XY
            union {
                // U32
                uint32_t    position_xy;
                // I16 * 2
                struct  {
                    // X pos
                    int16_t x;
                    // Y pos
                    int16_t y;
                } i16;
            };
            // atribute
            uint32_t        atribute = 0;
            // logger name
            const wchar_t*  logger_name = nullptr;
            // logger name
            const wchar_t*  helper_executable = nullptr;
        };
        // ctor
        CUIConsole() noexcept;
        // ctor
        ~CUIConsole() noexcept;
    public:
        // bool
        operator bool() const noexcept { return m_hConsole != INVALID_HANDLE_VALUE || !m_hConsole; }
        // create a new console foe this app
        long Create(const wchar_t*, Config& config) noexcept;
        // close this
        long Close() noexcept;
        // output the string
        long Output(const wchar_t* str, bool flush, size_t len) noexcept;
        // output the string
        long Output(const wchar_t* str, bool flush) noexcept;// { return this->Output(str, flush, std::wcslen(str)); }
    protected:
        // handle for console
        HANDLE              m_hConsole = INVALID_HANDLE_VALUE;
        // current length
        size_t              m_length = 0;
        // lock
        //CRITICAL_SECTION    m_cs;
        // name for this
        wchar_t             m_name[64];
        // buffer
        wchar_t             m_buffer[1024];
    };

    // CUIConsole 构造函数
    CUIConsole::CUIConsole() noexcept {
        //::InitializeCriticalSection(&m_cs);  
        m_name[0] = L'\0';
        { if (m_hConsole != INVALID_HANDLE_VALUE) this->Close(); }
    }


    // output the string
    long CUIConsole::Output(const wchar_t* str, bool flush) noexcept { 
        return this->Output(str, flush, std::wcslen(str)); 
    }

    // CUIConsole 析构函数
    CUIConsole::~CUIConsole() noexcept {
        this->Close();
        // 关闭
        if (m_hConsole != INVALID_HANDLE_VALUE) {
            ::CloseHandle(m_hConsole);
            m_hConsole = INVALID_HANDLE_VALUE;
        }
        //::DeleteCriticalSection(&m_cs);
    }

    // CUIConsole 关闭
    long CUIConsole::Close() noexcept {
        if (!(*this))
            return -1;
        else
            return ::DisconnectNamedPipe(m_hConsole);
    }

    // CUIConsole 输出
    long CUIConsole::Output(const wchar_t * str, bool flush, size_t len) noexcept {
        constexpr size_t BUFLEN = sizeof(m_buffer) / sizeof(m_buffer[0]);
        // 过长则分批
        if (len > BUFLEN) {
            // 直接递归
            while (len) {
                auto len_in = len > BUFLEN ? BUFLEN : len;
                this->Output(str, true, len_in);
                len -= len_in;
                str += len_in;
            }
            return 0;
        }
        // 计算目标
        if (m_length + len > BUFLEN) {
            flush = true;
        }
        // 写入
        if (m_length + len < BUFLEN) {
            std::memcpy(m_buffer + m_length, str, len * sizeof(wchar_t));
            m_length += len;
            str = nullptr;
            // 不用flush
            if (!flush) return 0;
        }
        DWORD dwWritten = DWORD(-1);
        // 写入
        auto safe_write_file = [this, &dwWritten]() {
            return ::WriteFile(
                m_hConsole, m_buffer,
                static_cast<uint32_t>(m_length * sizeof(wchar_t)),
                &dwWritten, nullptr
            );
        };
        // 先写入缓冲区
        if (m_length) {
            safe_write_file();
            m_length = 0;
        }
        // 再写入目标
        if (str) {
            len *= sizeof(wchar_t);
            return (!safe_write_file() || (int)dwWritten != len) ? -1 : (int)dwWritten;
        }
        return 0;
    }

    // CUIConsole 创建
    long CUIConsole::Create(const wchar_t* lpszWindowTitle, Config& config) noexcept {
        // 二次创建?
        if (m_hConsole != INVALID_HANDLE_VALUE) {
            ::DisconnectNamedPipe(m_hConsole);
            ::CloseHandle(m_hConsole);
            m_hConsole = INVALID_HANDLE_VALUE;
        }
        // 先复制
        std::wcscpy(m_name, LR"(\\.\pipe\)");
        constexpr int bl = 128;
        wchar_t logger_name_buffer[bl];
        // 未给logger?
        if (!config.logger_name) {
            static float s_times = 1.f;
            std::swprintf(
                logger_name_buffer, bl,
                L"logger_%7.5f",
                float(::GetTickCount()) / float(1000 * 60 * 60) *
                (float(std::rand()) / float(RAND_MAX)) * s_times
            );
            config.logger_name = logger_name_buffer;
            ++s_times;
        }
        std::wcscat(m_name, config.logger_name);
        // 创建管道
        m_hConsole = ::CreateNamedPipeW(
            m_name,
            PIPE_ACCESS_OUTBOUND,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_BYTE | PIPE_WAIT,
            1,
            4096,   // 输出缓存
            0,      // 输入缓存
            1,
            nullptr
        );
        // 无效
        if (m_hConsole == INVALID_HANDLE_VALUE) {
            ::MessageBoxW(nullptr, L"CreateNamedPipe failed", L"CUIConsole::Create failed", MB_ICONERROR);
            return -1;
        }
        // 创建控制台
        PROCESS_INFORMATION pi;
        STARTUPINFOW si; ::GetStartupInfoW(&si);

        const wchar_t* DEFAULT_HELPER_EXEsw[]{
            L"console.exe",
            L"../helper/console.exe",
            L"../../helper/console.exe",
        };

        const char* DEFAULT_HELPER_EXEs[]{
            "console.exe",
            "../helper/console.exe",
            "../../helper/console.exe",
        };
        auto exe_path = DEFAULT_HELPER_EXEsw[0];
        {
            int i = 0;
            for (auto exe : DEFAULT_HELPER_EXEs) {
                if (auto file = std::fopen(exe, "rb")) {
                    exe_path = DEFAULT_HELPER_EXEsw[i];
                    std::fclose(file);
                    break;
                }
                ++i;
            }
        }
        wchar_t cmdline[MAX_PATH];;
        if (!config.helper_executable)
            config.helper_executable = exe_path;

        std::swprintf(cmdline, MAX_PATH, L"%ls %ls", config.helper_executable, config.logger_name);
        BOOL bRet = ::CreateProcessW(nullptr, cmdline, nullptr, nullptr, false, CREATE_NEW_CONSOLE, nullptr, nullptr, &si, &pi);
        if (!bRet) {
            auto path = ::_wgetenv(L"ConsoleLoggerHelper");
            if (path) {
                std::swprintf(cmdline, MAX_PATH, L"%ls %ls", path, config.logger_name);
                bRet = ::CreateProcessW(nullptr, nullptr, nullptr, nullptr, false, CREATE_NEW_CONSOLE, nullptr, nullptr, &si, &pi);
            }
            if (!bRet) {
                static bool s_first = true;
                if (s_first)
                    ::MessageBoxW(nullptr, L"Helper executable(console.exe) not found", L"ConsoleLogger failed", MB_ICONERROR);
                s_first = false;
                ::CloseHandle(m_hConsole);
                m_hConsole = INVALID_HANDLE_VALUE;
                return -1;
            }
        }
        // 连接
        BOOL bConnected = ::ConnectNamedPipe(m_hConsole, nullptr) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
        // 连接失败
        if (!bConnected) {
            ::MessageBoxW(nullptr, L"ConnectNamedPipe failed", L"ConsoleLogger failed", MB_ICONERROR);
            ::CloseHandle(m_hConsole);
            m_hConsole = INVALID_HANDLE_VALUE;
            return -1;
        }

        DWORD cbWritten;

        // 特性
        constexpr size_t bufferl = 128;
        wchar_t buffer[bufferl];
        // 传送标题
        if (!lpszWindowTitle) lpszWindowTitle = m_name + 9;
        std::swprintf(buffer, bufferl, L"TITLE: %ls\r\n", lpszWindowTitle);
        uint32_t len_in_byte = static_cast<uint32_t>(std::wcslen(buffer) * sizeof(wchar_t));
        ::WriteFile(m_hConsole, buffer, len_in_byte, &cbWritten, nullptr);
        if (cbWritten != len_in_byte) {
            ::MessageBoxW(nullptr, L"WriteFile failed(1)", L"ConsoleLogger failed", MB_ICONERROR);
            ::DisconnectNamedPipe(m_hConsole);
            ::CloseHandle(m_hConsole);
            m_hConsole = INVALID_HANDLE_VALUE;
            return -1;
        }

        // 传送位置
        if (config.position_xy != -1) {
            std::swprintf(buffer, bufferl, L"POS: %d\r\n", config.position_xy);
            len_in_byte = static_cast<uint32_t>(std::wcslen(buffer) * sizeof(wchar_t));
            ::WriteFile(m_hConsole, buffer, len_in_byte, &cbWritten, nullptr);
            if (cbWritten != len_in_byte) {
                ::MessageBoxW(nullptr, L"WriteFile failed(1.1)", L"ConsoleLogger failed", MB_ICONERROR);
                ::DisconnectNamedPipe(m_hConsole);
                ::CloseHandle(m_hConsole);
                m_hConsole = INVALID_HANDLE_VALUE;
                return -1;
            }
        }
        // 传送属性
        if (config.atribute) {
            std::swprintf(buffer, bufferl, L"ATTR: %d\r\n", config.atribute);
            len_in_byte = static_cast<uint32_t>(std::wcslen(buffer) * sizeof(wchar_t));
            ::WriteFile(m_hConsole, buffer, len_in_byte, &cbWritten, nullptr);
            if (cbWritten != len_in_byte) {
                ::MessageBoxW(nullptr, L"WriteFile failed(1.2)", L"ConsoleLogger failed", MB_ICONERROR);
                ::DisconnectNamedPipe(m_hConsole);
                ::CloseHandle(m_hConsole);
                m_hConsole = INVALID_HANDLE_VALUE;
                return -1;
            }
        }

        // 传送缓存区大小
        if (config.buffer_size_x != -1 && config.buffer_size_y != -1) {
            std::swprintf(buffer, bufferl, L"BUFFER-SIZE: %dx%d\r\n", config.buffer_size_x, config.buffer_size_y);
            len_in_byte = static_cast<uint32_t>(std::wcslen(buffer) * sizeof(wchar_t));
            ::WriteFile(m_hConsole, buffer, len_in_byte, &cbWritten, nullptr);
            if (cbWritten != len_in_byte) {
                ::MessageBoxW(nullptr, L"WriteFile failed(2)", L"ConsoleLogger failed", MB_ICONERROR);
                ::DisconnectNamedPipe(m_hConsole);
                ::CloseHandle(m_hConsole);
                m_hConsole = INVALID_HANDLE_VALUE;
                return -1;
            }
        }

        // 添加头
        if (false) {
            ::DisconnectNamedPipe(m_hConsole);
            ::CloseHandle(m_hConsole);
            m_hConsole = INVALID_HANDLE_VALUE;
            return -1;
        }

        // 传送完毕
        buffer[0] = 0;
        ::WriteFile(m_hConsole, buffer, 2, &cbWritten, nullptr);
        if (cbWritten != 2) {
            ::MessageBoxW(nullptr, L"WriteFile failed(3)", L"ConsoleLogger failed", MB_ICONERROR);
            ::DisconnectNamedPipe(m_hConsole);
            ::CloseHandle(m_hConsole);
            m_hConsole = INVALID_HANDLE_VALUE;
            return -1;
        }
        // 关闭进程句柄
        if (bRet) {
            ::CloseHandle(pi.hProcess);
            ::CloseHandle(pi.hThread);
        }
        return 0;
    }
}
#endif

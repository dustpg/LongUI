// ConsoleLoggerHelper.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <windows.h>
#include <cwchar>
#include <clocale>
#include <VersionHelpers.h>
#include <cassert>

#define lengthof(a) sizeof(a)/sizeof(*a)

CONSOLE_SCREEN_BUFFER_INFO  g_ConsoleBufferInfo;
HANDLE  g_hPipe;
BOOL    g_bExtendedConsole = FALSE;
HANDLE  g_hConsole = INVALID_HANDLE_VALUE;
DWORD   g_dwConsoleSize = 0;
COORD   g_dwConsoleCoords;


long ConsoleLoop(void);
long ConsoleExLoop(void);
void cls(WORD color);
void cls(void);
void clear_eol(void);
void clear_eol(WORD color);
void gotoxy(int x, int y);
int getxy(int *x, int *y);
int getx(void);
int gety(void);


template <class T>
T ReadPipe(T &v) {
    DWORD cbRead;
    if (!::ReadFile(g_hPipe, &v, sizeof(T), &cbRead, nullptr) || cbRead != sizeof(T)) {
        return 0;
    }
    return v;
}

WORD ReadWord(void) {
    WORD w;
    return ::ReadPipe(w);
}
DWORD ReadDword(void) {
    DWORD d;
    return ::ReadPipe(d);
}
BYTE ReadByte(void) {
    BYTE b;
    return ::ReadPipe(b);
}


// 控制台循环
long ConsoleLoop(void) {
    DWORD cbRead, cbWritten=0;
    wchar_t chBuf[1024 * 8 + 2];
    BOOL bSuccess;
    long lResult = -1;
    do  {
        // 读取管道
        bSuccess = ::ReadFile(g_hPipe, chBuf, sizeof(chBuf), &cbRead, nullptr);
        // 检查错误
        if (!bSuccess && ::GetLastError() != ERROR_MORE_DATA) break;
        // 写入到STDOUT
        chBuf[cbRead / sizeof(wchar_t)] = 0;
        std::wprintf(chBuf);
        /*if (!WriteFile(g_hConsole, chBuf, cbRead, &cbWritten, nullptr)) {
            break;
        }*/

    } while (true/*!bSuccess*/);
    return lResult;
}

// 
long ConsoleExLoop(void) {
    enum enumCommands : DWORD {
        COMMAND_PRINT,
        COMMAND_CPRINT,
        COMMAND_CLEAR_SCREEN,
        COMMAND_COLORED_CLEAR_SCREEN,
        COMMAND_GOTOXY,
        COMMAND_CLEAR_EOL,
        COMMAND_COLORED_CLEAR_EOL
    };

    DWORD cbRead, cbWritten;
    char chBuf[1024];
    BOOL bSuccess;
    long lResult = -1;

    DWORD dwCommand;
    bool loop = true;
    while (loop) {
        //////////////////////////////////////////////////////////////////////////
        // 读取DWORD做为命令
        //////////////////////////////////////////////////////////////////////////
        cbRead = 0;
        if (!::ReadFile(g_hPipe, &dwCommand, sizeof(dwCommand), &cbRead, nullptr) || cbRead != sizeof(dwCommand)) {
            loop = false;
            break;
        }

        char first_byte_of_command = (char)((DWORD)dwCommand >> 24);
        switch (first_byte_of_command)
        {
        case COMMAND_PRINT:
        {
            int size = dwCommand & 0x00FFFFFF;
            while (size)  {
                bSuccess = ::ReadFile(g_hPipe,
                    chBuf,
                    (size > sizeof(chBuf)) ? sizeof(chBuf) : size,
                    &cbRead,
                    nullptr);
                if (!bSuccess) {
                    loop = false;
                    break;
                }
                if (!::WriteFile(g_hConsole, chBuf, cbRead, &cbWritten, nullptr)) {
                    loop = false;
                    break;
                }
                size -= cbRead;
            }

        }
        break;
        case COMMAND_CPRINT:
        {
            DWORD attributes;
            ::ReadPipe(attributes);
            ::SetConsoleTextAttribute(g_hConsole, (WORD)attributes);
            int size = dwCommand & 0x00FFFFFF;
            while (size) {
                bSuccess = ::ReadFile(
                    g_hPipe,
                    chBuf,
                    (size > sizeof(chBuf)) ? sizeof(chBuf) : size,
                    &cbRead,
                    nullptr
                    );
                if (!bSuccess) {
                    loop = false;
                    break;
                }

                if (!WriteFile(g_hConsole, chBuf, cbRead, &cbWritten, nullptr)) {
                    loop = false;
                    break;
                }
                size -= cbRead;
            }

        }
        break;
        case COMMAND_CLEAR_SCREEN:
            ::cls();
            break;
        case COMMAND_COLORED_CLEAR_SCREEN:
        {
            DWORD attributes;
            ::ReadPipe(attributes);
            ::cls((unsigned short)attributes);
        }
        break;
        case COMMAND_GOTOXY:
        {
            DWORD xy;
            ::ReadPipe(xy);
            ::gotoxy(xy >> 16, xy & 0x0000FFFF);
        }
        break;
        case COMMAND_CLEAR_EOL:
        {
            ::clear_eol();
        }
        break;

        case COMMAND_COLORED_CLEAR_EOL:
        {
            DWORD attributes;
            ::ReadPipe(attributes);
            ::clear_eol((unsigned short)attributes);
        }
        break;

        }
    }
    return lResult;
}


int wmain(int argc, wchar_t* argv[]){ 
    // 参数检查
    if (argc == 1 || !argv[1] || !argv[1][0]) {
        ::MessageBoxW(nullptr, L"\nFailed to start logger\n", L"FAILED", MB_ICONERROR);
        return -1;
    }

    wchar_t szPipename[64];
    std::swprintf(szPipename, lengthof(szPipename), L"\\\\.\\pipe\\%s", argv[1]);
    // 主循环
    while (true) {
        g_hPipe = ::CreateFileW(
            szPipename,
            GENERIC_READ/*|GENERIC_WRITE*/ /* GENERIC_WRITE*/,
            0,
            nullptr,
            OPEN_EXISTING,
            0,
            nullptr
            );
        // 无效退出
        if (g_hPipe != INVALID_HANDLE_VALUE)
            break;
        // 检查错误
        if (::GetLastError() != ERROR_PIPE_BUSY)  {
            ::MessageBoxW(nullptr, L"Could not open pipe(1)", L"FAILED", MB_ICONERROR);
            (void)::getchar();
            return 0;
        }
        // 忙时等待
        std::wprintf(L"Wait for pipe...");
        if (!::WaitNamedPipeW(szPipename, 20000)) {
            ::MessageBoxW(nullptr, L"Could not open pipe(2)", L"FAILED", MB_ICONERROR);
            (void)::getchar();
            return 0;
        }
        // 等待
        ::Sleep(500);
    }
    // 转换为message-read模式
    // The pipe connected; change to message-read mode.

    BOOL bSuccess;
    DWORD cbRead;
    //////////////////////////////////////////////////////////////////////////
    // Read header
    //////////////////////////////////////////////////////////////////////////
    {
        wchar_t *p1, *p2;
        wchar_t header[1024*4];
        int len = 0;
        wchar_t ch = 1;

        header[lengthof(header) - 1] = 0;

        do {
            // 读取管道
            bSuccess = ::ReadFile(
                g_hPipe,
                &ch,
                sizeof(ch),
                &cbRead,
                nullptr
                ); 
            // 检查错误
            if (!bSuccess && ::GetLastError() != ERROR_MORE_DATA)  {
                MessageBoxW(nullptr, L"ReadFile failed", L"FAILED", MB_ICONERROR);
                return -1;
            }
            header[len++] = ch;
        } while (ch && len <= lengthof(header) - 1);
        header[len] = 0;
        // 设置标题
        {
            if ((p1 = std::wcsstr(header, L"TITLE:"))) {
                p1 += 6;
                while (*p1 == L' ' || *p1 == L'\t')
                    p1++;

                p2 = p1;
                while (*p2 && *p2 != L'\r' && *p2 != L'\n')
                    p2++;
                ch = *p2;
                *p2 = 0;
                ::SetConsoleTitleW(p1);
                *p2 = ch;
            }

        }
        {
            // 设置位置
            if ((p1 = std::wcsstr(header, L"POS:"))) {
                p1 += 4;
                while (*p1 == L' ' || *p1 == L'\t')
                    p1++;

                p2 = p1;
                while (*p2 && *p2 != L'\r' && *p2 != L'\n')
                    p2++;
                ch = *p2;
                *p2 = 0;
                union { UINT32 all; INT16 xy[2]; };
                all = UINT32(::_wtoi(p1));
                ::SetWindowPos(
                    ::GetConsoleWindow(), nullptr,
                    xy[0], xy[1], 0, 0,
                    SWP_NOSIZE
                    );
                *p2 = ch;
            }
        }
        {
            // 设置属性
            if ((p1 = std::wcsstr(header, L"ATTR:"))) {
                p1 += 5;
                while (*p1 == L' ' || *p1 == L'\t')
                    p1++;

                p2 = p1;
                while (*p2 && *p2 != L'\r' && *p2 != L'\n')
                    p2++;
                ch = *p2;
                *p2 = 0;
                UINT32 all = UINT32(::_wtoi(p1));
                ::SetConsoleTextAttribute(
                    ::GetStdHandle(STD_OUTPUT_HANDLE),
                    all
                    );

                *p2 = ch;
            }
        }
        // 设置缓冲区
        // Set buffer-size
        {
            COORD coord;
            coord.X = coord.Y = 0;
            if (p1 = std::wcsstr(header, L"BUFFER-SIZE:")) {
                p1 += 12;
                while (*p1 == L' ' || *p1 == L'\t') ++p1;

                p2 = p1;
                while (*p2 && *p2 != L',' && *p2 != L'x') ++p2;
                ch = *p2;
                *p2 = 0;
                coord.X = (SHORT)::_wtoi(p1);
                *p2 = ch;
                p1 = p2 + 1;
                while (*p2 && *p2 != L'\r' && *p2 != L'\n') ++p2;
                ch = *p2;
                *p2 = 0;
                coord.Y = (SHORT)::_wtoi(p1);
                *p2 = ch;
                // 有效
                if (coord.X && coord.Y)
                    ::SetConsoleScreenBufferSize(::GetStdHandle(STD_OUTPUT_HANDLE), coord);
            }

        }
        // 检查是否为"extended"控制台
        {
            if (std::wcsstr(header, L"Extended-Console: TRUE"))
                g_bExtendedConsole = TRUE;
        }


    }
    std::setlocale(LC_ALL, "");
    //////////////////////////////////////////////////////////////////////////
    // 获取静态信息
    //////////////////////////////////////////////////////////////////////////
    g_hConsole = ::GetStdHandle(STD_OUTPUT_HANDLE);
    if (!::GetConsoleScreenBufferInfo(g_hConsole, &g_ConsoleBufferInfo)) {
        ::MessageBoxW(nullptr, L"GetConsoleScreenBufferInfo failed", L"FAILURE", MB_OK);
        return -3;
    }
    g_dwConsoleCoords = g_ConsoleBufferInfo.dwSize;
    g_dwConsoleSize = g_ConsoleBufferInfo.dwSize.X * g_ConsoleBufferInfo.dwSize.Y;





    if (g_bExtendedConsole)
        ::ConsoleExLoop();
    else
        ::ConsoleLoop();

    ::CloseHandle(g_hPipe);


    //::wprintf(L"\r\nPress any key to end...\r\n");
    //::getchar();
    std::wprintf(L"\r\nConsoleHelper Ended\r\n");
    return 0;
}


//////////////////////////////////////////////////////////////////////////
// Console Functions
//////////////////////////////////////////////////////////////////////////

void cls(WORD color) {
    static COORD coordScreen = { 0, 0 };
    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    ::SetConsoleTextAttribute(g_hConsole, color);
    ::FillConsoleOutputCharacterW(g_hConsole, L' ', g_dwConsoleSize, coordScreen, &cCharsWritten);
    ::GetConsoleScreenBufferInfo(g_hConsole, &csbi);
    ::FillConsoleOutputAttribute(g_hConsole, csbi.wAttributes, g_dwConsoleSize, coordScreen, &cCharsWritten);
    ::SetConsoleCursorPosition(g_hConsole, coordScreen);
}

void cls(void) {
    static COORD coordScreen = { 0, 0 };
    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi; 
    ::SetConsoleTextAttribute(g_hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    ::FillConsoleOutputCharacterW(g_hConsole, L' ', g_dwConsoleSize, coordScreen, &cCharsWritten);
    ::GetConsoleScreenBufferInfo(g_hConsole, &csbi);
    ::FillConsoleOutputAttribute(g_hConsole, csbi.wAttributes, g_dwConsoleSize, coordScreen, &cCharsWritten);
    ::SetConsoleCursorPosition(g_hConsole, coordScreen);
}

void gotoxy(int x, int y) {
    COORD coordScreen = { (SHORT)x,(SHORT)y };
    /*coordScreen.X=x;
    coordScreen.Y=y;*/
    ::SetConsoleCursorPosition(g_hConsole, coordScreen);
}

int getxy(int *x, int *y) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    if (!::GetConsoleScreenBufferInfo(g_hConsole, &csbi))
        return -1;
    if (x)
        *x = csbi.dwCursorPosition.X;
    if (y)
        *y = csbi.dwCursorPosition.Y;
    return 0;
}

int getxy(COORD &coord) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!::GetConsoleScreenBufferInfo(g_hConsole, &csbi))
        return -1;
    coord = csbi.dwCursorPosition;
    return 0;
}

int getx(void) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    if (!::GetConsoleScreenBufferInfo(g_hConsole, &csbi))
        return -1;
    return  csbi.dwCursorPosition.X;
}

int gety(void) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    if (!::GetConsoleScreenBufferInfo(g_hConsole, &csbi))
        return -1;
    return  csbi.dwCursorPosition.Y;
}

void clear_eol(void) {
    DWORD dummy;
    COORD xy;
    ::getxy(xy);
    int size_to_fill = g_dwConsoleCoords.X - xy.X;
    ::FillConsoleOutputCharacterW(g_hConsole, L' ', size_to_fill, xy, &dummy);
    ::FillConsoleOutputAttribute(
        g_hConsole,
        FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
        size_to_fill, xy, &dummy
        );
}

void clear_eol(WORD color) {
    DWORD dummy;
    COORD xy;
    ::getxy(xy);
    int size_to_fill = g_dwConsoleCoords.X - xy.X;
    ::FillConsoleOutputCharacterW(g_hConsole, L' ', size_to_fill, xy, &dummy);
    ::FillConsoleOutputAttribute
        (g_hConsole,
        color,
        size_to_fill, xy, &dummy
        );
}

// 初始化库
class InitializeLibrary {
    typedef enum PROCESS_DPI_AWARENESS {
        PROCESS_DPI_UNAWARE = 0,
        PROCESS_SYSTEM_DPI_AWARE = 1,
        PROCESS_PER_MONITOR_DPI_AWARE = 2
    } PROCESS_DPI_AWARENESS;
    // SetProcessDpiAwareness
    static HRESULT STDAPICALLTYPE SetProcessDpiAwarenessF(PROCESS_DPI_AWARENESS);
public:
    //
    InitializeLibrary() {
        // >= Win8.1 ?
        if (::IsWindows8OrGreater()) {
            m_hDllShcore = ::LoadLibraryW(L"Shcore.dll");
            assert(m_hDllShcore);
            if (m_hDllShcore) {
                auto setProcessDpiAwareness =
                    reinterpret_cast<decltype(&InitializeLibrary::SetProcessDpiAwarenessF)>(
                        ::GetProcAddress(m_hDllShcore, "SetProcessDpiAwareness")
                        );
                assert(setProcessDpiAwareness);
                if (setProcessDpiAwareness) {
                    setProcessDpiAwareness(InitializeLibrary::PROCESS_PER_MONITOR_DPI_AWARE);
                }
            }
        }
    };
    //
    ~InitializeLibrary() {
        if (m_hDllShcore) {
            ::FreeLibrary(m_hDllShcore);
            m_hDllShcore = nullptr;
        }
    }
private:
    // Shcore
    HMODULE     m_hDllShcore = nullptr;
} instance;
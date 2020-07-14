// ui
#include <luiconf.h>
#include <core/ui_basic_type.h>
// c++
#include <cassert>
// windows
#include <Windows.h>
#include <Winuser.h>

// longui::impl
namespace LongUI { namespace impl {
    // dpi awareness
    enum process_dpi_awareness {
        dpi_unaware = 0,
        dpi_system = 1,
        dpi_per_monitor = 2,
    };
    // monitor dpi type
    enum monitor_dpi_type {
        mdt_effective_dpi,
        mdt_angular_dpi,
        mdt_raw_dpi,
        mdt_default
    };
    // dll name
    static constexpr const char* dpi_support_dll = "Shcore.dll";
    // windows api
    struct windows_api {
        // AdjustWindowRectExForDpi
        BOOL(WINAPI* adjust) (LPRECT, DWORD, BOOL, DWORD, UINT) noexcept;
        // GetDpiForMonitor
        HRESULT(WINAPI* get_dpi)(HMONITOR, monitor_dpi_type, UINT*, UINT*) noexcept;
    } g_api;
    // init HiDPI
    void init_high_dpi_support() noexcept {
        // Win10API
        {
            const auto dll = ::GetModuleHandleA("User32.dll");
            const auto api = ::GetProcAddress(dll, "AdjustWindowRectExForDpi");
            reinterpret_cast<FARPROC&>(g_api.adjust) = api;
        }
        using pda = process_dpi_awareness;
        // 万能union转换
        union { HRESULT(WINAPI* set_pda)(pda) noexcept; FARPROC call; };
        // XXX: 使用编号
        const auto SETPROCESSDPIAWARENESS_NAME = "SetProcessDpiAwareness";
        const auto GETDPIFORMONITOR_NAME = "GetDpiForMonitor";
        // 载入
#ifdef LUI_WIN10_ONLY
        const auto dll = ::LoadLibraryA(dpi_support_dll);
        assert(dll && "support windows 10 only");
        call = ::GetProcAddress(dll, SETPROCESSDPIAWARENESS_NAME);
        assert(call && "support windows 10 only");
        ptr_GetDpiForMonitor = ::GetProcAddress(dll, GETDPIFORMONITOR_NAME);
        set_pda(dpi_per_monitor);
#else
        // win 8
        if (const auto dll = ::LoadLibraryA(dpi_support_dll)) {
            // win 8.1
            if ((call = ::GetProcAddress(dll, SETPROCESSDPIAWARENESS_NAME))) {
                const auto api = ::GetProcAddress(dll, GETDPIFORMONITOR_NAME);
                reinterpret_cast<FARPROC&>(g_api.get_dpi) = api;
                // 设置DPI支持等级
                if (set_pda(dpi_per_monitor) == E_INVALIDARG)
                    set_pda(dpi_system);
                return;
            }
        }
        // win7/win8 
        ::SetProcessDPIAware();
#endif
    }
    /// <summary>
    /// Uninits the high dpi support.
    /// </summary>
    /// <returns></returns>
    void uninit_high_dpi_support() noexcept {
#ifdef LUI_WIN10_ONLY
        ::FreeLibrary(::GetModuleHandleA(dpi_support_dll));
#else
        if (const auto dll = ::GetModuleHandleA(dpi_support_dll)) {
            ::FreeLibrary(dll);
        }
#endif
    }
    /// <summary>
    /// Gets the dpi from Rect.
    /// </summary>
    /// <param name="rect">The rect.</param>
    /// <returns></returns>
    auto get_dpi_from_rect(const RectL& rect) noexcept -> uint32_t {
        //return { 3.f, 3.f };
#ifndef LUI_WIN10_ONLY
        // WIN7/Win8 只支持相同的DPI
        if (!g_api.get_dpi) {
            static const auto dpi_x = []() noexcept ->uint32_t {
                const auto hdc = ::GetDC(nullptr);
                const auto dpi_x = ::GetDeviceCaps(hdc, LOGPIXELSX);
                return static_cast<uint32_t>(dpi_x);
            }();
            return dpi_x;
        }
#endif
        UINT dpi[2] = { 0 };
        const auto ptr = reinterpret_cast<LPCRECT>(&rect);
        const auto monitor = ::MonitorFromRect(ptr, MONITOR_DEFAULTTOPRIMARY);
        const auto hr = g_api.get_dpi(monitor, mdt_effective_dpi, dpi, dpi + 1);
        if (FAILED(hr)) return BASIC_DPI;
        return dpi[0];
    }
    /// <summary>
    /// Gets the dpi scale from HWND.
    /// </summary>
    /// <param name="hwnd">The HWND.</param>
    /// <returns></returns>
    auto get_dpi_scale_from_hwnd(HWND hwnd) noexcept -> Size2F {
        //return { 3.f, 3.f };
#ifndef LUI_WIN10_ONLY
        // WIN7/Win8 只支持相同的DPI
        if (!g_api.get_dpi) {
            static const Size2F dpi_xy = []() noexcept ->Size2F {
                const auto hdc = ::GetDC(nullptr);
                const auto dpi_x = ::GetDeviceCaps(hdc, LOGPIXELSX);
                const auto dpi_y = ::GetDeviceCaps(hdc, LOGPIXELSY);
                ::ReleaseDC(nullptr, hdc);
                const auto x = float(dpi_x) / float(BASIC_DPI);
                const auto y = float(dpi_y) / float(BASIC_DPI);
                return{ x, y };
            }();
            return dpi_xy;
        }
#endif
        UINT dpi[2] = { 0 };
        const auto monitor = ::MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
        const auto hr = g_api.get_dpi(monitor, mdt_effective_dpi, dpi, dpi + 1);
        if (FAILED(hr)) return { 1.f, 1.f };
        const auto x = float(dpi[0]) / float(BASIC_DPI);
        const auto y = float(dpi[1]) / float(BASIC_DPI);
        return { x, y };
    }
    /// <summary>
    /// adjust window rect
    /// </summary>
    /// <param name="rect"></param>
    /// <param name="style"></param>
    /// <param name="ex"></param>
    /// <param name="dpi"></param>
    /// <returns></returns>
    void adjust_window(RectL& rect, uint32_t style, uint32_t ex, uint32_t dpi) noexcept {
        static_assert(sizeof(RectL) == sizeof(RECT), "SAME!");
        const auto ptr = reinterpret_cast<RECT*>(&rect);
#ifndef LUI_WIN10_ONLY
        if (g_api.adjust) {
            g_api.adjust(ptr, style, FALSE, ex, dpi);
            return;
        }
        ::AdjustWindowRectEx(ptr, style, FALSE, ex);
#else
        g_api.adjust(ptr, style, FALSE, ex, dpi);
#endif
    }
}}


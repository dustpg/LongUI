// ui
#include <luiconf.h>
#include <core/ui_basic_type.h>
// c++
#include <cassert>
// windows
#include <Windows.h>

namespace LongUI {
    // impl
    namespace impl {
        // dll name
        static const char* dpi_support_dll = "Shcore.dll";
        // GetDpiForMonitor
        static FARPROC ptr_GetDpiForMonitor = nullptr;
        // dpi awareness
        enum process_dpi_awareness {
            dpi_unaware = 0,
            dpi_system  = 1,
            dpi_per_monitor = 2,
        };
        // monitor dpi type
        enum monitor_dpi_type {
            mdt_effective_dpi,
            mdt_angular_dpi,
            mdt_raw_dpi,
            mdt_default
        };
        // init HiDPI
        void init_high_dpi_support() noexcept {
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
            set_pda(dpi_per_monitor);
#else
            if (const auto dll = ::LoadLibraryA(dpi_support_dll)) {
                call = ::GetProcAddress(dll, SETPROCESSDPIAWARENESS_NAME);
                if (!call) return;
                ptr_GetDpiForMonitor = ::GetProcAddress(dll, GETDPIFORMONITOR_NAME);
                // 设置DPI支持等级
                if (set_pda(dpi_per_monitor) == E_INVALIDARG)
                    set_pda(dpi_system);
            }
#endif
        }
        /// <summary>
        /// Uninits the high dpi support.
        /// </summary>
        /// <returns></returns>
        void uninit_high_dpi_support() noexcept {
            // FreeLibrary(nullptr) is undefined on doc
#ifdef LUI_WIN10_ONLY
            ::FreeLibrary(::GetModuleHandleA(dpi_support_dll));
#else
            // 释放
            if (const auto dll = ::GetModuleHandleA(dpi_support_dll)) {
                ::FreeLibrary(dll);
            }
#endif
        }
        /// <summary>
        /// Gets the dpi scale from HWND.
        /// </summary>
        /// <param name="hwnd">The HWND.</param>
        /// <returns></returns>
        auto get_dpi_scale_from_hwnd(HWND hwnd) noexcept -> Size2F {
            // XXX: win7的场合?
            if (!ptr_GetDpiForMonitor) return { 1.f, 1.f };
            const auto monitor = ::MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
            union { 
                HRESULT(WINAPI* get_dpi)(HMONITOR, monitor_dpi_type, UINT*, UINT*) noexcept; 
                FARPROC call; 
            };
            call = ptr_GetDpiForMonitor;
            UINT dpi[2] = { 0 };
            const auto hr = get_dpi(monitor, mdt_effective_dpi, dpi, dpi + 1);
            if (FAILED(hr)) return { 1.f, 1.f };
            const auto x = float(dpi[0]) / float(BASIC_DPI);
            const auto y = float(dpi[1]) / float(BASIC_DPI);
            return { x, y };
        }
    }
}


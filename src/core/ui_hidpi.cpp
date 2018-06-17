// ui
#include <luiconf.h>
// c++
#include <cassert>
// windows
#include <Windows.h>

namespace LongUI {
    // impl
    namespace impl {
        // dll name
        static const char* dpi_support_dll = "Shcore.dll";
        // dpi awareness
        enum process_dpi_awareness {
            dpi_unaware = 0,
            dpi_system  = 1,
            dpi_per_monitor = 2,
        };
        // init HiDPI
        void init_high_dpi_support() noexcept {
            using pda = process_dpi_awareness;
            // 万能union转换
            union { HRESULT(WINAPI* set_pda)(pda) noexcept; FARPROC call; };
            // XXX: 使用编号
            const auto SETPROCESSDPIAWARENESS_NAME = "SetProcessDpiAwareness";
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
    }
}


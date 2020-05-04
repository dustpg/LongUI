#include <graphics/ui_cursor.h>
#include <util/ui_unimacro.h>
#include <windows.h>
#include <cassert>

// ui namespace
namespace LongUI {
    // handle list
    static uintptr_t s_cursors[] = {
        reinterpret_cast<uintptr_t>(IDC_ARROW),
        reinterpret_cast<uintptr_t>(IDC_IBEAM),
        reinterpret_cast<uintptr_t>(IDC_WAIT),
        reinterpret_cast<uintptr_t>(IDC_HAND),
        reinterpret_cast<uintptr_t>(IDC_HELP),
        reinterpret_cast<uintptr_t>(IDC_CROSS),
        reinterpret_cast<uintptr_t>(IDC_SIZEALL),
        reinterpret_cast<uintptr_t>(IDC_UPARROW),
        reinterpret_cast<uintptr_t>(IDC_SIZENWSE),
        reinterpret_cast<uintptr_t>(IDC_SIZENESW),
        reinterpret_cast<uintptr_t>(IDC_SIZEWE),
        reinterpret_cast<uintptr_t>(IDC_SIZENS),
    };
    // impl
    namespace impl {
        /// <summary>
        /// Initializes the cursor.
        /// </summary>
        /// <returns></returns>
        void init_cursor() noexcept {
            assert(s_cursors[0] == reinterpret_cast<uintptr_t>(IDC_ARROW));
            for (auto& x : s_cursors) {
                const auto ptr = reinterpret_cast<const wchar_t*>(x);
                x = reinterpret_cast<uintptr_t>(::LoadCursorW(nullptr, ptr));
            }
        }
        /// <summary>
        /// Uninits the cursor.
        /// </summary>
        /// <returns></returns>
        void uninit_cursor() noexcept {

        }
    }
}

PCN_NOINLINE
/// <summary>
/// Initializes a new instance of the <see cref="CUICursor"/> class.
/// </summary>
/// <param name="dc">The dc.</param>
LongUI::CUICursor::CUICursor(DefaultCursor dc) noexcept  {
    constexpr uintptr_t ccc = sizeof(s_cursors) / sizeof(s_cursors[0]);
    static_assert(ccc == CUICursor::CURSOR_COUNT, "must be same");
    if (dc >= CURSOR_COUNT) dc = Cursor_Arrow;
    m_handle = s_cursors[dc];
}
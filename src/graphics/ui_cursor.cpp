#include <graphics/ui_cursor.h>
#include <util/ui_unimacro.h>
#include <windows.h>

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
}

PCN_NOINLINE
/// <summary>
/// Initializes a new instance of the <see cref="CUICursor"/> class.
/// </summary>
/// <param name="dc">The dc.</param>
LongUI::CUICursor::CUICursor(DefaultCursor dc) noexcept {
    constexpr uintptr_t ccc = sizeof(s_cursors) / sizeof(s_cursors[0]);
    static_assert(ccc == CUICursor::CURSOR_COUNT, "must be same");
    if (dc >= CURSOR_COUNT) dc = Cursor_Arrow;
    // TODO: 检查
    m_handle = reinterpret_cast<uintptr_t>(::LoadCursorW(
        nullptr,
        reinterpret_cast<const wchar_t*>(s_cursors[dc])
    ));
}
#pragma once

#include <cstdint>

namespace LongUI {
    // cursor
    class CUICursor final {
    public:
        // default
        enum DefaultCursor : uintptr_t {
            // arraw
            Cursor_Arrow = 0,
            // 'I' beam
            Cursor_Ibeam,
            // wait
            Cursor_Wait,
            // hand
            Cursor_Hand,
            // help
            Cursor_Help,
            // + cross
            Cursor_Cross,
            // 4-pointed arrow
            Cursor_SizeAll,
            // up arrow
            Cursor_UpArrow,
            // NW-SE arrow
            Cursor_SizeNWSE,
            // NE-SW arrow
            Cursor_SizeNESW,
            // W-E arrow
            Cursor_SizeWE,
            // N-S arrow
            Cursor_SizeNS,
            // count of this
            CURSOR_COUNT
        };
    public:
        // from DefaultCursor
        CUICursor(DefaultCursor) noexcept;
        // copy ctor
        CUICursor(const CUICursor&) noexcept = default;
        // dtor
        ~CUICursor() noexcept = default;
        // get handle
        auto GetHandle() const noexcept { return m_handle; }
    private:
        // handle to cursor
        uintptr_t           m_handle;
    };
}
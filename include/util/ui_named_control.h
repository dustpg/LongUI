#pragma once

#include <cstdint>
#include <cassert>

// ui namespace
namespace LongUI {
    // control 
    class UIControl;
    // window
    class CUIWindow;
    // named control to find
    union NamedControl {
        // ctor
        NamedControl() noexcept : ctrl(nullptr) {}
#ifndef NDEBUG
        // dtor
        ~NamedControl() noexcept { assert((value & 1) == 0 && "must call Find if SetName"); }
#endif
        // set name
        void SetName(const char* begin, const char* end) noexcept;
        // set control
        void SetControl(UIControl* c) noexcept { assert((value & 1) == 0); ctrl = c; }
        // find control window
        void FindControl(CUIWindow* window) noexcept;
        // control ptr
        UIControl*      ctrl;
        // control name
        const char*     name;
        // pointer value
        uintptr_t       value;
    };
}
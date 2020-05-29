#pragma once

// ui
#include "../core/ui_core_type.h"
#include <cstdint>

// ui namepsace
namespace LongUI {
    // behavior type
    enum class BehaviorType : uint8_t {
        Type_Normal = 0,    // normal type
        Type_Checkbox,      // toggle button
        Type_Radio,         // radio-like button
        Type_Menu,          // menu
    };
    // parse button type
    auto ParseBehaviorType(U8View) noexcept->BehaviorType;
}

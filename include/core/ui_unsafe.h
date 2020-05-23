#pragma once

#include "../control/ui_control.h"

namespace LongUI {
    // use this carefully 
    struct Unsafe {
        // add attribute before inited
        static void AddAttrUninited(UIControl& ctrl, uint32_t key, U8View value) noexcept {
            ctrl.add_attribute(key, value);
        }
    };
}

#pragma once

// ui
#include "ui_node.h"
#include "../luiconf.h"
#include "ui_core_type.h"
#include "ui_basic_type.h"
// c++
#include <cstddef>

namespace LongUI {
    // window
    class CUIWindow;
    /// <summary>
    /// window event handle class
    /// </summary>
    class CUIWindowEvent : public Node<CUIWindow> {
    public:
        // on window event: Resize
        //void OnResize(Size2U size) noexcept;
        // on window event: Closed
        //void OnClosed() noexcept;
    };
}
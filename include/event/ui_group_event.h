#pragma once

#include <cstdint>
#include "ui_notice_event.h"
#include "../core/ui_core_type.h"

namespace LongUI {
    // gui event, EventAccept: have no effect 
    struct ImplicitGroupGuiArg : EventArg {
        // group name
        const char* const group_name;
        // ctor
        ImplicitGroupGuiArg(const char* g) noexcept : group_name(g) {
            this->nevent = NoticeEvent::Event_ImplicitGroupChecked;
        }
    };
    // do ImplicitGroupGuiArg
    void DoImplicitGroupGuiArg(UIControl&, const char* group) noexcept;
}
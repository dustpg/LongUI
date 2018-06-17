#pragma once

#include "ui_notice_event.h"
#include <core/ui_basic_type.h>

namespace LongUI {
    // initialize event, EventAccept: have no effect 
    struct EventInitializeArg : EventArg {
        // ctor
        EventInitializeArg() noexcept { 
            this->nevent = NoticeEvent::Event_Initialize;
            this->derived = static_cast<uint32_t>(Result::RS_OK);
        }
        // set result
        void SetResult(Result hr) const noexcept {
            const auto code = static_cast<uint32_t>(hr.code);
            const_cast<uint32_t&>(this->derived) = code;
        }
        // get result
        auto GetResult() const noexcept -> Result { 
            return{ static_cast<int32_t>(this->derived) };
        }
    };
}
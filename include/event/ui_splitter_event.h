#pragma once

#include <cstdint>
#include "ui_notice_event.h"

namespace LongUI {
    // splitter event, EventAccept: have no effect 
    struct EventSplitterArg : EventArg {
        // offset x
        const float     offset_x;
        // offset y
        const float     offset_y;
        // ctor
        EventSplitterArg(float x, float y) noexcept:
            offset_x(x), offset_y(y) {
            nevent = NoticeEvent::Event_Splitter;
            derived = 0;
        }
    };
}
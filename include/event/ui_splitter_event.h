#pragma once

#include <cstdint>
#include "ui_notice_event.h"
#include "../style/ui_attribute.h"

namespace LongUI {
    // Attribute Splitter
    struct SplitterAttribute {
        // resize mode
        AttributeResize     resizebefore;
        // resize mode
        AttributeResize     resizeafter;
        // collapse
        AttributeCollapse   collapse;
        // drag off to change
        bool                dragoff;
    };
    // splitter event, EventAccept: have no effect 
    struct EventSplitterArg : EventArg {
        // offset x
        const float         offset_x;
        // offset y
        const float         offset_y;
        // Attribute Splitter
        SplitterAttribute   attribute;
        // ctor
        EventSplitterArg(float x, float y, SplitterAttribute a) noexcept:
            offset_x(x), offset_y(y), attribute(a) {
            nevent = NoticeEvent::Event_Splitter;
            derived = 0;
        }
    };
}
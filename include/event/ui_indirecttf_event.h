#pragma once

#include <cstdint>
#include "ui_notice_event.h"
#include "../core/ui_core_type.h"

namespace LongUI {
    // TextFont
    struct TextFont; using TextFontPtr = TextFont * ;
    // gui event, EventAccept: have no effect 
    struct IndirectTextFontGuiArg : EventArg {
        // group name
        mutable TextFontPtr     text_font;
        // ctor
        IndirectTextFontGuiArg() noexcept : text_font(nullptr) {
            this->nevent = NoticeEvent::Event_IndirectTextFont;
        }
    };
}

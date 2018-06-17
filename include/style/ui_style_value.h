#pragma once

#include "ui_ssvalue_list.h"
#include "ui_attribute.h"
#include "../core/ui_color.h"

namespace LongUI {
    /// <summary>
    /// gui style value host
    /// </summary>
    class CUIStyleValue {
    public:
        // set background clip
        void SetBgClip(AttributeBox clip) noexcept;
        // set background color
        void SetBgColor(RGBA color) noexcept;
        // set background image from resource id
        void SetBgImage(uint32_t id) noexcept;
        // set background repeat
        void SetBgRepeat(AttributeRepeat ar) noexcept;
        // set background origin
        void SetBgOrigin(AttributeBox ab) noexcept;
        // set background attachment
        void SetBgAttachment(AttributeAttachment aa) noexcept;
    };
}
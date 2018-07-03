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
        // set foreground color
        void SetFgColor(RGBA color) noexcept;
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
    public:
        // get background clip
        auto GetBgClip() const noexcept->AttributeBox;
        // get foreground color
        auto GetFgColor() const noexcept->RGBA;
        // get background color
        auto GetBgColor() const noexcept->RGBA;
        // get background image resource id
        auto GetBgImage() const noexcept->uint32_t;
        // get background repeat
        auto GetBgRepeat() const noexcept->AttributeRepeat;
        // get background origin
        auto GetBgOrigin() const noexcept->AttributeBox;
    };
}
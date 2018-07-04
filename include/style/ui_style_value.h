#pragma once

#include "ui_ssvalue_list.h"
#include "ui_attribute.h"
#include "../core/ui_color.h"

namespace LongUI {
    /// <summary>
    /// gui style value host
    /// </summary>
    class CUIStyleValue {
        // after box changed
        void after_box_changed();
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
    public:
        // set margin top
        void SetMarginTop(float value) noexcept;
        // set margin left
        void SetMarginLeft(float value) noexcept;
        // set margin right
        void SetMarginRight(float value) noexcept;
        // set margin bottom
        void SetMarginBottom(float value) noexcept;
        // set padding top
        void SetPaddingTop(float value) noexcept;
        // set padding left
        void SetPaddingLeft(float value) noexcept;
        // set padding right
        void SetPaddingRight(float value) noexcept;
        // set padding bottom
        void SetPaddingBottom(float value) noexcept;
        // set border top width
        void SetBorderTop(float value) noexcept;
        // set border left width
        void SetBorderLeft(float value) noexcept;
        // set border right width
        void SetBorderRight(float value) noexcept;
        // set border bottom width
        void SetBorderBottom(float value) noexcept;
    };
}
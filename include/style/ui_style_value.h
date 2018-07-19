#pragma once

#include "ui_ssvalue_list.h"
#include "ui_attribute.h"
#include "../text/ui_attribute.h"
#include "../core/ui_color.h"
#include "../core/ui_basic_type.h"

namespace LongUI {
    /// <summary>
    /// gui style value host
    /// </summary>
    class CUIStyleValue {
        // after box changed
        void after_box_changed();
    public:
        // set text stroke color
        void SetTextStrokeWidth(float width) noexcept;
        // set text stroke color
        void SetTextStrokeColor(RGBA color) noexcept;
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
        // set border image source from resource id
        void SetBdImageSource(uint32_t id) noexcept;
        // set border image slice
        void SetBdImageSlice(const RectF& slice, bool fill) noexcept;
        // set font size
        void SetFontSize(float size) noexcept;
        // set font style
        void SetFontStyle(AttributeFontStyle style) noexcept;
        // set font family(maybe you should use GetUniqueText)
        void SetFontFamily(const char* family) noexcept;
        // set font weight
        void SetFontWeight(AttributeFontWeight weight) noexcept;
        // set font stretch
        void SetFontStretch(AttributeFontStretch stretch) noexcept;
    public:
        // set text stroke color
        auto GetTextStrokeWidth() const noexcept->float;
        // set text stroke color
        auto GetTextStrokeColor() const noexcept->RGBA;
        // get foreground color
        auto GetFgColor() const noexcept->RGBA;
        // get background clip
        auto GetBgClip() const noexcept->AttributeBox;
        // get background color
        auto GetBgColor() const noexcept->RGBA;
        // get background image resource id
        auto GetBgImage() const noexcept->uint32_t;
        // get background repeat
        auto GetBgRepeat() const noexcept->AttributeRepeat;
        // get background origin
        auto GetBgOrigin() const noexcept->AttributeBox;
        // get border image source [resource id]
        auto GetBdImageSource() const noexcept ->uint32_t;
        // get border image slice
        bool GetBdImageSlice(RectF& output) const noexcept;
        // get font size
        auto GetFontSize() const noexcept->float;
        // get font style
        auto GetFontStyle() const noexcept->AttributeFontStyle;
        // get font family
        auto GetFontFamily() const noexcept->const char*;
        // get font weight
        auto GetFontWeight() const noexcept->AttributeFontWeight;
        // get font stretch
        auto GetFontStretch() const noexcept->AttributeFontStretch;
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
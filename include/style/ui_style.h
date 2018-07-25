#pragma once

// ui
#include "../luiconf.h"
#include "ui_ssvalue.h"
#include "ui_attribute.h"
#include "ui_style_state.h"
#include "../core/ui_basic_type.h"
// c++
#include <cassert>

namespace LongUI {
    // Box Model
    struct Box {
        // visible rect[world border edge]
        RectF       visible;
        // box position
        Point2F     pos;
        // box rect
        Size2F      size;
        // margin
        RectF       margin;
        // border
        RectF       border;
        // padding
        RectF       padding;
        // box used min size
        Size2F      minsize;
        // TODO: box used max size 
        Size2F      maxsize;
        // ctor
        void Init() noexcept;
        // get Non-content rect
        auto GetNonContect() const noexcept { RectF rc; GetNonContect(rc); return rc; }
        // get margin edge
        auto GetMarginEdge() const noexcept { RectF rc; GetMarginEdge(rc); return rc; }
        // get border edge
        auto GetBorderEdge() const noexcept { RectF rc; GetBorderEdge(rc); return rc; }
        // get padding edge
        auto GetPaddingEdge() const noexcept { RectF rc; GetPaddingEdge(rc); return rc; }
        // get content edge
        auto GetContentEdge() const noexcept { RectF rc; GetContentEdge(rc); return rc; }
        // get contect size
        auto GetContentSize() const noexcept->Size2F;
        // get border size
        auto GetBorderSize() const noexcept->Size2F;
        // get contect pos
        auto GetContentPos() const noexcept->Point2F;
        // get Non-content rect
        void GetNonContect(RectF&) const noexcept;
        // get margin edge
        void GetMarginEdge(RectF&) const noexcept;
        // get border edge
        void GetBorderEdge(RectF&) const noexcept;
        // get padding edge
        void GetPaddingEdge(RectF&) const noexcept;
        // get content edge
        void GetContentEdge(RectF&) const noexcept;
    };
    // text/font
    struct TextFont;
    // Style model
    struct Style {
        // get TextFont
        auto GetTextFont() noexcept {
            assert(offset_tf && "bad offset");
            const auto ptr = reinterpret_cast<char*>(this) + offset_tf;
            return reinterpret_cast<TextFont*>(ptr);
        }
        // ctor
        Style() noexcept;
        // dtor
        ~Style() noexcept;
        // no copy
        Style(const Style&) noexcept = delete;
        // state            [4]
        StyleState          state;
        // pack             [1]
        AttributePack       pack;
        // t-duration       [2]
        uint16_t            tduration;
        // unused           [2]
        uint16_t            unused2;
        // unused           [4]
        uint32_t            unused4;
        // align            [1]
        AttributeAlign      align;
        // appearance tpye  [1]
        AttributeAppearance appearance;
        // t-timing funtion [1]
        uint8_t             tfunction;
        // offset in byte for text font, set this if support text [2]
        uint16_t            offset_tf;
        // overflow-x       [1]
        AttributeOverflow   overflow_x;
        // overflow-y       [1]
        AttributeOverflow   overflow_y;
        // flex             [4]
        float               flex;
        // style used min size
        Size2F              minsize;
        // style used max size
        Size2F              maxsize;
#ifndef LUI_DISABLE_STYLE_SUPPORT
        // value list matched
        SSValues            matched;
        // trigger
        SSTrigger           trigger;
#endif
    };
}
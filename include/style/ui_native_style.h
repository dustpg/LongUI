#pragma once

#include "ui_style.h"
#include "ui_attribute.h"
#include "../core/ui_basic_type.h"

// ui namespace
namespace LongUI {
    // color
    struct ColorF;
    // draw args
    struct NativeDrawArgs {
        // border edge rect
        RectF               border;
        // state from
        StyleState          from;
        // state to(vaild if progress > 0)
        StyleState          to;
        // progress
        float               progress;
        // type
        AttributeAppearance appearance;
    };
    // get duration args
    struct GetDurationArgs {
        // type
        AttributeAppearance appearance;
    };
    // control class
    // sub element type
    /*enum class SubElement : uint8_t {

    };*/
    // adjust sub element rect in native style
    //void NativeStyleAdjustSubElement(SubElement sube, RectF& rect) noexcept;
    // draw native style
    void NativeStyleDraw(const NativeDrawArgs& args) noexcept;
    // draw native focus rect
    void NativeStyleFocus(const RectF& rect) noexcept;
    // init native style
    void NativeStyleInit(UIControl& ctrl, AttributeAppearance) noexcept;
    // get animation dur
    auto NativeStyleDuration(const GetDurationArgs) noexcept -> uint32_t;
    // get foreground color if changed
    auto NativeFgColor(StyleState now) noexcept->uint32_t;
    // get foreground color if exist
    //bool NativeStyleGetForeground(const NativeDrawArgs& args, ColorF&) noexcept;
}
#pragma once

#include "ui_style.h"
#include "ui_attribute.h"
#include "../core/ui_color.h"
//#include "../core/ui_object.h"
#include "../core/ui_basic_type.h"

// ui namespace
namespace LongUI {
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
    /// <summary>
    /// native style
    /// </summary>
    /// <seealso cref="CUIObject" />
    struct CUINativeStyle /*: CUISmallObject*/ {
        // ctxmenu  clear color
        ColorF          clearcolor_ctxmenu;
        // combobox clear color
        ColorF          clearcolor_combobox;
        // base label margin
        RectF           margin_baselabel;
        // base textfiled margin
        RectF           margin_basetextfiled;
        // draw native style
        void DrawStyle(const NativeDrawArgs& args) noexcept;
        // draw native focus rect
        void DrawFocus(const RectF& rect) noexcept;
        // init native style
        void InitStyle(UIControl& ctrl, AttributeAppearance) noexcept;
        // get animation dur
        auto GetDuration(const GetDurationArgs) noexcept->uint32_t;
        // get foreground color if changed
        auto GetFgColor(StyleState now) noexcept->uint32_t;
    };

    // control class
    // sub element type
    /*enum class SubElement : uint8_t {

    };*/
    // adjust sub element rect in native style
    //void NativeStyleAdjustSubElement(SubElement sube, RectF& rect) noexcept;
}
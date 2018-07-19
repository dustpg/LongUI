#pragma once

#include "../luiconf.h"
#ifndef LUI_DISABLE_STYLE_SUPPORT

#include <core/ui_core_type.h>
#include <cstdint>
#include <cassert>

// simpac namespace
namespace SimpAC {
    // Func Value
    struct FuncValue;
}

namespace LongUI {
    /// <summary>
    /// type of value
    /// </summary>
    enum class ValueType : uint32_t {
        // new one
        Type_NewOne = 0xfffffffful,
        // unknown
        Type_Unknown = 0,

        // [Position] overflow-x
        Type_PositionOverflowX,
        // [Position] overflow-y
        Type_PositionOverflowY,
        // [Position] left 
        Type_PositionLeft,
        // [Position] top 
        Type_PositionTop,

        // [Dimension] width
        Type_DimensionWidth,
        // [Dimension] height
        Type_DimensionHeight,
        // [Dimension] min-width
        Type_DimensionMinWidth,
        // [Dimension] min-height
        Type_DimensionMinHeight,
        // [Dimension] max-width
        Type_DimensionMaxWidth,
        // [Dimension] max-height
        Type_DimensionMaxHeight,

        // [Box] flex
        Type_BoxFlex,

        // [Margin] top
        Type_MarginTop,
        // [Margin] right
        Type_MarginRight,
        // [Margin] bottom
        Type_MarginBottom,
        // [Margin] left
        Type_MarginLeft,

        // [Padding] top
        Type_PaddingTop,
        // [Padding] right
        Type_PaddingRight,
        // [Padding] bottom
        Type_PaddingBottom,
        // [Padding] left
        Type_PaddingLeft,

        // [Border] top-width
        Type_BorderTopWidth,
        // [Border] right-width
        Type_BorderRightWidth,
        // [Border] bottom-width
        Type_BorderBottomWidth,
        // [Border] left-width
        Type_BorderLeftWidth,
        // [Border] image-source
        Type_BorderImageSource,
        // [Border] image-slice
        Type_BorderImageSlice,

        // [Background] color
        Type_BackgroundColor,
        // [Background] image
        Type_BackgroundImage,
        // [Background] attachment
        Type_BackgroundAttachment,
        // [Background] repeat
        Type_BackgroundRepeat,
        // [Background] clip
        Type_BackgroundClip,
        // [Background] origin 
        Type_BackgroundOrigin,

        // [Transition] duration
        Type_TransitionDuration,
        // [Transition] timing funtion
        Type_TransitionTimingFunc,

        // [Text] color
        Type_TextColor,
        // [-Webkit-Text] stroke-width
        Type_WKTextStrokeWidth,
        // [-Webkit-Text] stroke-color
        Type_WKTextStrokeColor,

        // [Font] size
        Type_FontSize,
        // [Font] style
        Type_FontStyle,
        // [Font] stretch
        Type_FontStretch,
        // [Font] weight
        Type_FontWeight,
        // [Font] family
        Type_FontFamily,
        // [LongUI] appearance
        Type_LUIAppearance,

        // last single property
        SINGLE_LAST = Type_LUIAppearance,

        // [ShortHand] overflow
        Type_ShortHandOverflow,
        // [ShortHand] -webkit-text-stroke 
        Type_ShortHandWKTextStroke,
        // [ShortHand] margin
        Type_ShortHandMargin,
        // [ShortHand] padding
        Type_ShortHandPadding,
        // [ShortHand] border-width
        Type_ShortHandBorderWidth,
    };
    // value easy type
    enum class ValueEasyType : int8_t {
        // no animation
        Type_NoAnimation = 0,
        // float
        Type_Float,
        // color
        Type_Color,
        // uint32_t
        //Type_Uint32,
    };
    // union 4
    union UniByte4 {
        // u32 data
        uint32_t    u32;
        // i32 data
        int32_t     i32;
        // u16 data
        uint16_t    word;
        // single float data
        float       single;
        // byte data
        uint8_t     byte;
        // boolean data
        bool        boolean;
    };
    // union 8
    union UniByte8 {
        // u32 data
        uint32_t    u32[2];
        // i32 data
        int32_t     i32[2];
        // u16 data
        uint16_t    word[4];
        // single float data
        float       single[2];
        // double float data
        //double      double_;
        // byte data
        uint8_t     byte[8];
        // boolean data
        bool        boolean[8];
        // u8 string data
        const char* strptr;
    };
    static_assert(sizeof(UniByte8) == sizeof(double), "same!");
    /// <summary>
    /// value of style sheet
    /// </summary>
    struct SSValue {
        // type of value
        ValueType       type;
        // data
        UniByte4        data4;
        // data
        UniByte8        data8;
    };
    // get easy type
    auto GetEasyType(ValueType) noexcept->ValueEasyType;
    // is image
    bool IsImageType(ValueType) noexcept;
    // init states buffer
    void InitStateBuffer(UniByte4[/*ValueType::TYPE_COUNT*/]) noexcept;
    // u8view to value type
    auto U8View2ValueType(U8View view) noexcept->ValueType;
    // make value
    void ValueTypeMakeValue(
        ValueType ex,
        uint32_t value_len,
        const SimpAC::FuncValue values[],
        void* values_write
    ) noexcept;
}

#endif
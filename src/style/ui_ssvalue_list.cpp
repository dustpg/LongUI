// lui
#include <luiconf.h>
#include <core/ui_string_view.h>
// css
#include <xul/SimpAC.h>

namespace LongUI {
    // split unit
    auto SplitUnit(PodStringView<char>& view) noexcept -> PodStringView<char> {
        auto& pair = reinterpret_cast<SimpAC::StrPair&>(view);
        const auto rv = SimpAC::SplitUnit(pair);
        return reinterpret_cast<const PodStringView<char>&>(rv);
    }
    /// <summary>
    /// Finds the last dir.
    /// </summary>
    /// <param name="view">The view.</param>
    /// <returns></returns>
    auto FindLastDir(PodStringView<char> view) noexcept -> PodStringView<char> {
        while (view.end() > view.begin()) {
            --view.second; const auto ch = *view.end();
            if (ch == '/' || ch == '\\') break;
        }
        return view;
    }
    /// <summary>
    /// Splits the string.
    /// </summary>
    /// <param name="a">a.</param>
    /// <param name="b">The b.</param>
    /// <returns></returns>
    auto SplitStr(PodStringView<char>& a, const PodStringView<char> b) noexcept->PodStringView<char> {
        PodStringView<char> rv = { a.first, a.first };
        const auto sublen = b.size();
        const auto end_of_a = a.end() - sublen + 1;
        // 遍历完整的字符串
        for (auto itr = a.begin(); itr < end_of_a;) {
            auto substr = b.first;
            // 第一次
            if (*itr == *substr) {
                while (*itr == *substr) {
                    ++itr; ++substr;
                    // 有戏
                    if (substr == b.second) {
                        a.first = itr;
                        rv.second = itr;
                        goto force_break_on_found;
                    }
                }
            }
            else ++itr;
        }
        // 没有找到
        a.first = a.second;
    force_break_on_found:
        return rv;
    }
}

#ifndef LUI_DISABLE_STYLE_SUPPORT
#include <style/ui_ssvalue.h>
#include <util/ui_aniamtion.h>
#include <text/ui_attribute.h>
#include <style/ui_attribute.h>
#include <core/ui_basic_type.h>
#include <core/ui_color_list.h>
#include <core/ui_string_view.h>
#include <style/ui_ssvalue_list.h>
#include <typecheck/int_by_size.h>
#include <util/ui_little_math.h>
#include <core/ui_manager.h>
// C++
//#include <algorithm>


// longui namespace
namespace LongUI {
    // SimpAC::FuncValue to U8View
    auto U8(SimpAC::FuncValue v) noexcept {
        return U8View{ v.first, v.first + v.length };
    }
    // detail namespace
    namespace detail {
        // using namespace
        using namespace SimpAC;
        // attribute write
        template<typename T, typename U> 
        static inline void attribute(T& a, U b) noexcept {
            static_assert(sizeof(T) == sizeof(U), "must be same");
            a = static_cast<T>(b);
        }
        // xul image to id
        auto xul_image_to_id(U8View) noexcept->uintptr_t;
        // parse bgrepeat
        auto parse_bgrepeat(const FuncValue[], uint32_t) noexcept->uint8_t;
        // parse image
        auto parse_image(FuncValue value) noexcept->uintptr_t;
        // parse time
        auto parse_time(U8View value) noexcept -> float;
        // parse float
        auto parse_float(FuncValue value) noexcept -> float;
        // parse string
        auto parse_string(U8View value) noexcept->const char*;
        // parse timing function
        auto parse_timingfunc(U8View value) noexcept->AnimationType;
        // parse slice
        void parse_slice(const FuncValue[], uint32_t, SSValue& out) noexcept;
        // 1 for 4
        void one_for_four(
            ValueType vtype,
            uint32_t value_len,
            const SimpAC::FuncValue values[],
            void* values_write
        ) noexcept;
    }
}

/// <summary>
/// Values the type make value.
/// </summary>
/// <param name="vtype">The vtype.</param>
/// <param name="value_len">Length of the value.</param>
/// <param name="values">The values.</param>
/// <param name="values_write">The values write.</param>
/// <returns></returns>
void LongUI::ValueTypeMakeValue(
    ValueType vtype, 
    uint32_t value_len,
    const SimpAC::FuncValue values[],
    void* values_write) noexcept {
    assert(value_len && "bad len");
#ifndef NDEBUG
    // 检测U8View有效性
    const auto len = value_len;
    for (uint32_t i = 0; i != len; ++i) {
        assert(values[i].length && "bad length");
    }
#endif // !NDEBUG
    SSValue ssv = {  };
    ssv.type = vtype;
    //ssv.data.u32 = 0;
    // 分类讨论
    switch (vtype)
    {
    default: 
        assert(!"NOT MATCHED");
        // 没有数据
        return;
    case ValueType::Type_PositionOverflowX:
    case ValueType::Type_PositionOverflowY:
        // [OVERFLOW]
        //   -- overflow-x
        //   -- overflow-y
        detail::attribute(ssv.data4.byte, AttrParser::Overflow(U8(values[0])));
        break;
    case ValueType::Type_MarginTop:
    case ValueType::Type_MarginRight:
    case ValueType::Type_MarginBottom:
    case ValueType::Type_MarginLeft:
    case ValueType::Type_PaddingTop:
    case ValueType::Type_PaddingRight:
    case ValueType::Type_PaddingBottom:
    case ValueType::Type_PaddingLeft:
    case ValueType::Type_BorderTopWidth:
    case ValueType::Type_BorderRightWidth:
    case ValueType::Type_BorderBottomWidth:
    case ValueType::Type_BorderLeftWidth:
        // [MBP]
    case ValueType::Type_PositionLeft:
    case ValueType::Type_PositionTop:
    case ValueType::Type_DimensionWidth:
    case ValueType::Type_DimensionHeight:
    case ValueType::Type_DimensionMinWidth:
    case ValueType::Type_DimensionMinHeight:
    case ValueType::Type_DimensionMaxWidth:
    case ValueType::Type_DimensionMaxHeight:
    case ValueType::Type_BoxFlex:
    case ValueType::Type_FontSize:
    case ValueType::Type_WKTextStrokeWidth:
        // [FLOAT]
        assert(value_len == 1 && "unsupported");
        detail::attribute(ssv.data4.single, detail::parse_float(values[0]));
        break;
    case ValueType::Type_BackgroundColor:
    case ValueType::Type_BorderColor:
    case ValueType::Type_TextColor:
    case ValueType::Type_WKTextStrokeColor:
        // [COLOR]
        //   -- background-color
        //   -- border-color
        //   -- color
        //   -- -webkit-text-stroke-color
        assert(value_len == 1 && "unsupported");
        assert(!values[0].func && "function unsupported");
        detail::attribute(ssv.data4.u32, U8(values[0]).ColorRGBA32());
        break;
    case ValueType::Type_BorderStyle:
        assert(value_len == 1 && "unsupported");
        // [B-STYLE]
        //   -- border-style
        // 目前只有solid
#ifndef NDEBUG
        if (std::strncmp(values->first, "solid", values->length)) {
            LUIDebug(Error) << "support 'solid' only" << endl;
        }
#endif // !NDEBUG
        detail::attribute(ssv.data4.byte, values->first[0] == 's' ? Style_Solid : Style_None);
        break;
    case ValueType::Type_BorderRadius:
        // [RADIUS]
        //   -- border-radius
        assert(value_len <= 3 && "unsupported");
        detail::attribute(ssv.data4.single, detail::parse_float(values[0]));
        ssv.data8.single[0] = ssv.data4.single;
        ssv.data8.single[1] = ssv.data8.single[0];
        if (value_len == 3) {
            assert(values[1].first[0] == '/' && "unsupported");
            detail::attribute(ssv.data8.single[1], detail::parse_float(values[2]));
        }
        break;
    case ValueType::Type_BorderImageSlice:
        // [SLICE]
        //   -- border-image-slice
        assert(value_len <= 5 && "unsupported");
        detail::parse_slice(values, value_len, ssv);
        break;
    case ValueType::Type_BorderImageSource:
    case ValueType::Type_BackgroundImage:
        // [IMAGE]
        //   -- border-image-source
        //   -- background-image
        assert(value_len == 1 && "unsupported");
        detail::attribute(ssv.data8.handle, detail::parse_image(values[0]));
        break;
    case ValueType::Type_BackgroundRepeat:
    case ValueType::Type_BorderImageRepeat:
        // [REPEAT]
        //   -- background-repeat
        assert(value_len <= 2 && "unsupported");
        detail::attribute(ssv.data4.byte, detail::parse_bgrepeat(values, value_len));
        break;
    case ValueType::Type_BackgroundClip:
    case ValueType::Type_BackgroundOrigin:
        // [BOX]
        //   -- background-clip
        //   -- background-origin
        assert(value_len < 2 && "unsupported");
        detail::attribute(ssv.data4.byte, AttrParser::Box(U8(values[0])));
        break;
    case ValueType::Type_TransitionDuration:
        // [TIME]
        //   -- transition-duration
        assert(value_len == 1 && "unsupported");
        detail::attribute(ssv.data4.single, detail::parse_time(U8(values[0])));
        break;
    case ValueType::Type_TransitionTimingFunc:
        // [TIMING FUNCTION]
        //   -- transition-timing-function
        assert(value_len == 1 && "unsupported");
        detail::attribute(ssv.data4.byte, detail::parse_timingfunc(U8(values[0])));
        break;
    case ValueType::Type_TextAlign:
        // [TEXT ALIGN]
        assert(value_len == 1 && "unsupported");
        detail::attribute(ssv.data4.byte, TFAttrParser::TextAlign(U8(values[0])));
        break;
    case ValueType::Type_FontStyle:
        // [FontStyle]
        //   -- font-size
        assert(value_len == 1 && "unsupported");
        detail::attribute(ssv.data4.byte, TFAttrParser::Style(U8(values[0])));
        break;
    case ValueType::Type_FontStretch:
        // [FontStretch]
        //   -- font-stretch
        assert(value_len == 1 && "unsupported");
        detail::attribute(ssv.data4.byte, TFAttrParser::Stretch(U8(values[0])));
        break;
    case ValueType::Type_FontWeight:
        // [FontStretch]
        //   -- font-weight
        assert(value_len == 1 && "unsupported");
        detail::attribute(ssv.data4.word, TFAttrParser::Weight(U8(values[0])));
        break;
    case ValueType::Type_FontFamily:
        // [FontFamily]
        //   -- font-family
        assert(value_len == 1 && "unsupported");
        detail::attribute(ssv.data8.strptr, detail::parse_string(U8(values[0])));
        break;
    case ValueType::Type_LUIAppearance:
        // [APPEARANCE]
        //   -- -moz-appearance
        detail::attribute(ssv.data4.byte, AttrParser::Appearance(U8(values[0])));
        break;

        // ------------------- 简写属性 -----------------

    case ValueType::Type_ShortHandOverflow:
        // overflow: a [b]
        // re:overflow-x
        ValueTypeMakeValue(
            ValueType::Type_PositionOverflowX,
            1, values, values_write
        );
        // re:overflow-y
        ValueTypeMakeValue(
            ValueType::Type_PositionOverflowY,
            1, values + value_len - 1, values_write
        );
        return;

    case ValueType::Type_ShortHandWKTextStroke:
        // [TEXT STROKE]
        //   -- -webkit-text-stroke
        assert(value_len == 2 && "unsupported");
        // re:-webkit-text-stroke-width
        ValueTypeMakeValue(
            ValueType::Type_WKTextStrokeWidth,
            1, values, values_write
        );
        // re:-webkit-text-stroke-color
        ValueTypeMakeValue(
            ValueType::Type_WKTextStrokeColor,
            1, values + value_len - 1, values_write
        );
        return;
    case ValueType::Type_ShortHandMargin:
    case ValueType::Type_ShortHandPadding:
    case ValueType::Type_ShortHandBorderWidth:
        // [1FOR4]
        //   -- margin
        //   -- padding
        //   -- border-width
    {
        const ValueType map[] = {
            ValueType::Type_MarginTop,
            ValueType::Type_PaddingTop,
            ValueType::Type_BorderTopWidth,
        };
        const auto vtype0 = static_cast<uint32_t>(ValueType::Type_ShortHandMargin);
        const auto vtype1 = static_cast<uint32_t>(vtype);
        assert(vtype1 - vtype0 < sizeof(map) / sizeof(map[0]));
        const auto vtype2 = map[vtype1 - vtype0];
        detail::one_for_four(vtype2, value_len, values, values_write);
        return;
    }
    }
    // 输出
    auto& out = *reinterpret_cast<SSValues*>(values_write);
    out.push_back(ssv);
}

/// <summary>
/// U8s the type of the view2 value.
/// </summary>
/// <param name="view">The view.</param>
/// <returns></returns>
auto LongUI::U8View2ValueType(U8View view) noexcept -> ValueType {
    const auto bkdr = LongUI::BKDRHash(view.begin(), view.end());
    switch (bkdr)
    {
        // ------------- Position ----------------

    case 0xa8321dfd_ui32:
        // overflow-x
        return { ValueType::Type_PositionOverflowX };
    case 0xa8321dfe_ui32:
        // overflow-y
        return { ValueType::Type_PositionOverflowY };
    case 0x0e936497_ui32:
        // left
        return { ValueType::Type_PositionLeft };
    case 0x001e9951_ui32:
        // top
        return { ValueType::Type_PositionTop };

        // ------------- Dimension ----------------
        // width
    case 0x370bff82_ui32:
        return { ValueType::Type_DimensionWidth };
        // height
    case 0x28d4978b_ui32:
        return { ValueType::Type_DimensionHeight };
        // min-width
    case 0xb722a407_ui32:
        return { ValueType::Type_DimensionMinWidth };
        // min-height
    case 0xb46ac79a_ui32:
        return { ValueType::Type_DimensionMinHeight };
        // max-width
    case 0x0aeeba29_ui32:
        return { ValueType::Type_DimensionMaxWidth };
        // max-height
    case 0x95da1b00_ui32:
        return { ValueType::Type_DimensionMaxHeight };

        // ---------------- Box -------------------
        
    case 0x1818e927:
        // box-flex
        return { ValueType::Type_BoxFlex };
        // ------------- Margin ----------------

    case 0x4b67d8e6_ui32:
        // margin-top
        return { ValueType::Type_MarginTop };
    case 0xb07f6e2d_ui32:
        // margin-right
        return { ValueType::Type_MarginRight };
    case 0x040402e6_ui32:
        // margin-bottom
        return { ValueType::Type_MarginBottom };
    case 0x950eedd6_ui32:
        // margin-left
        return { ValueType::Type_MarginLeft };

        // ------------- Padding ----------------

    case 0xf617c735_ui32:
        // padding-top
        return { ValueType::Type_PaddingTop };
    case 0xbc0d7bf4_ui32:
        // padding-right
        return { ValueType::Type_PaddingRight };
    case 0xedb50fbb_ui32:
        // padding-bottom
        return { ValueType::Type_PaddingBottom };
    case 0xed15e043_ui32:
        // padding-left
        return { ValueType::Type_PaddingLeft };


        // ------------- Border ----------------

    case 0xe2efc419_ui32:
        // border-top-width
        return { ValueType::Type_BorderTopWidth };
    case 0xd3d82b98_ui32:
        // border-right-width
        return { ValueType::Type_BorderRightWidth };
    case 0xec99a7dd_ui32:
        // border-bottom-width
        return { ValueType::Type_BorderBottomWidth };
    case 0x3d08d15d_ui32:
        // border-left-width
        return { ValueType::Type_BorderLeftWidth };
    case 0x3a8c9410_ui32:
        // border-style
        return { ValueType::Type_BorderStyle };
    case 0x2102451a_ui32:
        // border-color
        return { ValueType::Type_BorderColor };
    case 0xaa15d28b_ui32:
        // border-radius
        return { ValueType::Type_BorderRadius };
    case 0xdc5bccaa_ui32:
        // border-image-source
        return { ValueType::Type_BorderImageSource };
    case 0x9ba640af_ui32:
        // border-image-slice
        return { ValueType::Type_BorderImageSlice };
    case 0x309f759a_ui32:
        // border-image-repeat
        return { ValueType::Type_BorderImageRepeat };

        // ------------- Background ----------------

    case 0x985a232c_ui32:
        // background-color
        return { ValueType::Type_BackgroundColor };
    case 0x0164e854_ui32:
        // background-image
        return { ValueType::Type_BackgroundImage };
    case 0x03dddbea_ui32:
        // background-repeat
        return { ValueType::Type_BackgroundRepeat };
    case 0xdc07b69b_ui32:
        // background-clip
        return { ValueType::Type_BackgroundClip };
    case 0xf493a695_ui32:
        // background-origin
        return { ValueType::Type_BackgroundOrigin };

        // ------------- Transition ----------------

    case 0x10117138_ui32:
        // transition-duration
        return { ValueType::Type_TransitionDuration };
    case 0xa5460fb3_ui32:
        // transition-timing-function
        return { ValueType::Type_TransitionTimingFunc };

        // ------------- TextFont  ----------------

    case 0xd8c9a893_ui32:
        // color
        return { ValueType::Type_TextColor };
    case 0xa53a05fd_ui32:
        // text-align
        return { ValueType::Type_TextAlign };
    case 0xc7329a9b_ui32:
        // -webkit-text-stroke-width
        return { ValueType::Type_WKTextStrokeWidth };
    case 0x68f043ac_ui32:
        // -webkit-text-stroke-color
        return { ValueType::Type_WKTextStrokeColor };
    case 0x841ed0a7_ui32:
        // font-size
        return { ValueType::Type_FontSize };
    case 0x9d3ddc0f_ui32:
        // font-style
        return { ValueType::Type_FontStyle };
    case 0x3bf8aeab_ui32:
        // font-stretch
        return { ValueType::Type_FontStretch };
    case 0x5b44f3da_ui32:
        // font-weight
        return { ValueType::Type_FontWeight };
    case 0x61ced682_ui32:
        // font-family
        return { ValueType::Type_FontFamily };
        // ------------- LongUI ----------------
#if 0
    case 0xee354dac_ui32:
        // appearance
#endif
    case 0xd6e6b71a_ui32:
        // -moz-appearance
        return { ValueType::Type_LUIAppearance  };


        // ------------- SHORTHAND --------------

    case 0x29867c0e_ui32:
        // overflow
        return { ValueType::Type_ShortHandOverflow };
    case 0x2c36f3b2_ui32:
        // -webkit-text-stroke
        return { ValueType::Type_ShortHandWKTextStroke };
    case 0xcd67f276_ui32:
        // margin
        return { ValueType::Type_ShortHandMargin };
    case 0x44220515_ui32:
        // padding
        return { ValueType::Type_ShortHandPadding };
    case 0x7f449c09_ui32:
        // border-width
        return { ValueType::Type_ShortHandBorderWidth };
    default:
        break;
    }
    return { ValueType::Type_Unknown };
}

/// <summary>
/// Gets the type of the easy.
/// </summary>
/// <param name="type">The type.</param>
/// <returns></returns>
auto LongUI::GetEasyType(ValueType type) noexcept -> ValueEasyType {
    // XXX: 优化为简单查表
    switch (type)
    {
    case LongUI::ValueType::Type_MarginTop:
    case LongUI::ValueType::Type_MarginRight:
    case LongUI::ValueType::Type_MarginBottom:
    case LongUI::ValueType::Type_MarginLeft:
    case LongUI::ValueType::Type_PaddingTop:
    case LongUI::ValueType::Type_PaddingRight:
    case LongUI::ValueType::Type_PaddingBottom:
    case LongUI::ValueType::Type_PaddingLeft:
    case LongUI::ValueType::Type_BorderTopWidth:
    case LongUI::ValueType::Type_BorderRightWidth:
    case LongUI::ValueType::Type_BorderBottomWidth:
    case LongUI::ValueType::Type_BorderLeftWidth:
        // [MBP]
    case ValueType::Type_PositionLeft:
    case ValueType::Type_PositionTop:
    case ValueType::Type_DimensionWidth:
    case ValueType::Type_DimensionHeight:
    case ValueType::Type_DimensionMinWidth:
    case ValueType::Type_DimensionMinHeight:
    case ValueType::Type_DimensionMaxWidth:
    case ValueType::Type_DimensionMaxHeight:
    case ValueType::Type_BoxFlex:
    case ValueType::Type_FontSize:
        // [FLOAT]
        return ValueEasyType::Type_Float;

    case ValueType::Type_BorderRadius:
        // [SIZE]
        return ValueEasyType::Type_Size;
    default:
        assert(!"error type");

    case LongUI::ValueType::Type_BorderImageSource:
    case LongUI::ValueType::Type_BackgroundImage:
        // [IMAGE]
        //return ValueEasyType::Type_NoAnimation;
    case LongUI::ValueType::Type_BorderStyle:
    case LongUI::ValueType::Type_BorderImageSlice:
    case LongUI::ValueType::Type_BorderImageRepeat:
    case LongUI::ValueType::Type_BackgroundAttachment:
    case LongUI::ValueType::Type_BackgroundRepeat:
    case LongUI::ValueType::Type_BackgroundClip:
    case LongUI::ValueType::Type_BackgroundOrigin:
    case LongUI::ValueType::Type_TextAlign:
    case LongUI::ValueType::Type_FontStyle:
    case LongUI::ValueType::Type_FontStretch:
    case LongUI::ValueType::Type_FontWeight:
    case LongUI::ValueType::Type_LUIAppearance:
        // [STATE]
    case LongUI::ValueType::Type_FontFamily:
        // [STRING]
    case LongUI::ValueType::Type_TransitionDuration:
        // [SPTIME]
        return ValueEasyType::Type_NoAnimation;

    case LongUI::ValueType::Type_BackgroundColor:
    case LongUI::ValueType::Type_BorderColor:
    case LongUI::ValueType::Type_TextColor:
    case LongUI::ValueType::Type_WKTextStrokeColor:
        // [COLOR]
        return ValueEasyType::Type_Color;
    }
}

/// <summary>
/// Determines whether [is image type] [the specified type].
/// </summary>
/// <param name="type">The type.</param>
/// <returns></returns>
bool LongUI::IsImageType(ValueType type) noexcept {
    switch (type)
    {
    case LongUI::ValueType::Type_BackgroundImage:
    case LongUI::ValueType::Type_BorderImageSource:
        return true;
    }
    return false;
}

// longui
namespace LongUI {
    PCN_NOINLINE
    /// <summary>
    /// U8s to pc.
    /// </summary>
    /// <param name="pc">The pc.</param>
    /// <param name="view">The view.</param>
    /// <returns></returns>
    void U8ToPC(SSValuePC& pc, U8View view) noexcept {
        const auto bkdr = LongUI::BKDRHash(view.begin(), view.end());
        switch (bkdr)
        {
        case 0x2694fd9a_ui32:
            // :active
            pc.yes.active = true;
            break;
        case 0x091a155f_ui32:
            // :checked
            pc.yes.checked = true;
            break;
        case 0xdf345f61_ui32:
            // :default
            pc.yes.default5 = true;
            break;
        case 0x715f1adc_ui32:
            // :disabled
            pc.yes.disabled = true;
            break;
        case 0xd3d78067_ui32:
            // :enabled
            pc.noo.disabled = true;
            break;
        case 0x0d707348_ui32:
            // :focus
            pc.yes.focus = true;
            break;
        case 0x3090d164_ui32:
            // :hover
            pc.yes.hover = true;
            break;
        case 0xc42c9ea7_ui32:
            // :indeterminate
            pc.yes.indeterminate = true;
            break;
        case 0x03481b1f_ui32:
            // :selected
            pc.yes.selected = true;
            break;
        case 0xbaf00ef8_ui32:
            // :closed
            pc.yes.closed = true;
            break;
        //case 0x5f149358_ui32:
        //    // closed
        //    pc.yes.opening = true;
        //    break;
        //case 0xd398231b_ui32:
        //    // closed
        //    pc.yes.ending = true;
        //    break;

        }
    }
    // detail namespace
    namespace detail {
        // parse image
        auto parse_image(SimpAC::FuncValue value) noexcept -> uintptr_t {
            switch (value.func)
            {
            default: assert(!"unsupported"); break;
            case SimpAC::FuncType::Type_Url:
                if (*value.first == '\'' || *value.first == '\"') {
                    assert(value.first[value.length - 1] == *value.first);
                    assert(value.length > 2);
                    value.first++;
                    value.length -= 2;
                }
                return xul_image_to_id(U8(value));
            }
            return 0;
        }
        // parse time
        auto parse_time(U8View value) noexcept -> float {
            // 默认定为秒
            const auto unit = LongUI::SplitUnit(luiref value);
            float rv = value.ToFloat();
            // 检测单位
#if 1
            if (unit.begin()[0] == 'm')  rv /= 1000.;
#else
            if (unit.end() > unit.begin()) {
                switch (unit.begin()[0])
                {
                    // 毫秒就除以1000
                case 'm': rv /= 1000.;
                }
            }
#endif
            // 时长超过1分钟, 生命会减少一秒
            if (rv > 60.f) rv -= 1.f;
            return rv;
        }
        // parse bg repeat
        auto parse_bgrepeat(const FuncValue v[], uint32_t l) noexcept -> uint8_t {
            assert(l == 1 || l == 2);
            const auto v1 = U8(v[0]);
            const auto v2 = U8(l == 1 ? v[0] : v[1]);
            return AttrParser::Repeat2(v1, v2);
        }
        // parse float
        auto parse_float(FuncValue value) noexcept -> float {
            auto view = U8(value);
            const auto unit = LongUI::SplitUnit(luiref view);
            const auto single = view.ToFloat();
            // 存在单位
            if (unit.begin() != unit.end()) {
                // 百分比
                if (*unit.begin() == '%')
                    return detail::mark_percent_from100(single);
#ifndef NDEBUG
                else if (std::strncmp(unit.begin(), "px", 2)) {
                    assert(!"UNSUPPORTED UNIT");
                }
#endif
            }
            return single;
        }
        // one_for_four map
        static const int8_t one_for_four_map[] = {
            0, 0, 0, 0,
            0, 1, 1, 1,
            0, 0, 2, 2,
            0, 1, 1, 3,
        };
        PCN_NOINLINE
        /// <summary>
        /// Ones for four.
        /// </summary>
        /// <param name="vtype">The vtype.</param>
        /// <param name="value_len">Length of the value.</param>
        /// <param name="values">The values.</param>
        /// <param name="values_write">The values write.</param>
        /// <returns></returns>
        void one_for_four(
            ValueType vtype, 
            uint32_t value_len, 
            const SimpAC::FuncValue values[], 
            void* values_write) noexcept {
            assert(value_len);
            // 1, 1, 1, 1
            // 1, 2, 1, 2
            // 1, 2, 3, 2
            // 1, 2, 3, 4
            const auto vtype0 = static_cast<uint32_t>(vtype);
            const auto vtype1 = static_cast<ValueType>(vtype0 + 1);
            const auto vtype2 = static_cast<ValueType>(vtype0 + 2);
            const auto vtype3 = static_cast<ValueType>(vtype0 + 3);
            const auto index = (value_len > 4 ? 4 : value_len) - 1;
            const auto i1 = one_for_four_map[index + 4 * 0];
            const auto i2 = one_for_four_map[index + 4 * 1];
            const auto i3 = one_for_four_map[index + 4 * 2];
            const auto i4 = one_for_four_map[index + 4 * 3];
            LongUI::ValueTypeMakeValue(vtype , 1, values + i1, values_write);
            LongUI::ValueTypeMakeValue(vtype1, 1, values + i2, values_write);
            LongUI::ValueTypeMakeValue(vtype2, 1, values + i3, values_write);
            LongUI::ValueTypeMakeValue(vtype3, 1, values + i4, values_write);
        }
        /// <summary>
        /// Parses the timingfunc.
        /// </summary>
        /// <param name="value">The value.</param>
        /// <returns></returns>
        auto parse_timingfunc(U8View value) noexcept -> AnimationType {
            assert(value.end() > value.begin());
            // XXX: 
            /*
                ease
                linear
                ease-in
                ease-out
                ease-in-out
            */
            switch (value.end() - value.begin())
            {
            case 1: case 2:
                return static_cast<AnimationType>(value.ToInt32());
            case 4:
                // ease
                return Type_CubicEaseOut;
            case 6:
                // linear
                return Type_LinearInterpolation;
            case 7:
                // ease-in
                return Type_CubicEaseIn;
            case 8:
                // ease-out
                return Type_CubicEaseOut;
            case 11:
                // ease-out
                return Type_CubicEaseInOut;
            }
            return Type_LinearInterpolation;
        }
    }
}


#include <text/ui_ctl_arg.h>

/// <summary>
/// Initializes the default state.
/// </summary>
/// <param name="buf">The buf.</param>
/// <param name="buf2">The buf2.</param>
/// <returns></returns>
void LongUI::InitDefaultState(UniByte4 buf[], UniByte8 buf2[]) noexcept {
    // check type count
    static_assert(static_cast<int>(ValueType::SINGLE_LAST) <= 256, "TO HUGE");
    const auto& font = UIManager.GetDefaultFont();
    // 字体
    buf[static_cast<int>(ValueType::Type_TextColor)].u32 
        = RGBA_Black;
    buf[static_cast<int>(ValueType::Type_FontSize)].single 
        = font.size;
    buf[static_cast<int>(ValueType::Type_FontStretch)].byte
        = font.stretch;
    buf[static_cast<int>(ValueType::Type_FontWeight)].word
        = font.weight;
    // 大小
    buf[static_cast<int>(ValueType::Type_DimensionMaxWidth)].single
        = static_cast<float>(DEFAULT_CONTROL_MAX_SIZE);
    buf[static_cast<int>(ValueType::Type_DimensionMaxHeight)].single
        = static_cast<float>(DEFAULT_CONTROL_MAX_SIZE);
}


/// <summary>
/// Parses the string.
/// </summary>
/// <param name="value">The value.</param>
/// <returns></returns>
auto LongUI::detail::parse_string(U8View value) noexcept -> const char* {
    return UIManager.GetUniqueText(value).id;
}


/// <summary>
/// Parses the slice.
/// </summary>
/// <param name="list">The list.</param>
/// <param name="len">The length.</param>
/// <param name="out">The out.</param>
/// <returns></returns>
void LongUI::detail::parse_slice(
    const SimpAC::FuncValue list[], 
    uint32_t len, 
    SSValue& out) noexcept {
#if 0
    for (int16_t i = -10000; i != 101; ++i) {
        RectF rect;
        UniByte8 data, after;
        data.i16[0] = i;
        data.i16[1] = i;
        data.i16[2] = i;
        data.i16[3] = i;
        LongUI::UniByte8ToSliceRect(data, &rect.left);
        LongUI::SliceRectToUniByte8(&rect.left, after);
        LUIDebug(Log)
            << "[" << int(data.i16[0]) << "]"
            << (detail::is_percent_value(rect.left) ?
                detail::get_percent_value(rect.left) :
                rect.left)
            << "[" << int(after.i16[0]) << "]"
            << endl;
    }
    std::terminate();
#endif
    int16_t i16_list[4];
    int count = 0;
    for (uint32_t i = 0; i != len; ++i) {
        // 关键字 fill
        if (list[i].first[0] == 'f') {
            out.data4.boolean = true;
            continue;
        }
        // 其他情况
        auto view = U8(list[i]);
        const auto unit = LongUI::SplitUnit(view);
        const auto single = view.ToFloat();
        // 百分比
        if (unit.begin() != unit.end() && unit.begin()[0] == '%') {
            // 以一万为基础, 最多三万
            const auto after = detail::clamp(single, 0.f, 300.f);
#ifndef NDEBUG
            if (after != single) {
                LUIDebug(Error)
                    << "slice: unsupported value: "
                    << U8(list[i])
                    << endl;
            }
#endif
            const int16_t percent = static_cast<int16_t>(after * 100.f);
            i16_list[count] = -percent;
        }
        // 数字
        else i16_list[count] = static_cast<int16_t>(single);
        if (++count == 4) break;
    }
    // 建立映射
    const auto i0 = one_for_four_map[count - 1 + 4 * 0];
    const auto i1 = one_for_four_map[count - 1 + 4 * 1];
    const auto i2 = one_for_four_map[count - 1 + 4 * 2];
    const auto i3 = one_for_four_map[count - 1 + 4 * 3];
    out.data8.i16[0] = i16_list[i0];
    out.data8.i16[1] = i16_list[i1];
    out.data8.i16[2] = i16_list[i2];
    out.data8.i16[3] = i16_list[i3];
}

/// <summary>
/// Unis the byte8 to slice rect.
/// </summary>
/// <param name="data">The data.</param>
/// <param name="out">The out.</param>
/// <returns></returns>
void LongUI::UniByte8ToSliceRect(UniByte8 data, float out[4]) noexcept {
    constexpr int COUNT = 4;
    for (int i = 0; i != COUNT; ++i) {
        const auto i16value = data.i16[i];
        float writen;
        // 百分比
        if (i16value < 0) {
            const auto percent10000 = -i16value;
            writen = detail::mark_percent_from10000(float(percent10000));
        }
        // 大小
        else writen = static_cast<float>(i16value);
        // 写入
        out[i] = writen;
    }
}

/// <summary>
/// Slices the rect to uni byte8.
/// </summary>
/// <param name="rect">The rect.</param>
/// <param name="out">The out.</param>
/// <returns></returns>
void LongUI::SliceRectToUniByte8(const float rect[4], UniByte8& out) noexcept {
    constexpr int COUNT = 4;
    for (int i = 0; i != COUNT; ++i) {
        const auto fvalue = rect[i];
        int16_t writen;
        // 百分比
        if (detail::is_percent_value(fvalue)) {
            const auto percent = detail::get_percent_value(fvalue);
            const auto percent10000 = percent * 1e4f;
            const int16_t i16value = static_cast<int16_t>(percent10000);
            writen = -i16value;
        }
        // 大小
        else writen = static_cast<int16_t>(fvalue);
        // 写入
        out.i16[i] = writen;
    }
}

#endif
/*

//// [Position] cursor
//Type_PositionCursor,
//// [Position] z-index
//Type_PositionZindex,


//// [Border] top-style
//Type_BorderTopStyle,
//// [Border] right-style
//Type_BorderRightStyle,
//// [Border] bottom-style
//Type_BorderBottomStyle,
//// [Border] left-style
//Type_BorderLeftStyle,
//// [Border] top-color
//Type_BorderTopColor,
//// [Border] right-color
//Type_BorderRightColor,
//// [Border] bottom-color
//Type_BorderBottomColor,
//// [Border] left-color
//Type_BorderLeftColor,
//// [Border] top-left-radius
//Type_BorderTopLeftRadius,
//// [Border] top-right-radius
//Type_BorderTopRightRadius,
//// [Border] bottom-left-radius
//Type_BorderBottomLeftRadius,
//// [Border] bottom-right-radius
//Type_BorderBottomRightRadius,
*/
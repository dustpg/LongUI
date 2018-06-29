// lui
#include <style/ui_ssvalue.h>
#include <style/ui_attribute.h>
#include <core/ui_string_view.h>
#include <style/ui_ssvalue_list.h>
#include <typecheck/int_by_size.h>
// css
#include <xul/SimpAC.h>

// longui namespace
namespace LongUI {
    // BKDR Hash Function
    auto BKDRHash(const char* a, const char* b) noexcept->uint32_t;
    // detail namespace
    namespace detail {
        // attribute write
        template<typename T, typename U> 
        static inline void attribute(T& a, U b) noexcept {
            static_assert(sizeof(T) == sizeof(U));
            a = static_cast<T>(b);
        }
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
    const U8View values[],
    void* values_write) noexcept {
    assert(value_len && "bad len");
#ifndef NDEBUG
    // 检测U8View有效性
    const auto len = value_len;
    for (uint32_t i = 0; i != len; ++i) {
        assert(values[i].end() > values[i].begin());
    }
#endif // !NDEBUG
    SSValue ssv;
    ssv.type = vtype;
    ssv.data.u32 = 0;
    // 分类讨论
    switch (vtype)
    {
    default: 
        // 没有数据
        return;
    case ValueType::Type_PositionOverflow:
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
    case ValueType::Type_PositionOverflowX:
    case ValueType::Type_PositionOverflowY:
        // [OVERFLOW]
        //   -- overflow-x
        //   -- overflow-y
        detail::attribute(ssv.data.byte, AttrParser::Overflow(values[0]));
        break;


    case ValueType::Type_BackgroundColor:
        // [COLOR]
        //   -- background-color
        assert(value_len == 1 && "unsupported");
        detail::attribute(ssv.data.u32, values[0].ColorRGBA32());
        break;

    case ValueType::Type_TransitionDuration:
        // [TIME]
        //   -- transition-duration
        assert(value_len == 1 && "unsupported");
        // 默认定为秒
        {
            auto v0 = reinterpret_cast<const SimpAC::StrPair*>(values)[0];
            const auto unit = SimpAC::SplitUnit(v0);
            ssv.data.single = reinterpret_cast<U8View&>(v0).ToFloat();
            // 检测单位
            if (unit.end() > unit.begin()) {
                switch (unit.begin()[0])
                {
                    // 毫秒就除以1000
                case 'm': ssv.data.single /= 1000.;
                }
            }
        }
        break;
    case ValueType::Type_UIAppearance:
        // [APPEARANCE]
        //   -- -moz-appearance
        detail::attribute(ssv.data.byte, AttrParser::Appearance(values[0]));
        break;
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

    case 0x29867c0e_ui32:
        // overflow
        return { ValueType::Type_PositionOverflow };
    case 0xa8321dfd_ui32:
        // overflow-x
        return { ValueType::Type_PositionOverflowX };
    case 0xa8321dfe_ui32:
        // overflow-y
        return { ValueType::Type_PositionOverflowY };


        // ------------- Margin ----------------

    case 0xcd67f276_ui32:
        // margin
        return { ValueType::Type_Margin };
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

    case 0x44220515_ui32:
        // padding
        return { ValueType::Type_Padding };
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

    case 0x7f449c09_ui32:
        // border-width
        return { ValueType::Type_BorderWidth };
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



        // ------------- Background ----------------

    case 0x985a232c_ui32:
        // background-color
        return { ValueType::Type_BackgroundColor };


        // ------------- Transition ----------------

    case 0x10117138_ui32:
        // transition-duration
        return { ValueType::Type_TransitionDuration };
        // ------------- LongUI ----------------

#if 0
    case 0xee354dac_ui32:
        // appearance
#endif
    case 0xd6e6b71a_ui32:
        // -moz-appearance
        return { ValueType::Type_UIAppearance  };
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
        // [FLOAT]
        return ValueEasyType::Type_Float;
    case LongUI::ValueType::Type_BorderImageSource:
        // [IMAGE]
        assert(!"NOT IMPL");
    case LongUI::ValueType::Type_BackgroundAttachment:
    case LongUI::ValueType::Type_BackgroundRepeat:
    case LongUI::ValueType::Type_BackgroundClip:
    case LongUI::ValueType::Type_BackgroundOrigin:
    case LongUI::ValueType::Type_TransitionDuration:
    case LongUI::ValueType::Type_UIAppearance:
        // [STATE]
        return ValueEasyType::Type_NoAnimation;
    case LongUI::ValueType::Type_BackgroundColor:
        // [COLOR]
        return ValueEasyType::Type_Color;
    }
    assert(!"error type");
    return ValueEasyType::Type_NoAnimation;
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
            // active
            pc.yes.active = true;
            break;
        case 0x091a155f_ui32:
            // checked
            pc.yes.checked = true;
            break;
        case 0xdf345f61_ui32:
            // defualt
            pc.yes.default5 = true;
            break;
        case 0x715f1adc_ui32:
            // disabled
            pc.yes.disabled = true;
            break;
        case 0xd3d78067_ui32:
            // enabled
            pc.noo.disabled = true;
            break;
        case 0x0d707348_ui32:
            // focus
            pc.yes.focus = true;
            break;
        case 0x3090d164_ui32:
            // hover
            pc.yes.hover = true;
            break;
        case 0xc42c9ea7_ui32:
            // indeterminate
            pc.yes.indeterminate = true;
            break;
        case 0x03481b1f_ui32:
            // selected
            pc.yes.selected = true;
            break;
        }
    }
}


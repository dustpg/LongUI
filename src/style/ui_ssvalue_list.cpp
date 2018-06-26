// lui
#include <style/ui_ssvalue.h>
#include <style/ui_attribute.h>
#include <core/ui_string_view.h>
#include <style/ui_ssvalue_list.h>
#include <typecheck/int_by_size.h>

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
    ssv.u32 = 0;
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
        detail::attribute(ssv.byte, AttrParser::Overflow(values[0]));
        break;


    case ValueType::Type_BackgroundColor:
        // [COLOR]
        //   -- background-color

        assert(value_len == 1 && "unsupported");
        detail::attribute(ssv.u32, values[0].ColorRGBA32());
        break;


    case ValueType::Type_UIAppearance:
        // [APPEARANCE]
        //   -- -moz-appearance
        detail::attribute(ssv.byte, AttrParser::Appearance(values[0]));
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